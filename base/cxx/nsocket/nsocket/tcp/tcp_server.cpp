#include "tcp_server.h"

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

void SetThreadName(DWORD dwThreadID, const char* threadName)
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

void setThreadName(const std::string& name)
{
#ifdef _WIN32
    SetThreadName(-1, name.c_str());
#else
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
#endif
}

namespace nsocket
{
TcpServer::TcpServer(const std::string& name, size_t threadCount, const std::string& host, unsigned int port, bool reuseAddr, size_t bz)
    : m_name(name)
    , m_threadCount(threadCount)
    , m_worker(boost::asio::make_work_guard(m_ioContext))
#if (1 == ENABLE_NSOCKET_OPENSSL)
    , m_sslContext(nullptr)
#endif
{
    m_bufferSize = bz;
    try
    {
        m_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(
            m_ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host.c_str()), port), reuseAddr);
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

#if (1 == ENABLE_NSOCKET_OPENSSL)
void TcpServer::run(const std::shared_ptr<boost::asio::ssl::context>& sslContext)
#else
void TcpServer::run()
#endif
{
    if (m_running)
    {
        return;
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
        doAccept();
        m_threads.create_threads(
            [this] {
                /* 设置线程名称 */
                ++m_threadIndex;
                auto threadName = m_name + "-" + std::to_string(m_threadIndex);
                setThreadName(threadName);
                m_ioContext.run();
            },
            std::max<size_t>(1U, m_threadCount));
        m_running = true;
    }
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
    m_ioContext.stop();
    m_worker.reset();
    m_threads.join();
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
    const std::weak_ptr<TcpServer> wpSelf = shared_from_this();
    m_acceptor->async_accept([wpSelf](boost::system::error_code code, boost::asio::ip::tcp::socket socket) {
        const auto self = wpSelf.lock();
        if (self)
        {
            if (!code) /* 有新连接请求 */
            {
                /* 创建新连接 */
                std::shared_ptr<TcpConnection> conn;
#if (1 == ENABLE_NSOCKET_OPENSSL)
                if (self->m_sslContext) /* 启用TLS */
                {
                    conn = std::make_shared<TcpConnection>(std::make_shared<SocketTls>(std::move(socket), *(self->m_sslContext)), true,
                                                           self->m_bufferSize);
                }
                else /* 不启用TLS */
                {
#endif
                    conn = std::make_shared<TcpConnection>(std::make_shared<SocketTcp>(std::move(socket)), true, self->m_bufferSize);
#if (1 == ENABLE_NSOCKET_OPENSSL)
                }
#endif
                {
                    std::lock_guard<std::mutex> locker(self->m_mutex);
                    if (self->m_connectionMap.end() == self->m_connectionMap.find(conn->getId()))
                    {
                        self->m_connectionMap.insert(std::make_pair(conn->getId(), conn));
                    }
                }
                const std::weak_ptr<TcpConnection> wpConn = conn;
                /* 设置连接回调 */
                conn->setConnectCallback([wpSelf, wpConn](const boost::system::error_code& code) {
                    if (code) /* 断开连接 */
                    {
                        const auto self = wpSelf.lock();
                        const auto conn = wpConn.lock();
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
                                self->m_onConnectionCloseCallback(conn->getId(), conn->getRemoteEndpoint(), code);
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
#if (1 == ENABLE_NSOCKET_OPENSSL)
                if (self->m_sslContext) /* 启用TLS */
                {
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
                                conn->recv(); /* 开始接收数据 */
                            }
                        }
                    }); /* 需要握手 */
                }
                else /* 不启用TLS */
                {
#endif
                    if (self->m_onNewConnectionCallback)
                    {
                        self->m_onNewConnectionCallback(wpConn);
                    }
                    conn->recv(); /* 开始接收数据 */
#if (1 == ENABLE_NSOCKET_OPENSSL)
                }
#endif
            }
            /* 继续接收下一个连接 */
            self->doAccept();
        }
    });
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
std::shared_ptr<boost::asio::ssl::context> TcpServer::getSslContext(const std::string& certFile, const std::string& privateKeyFile,
                                                                    const std::string& privateKeyFilePwd)
{
    return TcpConnection::makeSslContext(boost::asio::ssl::context::sslv23_server, certFile, privateKeyFile, privateKeyFilePwd);
}
#endif
} // namespace nsocket
