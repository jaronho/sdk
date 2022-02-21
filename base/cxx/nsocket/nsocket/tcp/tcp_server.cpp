#include "tcp_server.h"

namespace nsocket
{
#ifdef _WIN32
namespace
{
/// See <http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx>
/// and <http://blogs.msdn.com/b/stevejs/archive/2005/12/19/505815.aspx> for
/// more information on the code below.
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void pthread_setname_np(DWORD dwThreadID, const char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}
} /* namespace */
#endif

static void setThreadName(const std::string& name)
{
#ifdef _WIN32
    pthread_setname_np(-1, name.c_str());
#else
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
#endif
}

io_context_pool::io_context_pool(const std::string& name, size_t poolSize) : m_name(name)
{
    poolSize = std::max<size_t>(1U, poolSize);
    for (size_t i = 0; i < poolSize; ++i)
    {
        auto context = std::make_shared<boost::asio::io_context>();
        auto worker = std::make_shared<boost::asio::io_context::work>(*context);
        m_contexts.emplace_back(context);
        m_workers.emplace_back(worker);
    }
}

io_context_pool::~io_context_pool()
{
    join();
}

void io_context_pool::start()
{
    for (size_t i = 0; i < m_contexts.size(); ++i)
    {
        auto th = std::make_shared<std::thread>([name = m_name, i, context = m_contexts[i]]() {
            /* 设置线程名称 */
            auto threadName = name + "-" + std::to_string(i + 1);
            setThreadName(threadName);
            context->run();
        });
        m_threads.emplace_back(th);
    }
}

void io_context_pool::join()
{
    for (size_t i = 0; i < m_contexts.size(); ++i)
    {
        m_contexts[i]->stop();
    }
    for (size_t i = 0; i < m_workers.size(); ++i)
    {
        m_workers[i].reset();
    }
    for (size_t i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i]->join();
    }
    m_threads.clear();
}

boost::asio::io_context& io_context_pool::getContext()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    boost::asio::io_context& context = *m_contexts[m_index];
    ++m_index;
    if (m_contexts.size() == m_index)
    {
        m_index = 0;
    }
    return context;
}

TcpServer::TcpServer(const std::string& name, size_t threadCount, const std::string& host, unsigned int port, bool reuseAddr, size_t bz)
    : m_contextPool(std::make_shared<io_context_pool>(name, threadCount))
#if (1 == ENABLE_NSOCKET_OPENSSL)
    , m_sslContext(nullptr)
#endif
{
    m_bufferSize = bz;
    try
    {
        m_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(
            m_contextPool->getContext(), boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host.c_str()), port), reuseAddr);
        m_host = host;
    }
    catch (...)
    {
        m_acceptor = nullptr;
    }
}

TcpServer::~TcpServer()
{
    stop();
}

bool TcpServer::isValid() const
{
    return (m_acceptor ? true : false);
}

bool TcpServer::isRunning() const
{
    return m_running;
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
bool TcpServer::run(const std::shared_ptr<boost::asio::ssl::context>& sslContext)
#else
bool TcpServer::run()
#endif
{
    if (m_running)
    {
        return true;
    }
    if (m_acceptor)
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        m_sslContext = sslContext;
        if (m_sslContext)
        {
            auto sessionIdCtx = std::to_string(m_acceptor->local_endpoint().port()) + ':';
            sessionIdCtx.append(m_host.rbegin(), m_host.rend());
            SSL_CTX_set_session_id_context(m_sslContext->native_handle(), (const unsigned char*)sessionIdCtx.data(),
                                           std::min<size_t>(sessionIdCtx.size(), SSL_MAX_SSL_SESSION_ID_LENGTH));
        }
#endif
        m_contextPool->start();
        doAccept();
        m_running = true;
        return true;
    }
    return false;
}

void TcpServer::stop()
{
    if (!m_running)
    {
        return;
    }
    if (m_acceptor)
    {
        m_acceptor->close();
        m_acceptor = nullptr;
    }
    m_contextPool->join();
    std::lock_guard<std::mutex> locker(m_mutex);
    m_connectionMap.clear();
    m_running = false;
}

void TcpServer::setNewConnectionCallback(const TCP_CONN_NEW_CALLBACK& onNewCb)
{
    m_onNewConnectionCallback = onNewCb;
}

void TcpServer::setConnectionDataCallback(const TCP_CONN_DATA_CALLBACK& onDataCb)
{
    m_onConnectionDataCallback = onDataCb;
}

void TcpServer::setConnectionCloseCallback(const TCP_CONN_CLOSE_CALLBACK& onCloseCb)
{
    m_onConnectionCloseCallback = onCloseCb;
}

void TcpServer::doAccept()
{
    if (m_acceptor)
    {
        const std::weak_ptr<TcpServer> wpSelf = shared_from_this();
        m_acceptor->async_accept(m_contextPool->getContext(),
                                 [wpSelf](boost::system::error_code code, boost::asio::ip::tcp::socket socket) {
                                     const auto self = wpSelf.lock();
                                     if (self)
                                     {
                                         if (!code) /* 有新连接请求 */
                                         {
                                             self->handleNewConnection(std::move(socket));
                                         }
                                         /* 继续接收下一个连接 */
                                         self->doAccept();
                                     }
                                 });
    }
}

void TcpServer::handleNewConnection(boost::asio::ip::tcp::socket socket)
{
    /* 创建新连接 */
    std::shared_ptr<TcpConnection> conn;
#if (1 == ENABLE_NSOCKET_OPENSSL)
    if (m_sslContext) /* 启用TLS */
    {
        conn = std::make_shared<TcpConnection>(std::make_shared<SocketTls>(std::move(socket), *(m_sslContext)), true, m_bufferSize);
    }
    else /* 不启用TLS */
    {
#endif
        conn = std::make_shared<TcpConnection>(std::make_shared<SocketTcp>(std::move(socket)), true, m_bufferSize);
#if (1 == ENABLE_NSOCKET_OPENSSL)
    }
#endif
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        if (m_connectionMap.end() == m_connectionMap.find(conn->getId()))
        {
            m_connectionMap.insert(std::make_pair(conn->getId(), conn));
        }
    }
    const std::weak_ptr<TcpServer> wpSelf = shared_from_this();
    const std::weak_ptr<TcpConnection> wpConn = conn;
    /* 设置连接回调 */
    conn->setConnectCallback([wpSelf, wpConn, point = conn->getRemoteEndpoint()](const boost::system::error_code& code) {
        const auto self = wpSelf.lock();
        const auto conn = wpConn.lock();
        if (!self || !conn)
        {
            return;
        }
        if (code) /* 断开连接 */
        {
            if (self && conn)
            {
                {
                    std::lock_guard<std::mutex> locker(self->m_mutex);
                    auto iter = self->m_connectionMap.find(conn->getId());
                    if (self->m_connectionMap.end() != iter)
                    {
                        self->m_connectionMap.erase(iter);
                    }
                }
                if (self->m_onConnectionCloseCallback)
                {
                    self->m_onConnectionCloseCallback(conn->getId(), point, code);
                }
            }
        }
        else /* 连接成功 */
        {
            if (conn->isEnableSSL()) /* 启用TLS */
            {
#if (1 == ENABLE_NSOCKET_OPENSSL)
                conn->handshake(boost::asio::ssl::stream_base::server, [wpSelf, wpConn](const boost::system::error_code& code) {
                    if (!code) /* 握手成功 */
                    {
                        const auto conn = wpConn.lock();
                        if (conn)
                        {
                            const auto self = wpSelf.lock();
                            if (self && self->m_onNewConnectionCallback)
                            {
                                self->m_onNewConnectionCallback(wpConn);
                            }
                        }
                    }
                }); /* 需要握手 */
#endif
            }
            else /* 没有启用TLS */
            {
                if (self->m_onNewConnectionCallback)
                {
                    self->m_onNewConnectionCallback(wpConn);
                }
            }
        }
    });
    /* 设置数据回调 */
    conn->setDataCallback([wpSelf, wpConn](const std::vector<unsigned char>& data) {
        const auto self = wpSelf.lock();
        if (self && self->m_onConnectionDataCallback)
        {
            self->m_onConnectionDataCallback(wpConn, data);
        }
    });
    /* 开始连接 */
    boost::system::error_code code;
    conn->connect(socket.remote_endpoint(code), true);
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
std::shared_ptr<boost::asio::ssl::context> TcpServer::getSsl2WayContext(const std::string& certFile, const std::string& privateKeyFile,
                                                                        const std::string& privateKeyFilePwd, bool allowSelfSigned)
{
    return TcpConnection::makeSsl2WayContext(boost::asio::ssl::context::sslv23_server, certFile, privateKeyFile, privateKeyFilePwd,
                                             allowSelfSigned);
}
#endif
} // namespace nsocket
