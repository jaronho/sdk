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

#if (1 == ENABLE_NSOCKET_OPENSSL)
static std::mutex s_mutexHandshakeCheck;
static std::unique_ptr<boost::asio::io_context> s_handshakeCheckContext = nullptr;
static std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> s_handshakeCheckWorker = nullptr;
std::unique_ptr<std::thread> s_handshakeCheckThread = nullptr; /* 握手检测线程 */
#endif

TcpServer::TcpServer(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, bool reuseAddr, size_t bz,
                     const std::chrono::steady_clock::duration& handshakeTimeout)
    : m_contextPool(std::make_shared<io_context_pool>(name, threadCount))
{
    m_bufferSize = bz;
    try
    {
        m_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(
            m_contextPool->getContext(), boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host.c_str()), port), reuseAddr);
#if (1 == ENABLE_NSOCKET_OPENSSL)
        {
            /* 创建定时器线程 */
            std::lock_guard<std::mutex> locker(s_mutexHandshakeCheck);
            if (!s_handshakeCheckContext)
            {
                s_handshakeCheckContext = std::make_unique<boost::asio::io_context>();
            }
            if (!s_handshakeCheckWorker)
            {
                s_handshakeCheckWorker = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
                    boost::asio::make_work_guard(*s_handshakeCheckContext));
            }
            if (!s_handshakeCheckThread)
            {
                s_handshakeCheckThread = std::make_unique<std::thread>([name] {
                    setThreadName(name + "::hs"); /* 设置线程名称 */
                    s_handshakeCheckContext->run();
                });
                s_handshakeCheckThread->detach();
            }
        }
        m_handshakeTimeout = handshakeTimeout > std::chrono::seconds(1) ? handshakeTimeout : std::chrono::seconds(1);
#endif
        m_host = host;
    }
    catch (const std::exception& e)
    {
        m_acceptor = nullptr;
        m_errorMsg = std::string("exception: ") + e.what();
    }
    catch (...)
    {
        m_acceptor = nullptr;
        m_errorMsg = "unknown exception";
    }
}

TcpServer::~TcpServer()
{
    stop();
}

void TcpServer::setNewConnectionCallback(const TCP_SRV_CONN_NEW_CALLBACK& onNewCb)
{
    m_onNewConnectionCallback = onNewCb;
}

void TcpServer::setHandshakeOkCallback(const TLS_SRV_HANDSHAKE_OK_CALLBACK& onHandshakeOkCb)
{
    m_onHandshakeOkCallback = onHandshakeOkCb;
}

void TcpServer::setHandshakeFailCallback(const TLS_SRV_HANDSHAKE_FAIL_CALLBACK& onHandshakeFailCb)
{
    m_onHandshakeFailCallback = onHandshakeFailCb;
}

void TcpServer::setConnectionDataCallback(const TCP_SRV_CONN_DATA_CALLBACK& onDataCb)
{
    m_onConnectionDataCallback = onDataCb;
}

void TcpServer::setConnectionCloseCallback(const TCP_SRV_CONN_CLOSE_CALLBACK& onCloseCb)
{
    m_onConnectionCloseCallback = onCloseCb;
}

bool TcpServer::run(bool sslOn, int sslWay, int certFmt, const std::string& certFile, const std::string& pkFile, const std::string& pkPwd)
{
    sslWay = (1 == sslWay || 2 == sslWay) ? sslWay : 1;
    certFmt = (1 == certFmt || 2 == certFmt) ? certFmt : 2;
    if (m_running)
    {
        return true;
    }
    std::shared_ptr<io_context_pool> contextPool = nullptr;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        contextPool = m_contextPool;
        acceptor = m_acceptor;
    }
    if (acceptor)
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        if (sslOn)
        {
            std::shared_ptr<boost::asio::ssl::context> sslContext = nullptr;
            if (1 == sslWay)
            {
                sslContext = TcpConnection::makeSsl1WayContextServer(boost::asio::ssl::context::sslv23_server,
                                                                     1 == certFmt ? boost::asio::ssl::context::file_format::asn1
                                                                                  : boost::asio::ssl::context::file_format::pem,
                                                                     certFile, pkFile, pkPwd, true);
            }
            else
            {
                sslContext = TcpConnection::makeSsl2WayContext(boost::asio::ssl::context::sslv23_server,
                                                               1 == certFmt ? boost::asio::ssl::context::file_format::asn1
                                                                            : boost::asio::ssl::context::file_format::pem,
                                                               certFile, pkFile, pkPwd, true);
            }
            if (sslContext)
            {
                auto sessionIdCtx = std::to_string(acceptor->local_endpoint().port()) + ':';
                sessionIdCtx.append(m_host.rbegin(), m_host.rend());
                SSL_CTX_set_session_id_context(sslContext->native_handle(), (const unsigned char*)sessionIdCtx.data(),
                                               std::min<size_t>(sessionIdCtx.size(), SSL_MAX_SSL_SESSION_ID_LENGTH));
            }
            {
                std::lock_guard<std::mutex> locker(m_mutex);
                m_sslContext = sslContext;
            }
        }
#endif
        if (contextPool)
        {
            contextPool->start();
            doAccept();
            m_running = true;
            return true;
        }
    }
    return false;
}

void TcpServer::stop()
{
    if (!m_running)
    {
        return;
    }
    std::shared_ptr<io_context_pool> contextPool = nullptr;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        contextPool = m_contextPool;
        m_contextPool.reset();
        acceptor = m_acceptor;
        m_acceptor.reset();
    }
    if (acceptor)
    {
        acceptor->close();
    }
    if (contextPool)
    {
        contextPool->join();
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexConnectionMap);
        m_connectionMap.clear();
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexHandshakeMap);
        m_handshakeMap.clear();
    }
    m_running = false;
}

bool TcpServer::isValid(std::string* errorMsg)
{
    if (errorMsg)
    {
        (*errorMsg) = m_errorMsg;
    }
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        acceptor = m_acceptor;
    }
    return (acceptor ? true : false);
}

bool TcpServer::isEnableSSL()
{
#if (1 == ENABLE_NSOCKET_OPENSSL)
    std::shared_ptr<boost::asio::ssl::context> sslContext = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        sslContext = m_sslContext;
    }
    return (sslContext ? true : false);
#else
    return false;
#endif
}

bool TcpServer::isRunning() const
{
    return m_running;
}

void TcpServer::doAccept()
{
    std::shared_ptr<io_context_pool> contextPool = nullptr;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        contextPool = m_contextPool;
        acceptor = m_acceptor;
    }
    if (contextPool && acceptor)
    {
        const std::weak_ptr<TcpServer> wpSelf = shared_from_this();
        acceptor->async_accept(contextPool->getContext(), [wpSelf](boost::system::error_code code, boost::asio::ip::tcp::socket socket) {
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
    std::shared_ptr<SocketTcpBase> socketPtr = nullptr;
#if (1 == ENABLE_NSOCKET_OPENSSL)
    std::shared_ptr<boost::asio::ssl::context> sslContext = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        sslContext = m_sslContext;
    }
    if (sslContext) /* 启用TLS */
    {
        socketPtr = std::make_shared<SocketTls>(std::move(socket), *sslContext);
    }
#endif
    if (!socketPtr)
    {
        socketPtr = std::make_shared<SocketTcp>(std::move(socket));
    }
    auto conn = std::make_shared<TcpConnection>(socketPtr, true, m_bufferSize);
    {
        std::lock_guard<std::mutex> locker(m_mutexConnectionMap);
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
        if (self && conn)
        {
            self->handleConnectionResult(conn, point, code);
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

void TcpServer::handleConnectionResult(const std::shared_ptr<TcpConnection>& conn, const boost::asio::ip::tcp::endpoint& point,
                                       const boost::system::error_code& code)
{
    if (code) /* 断开连接 */
    {
        {
            std::lock_guard<std::mutex> locker(m_mutexConnectionMap);
            auto iter = m_connectionMap.find(conn->getId());
            if (m_connectionMap.end() == iter)
            {
                return;
            }
            m_connectionMap.erase(iter);
        }
        if (m_onConnectionCloseCallback)
        {
            m_onConnectionCloseCallback(conn->getId(), point, code);
        }
    }
    else /* 连接成功 */
    {
        if (m_onNewConnectionCallback)
        {
            m_onNewConnectionCallback(conn);
        }
        if (conn->isEnableSSL()) /* 启用TLS */
        {
            handleHandshake(conn, point);
        }
    }
}

void TcpServer::handleHandshake(const std::shared_ptr<TcpConnection>& conn, const boost::asio::ip::tcp::endpoint& point)
{
#if (1 == ENABLE_NSOCKET_OPENSSL)
    std::shared_ptr<boost::asio::steady_timer> tm = nullptr;
    {
        std::lock_guard<std::mutex> locker(s_mutexHandshakeCheck);
        tm = std::make_shared<boost::asio::steady_timer>(*s_handshakeCheckContext);
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexHandshakeMap);
        if (m_handshakeMap.end() == m_handshakeMap.find(conn->getId()))
        {
            m_handshakeMap.insert(std::make_pair(conn->getId(), tm));
        }
        else
        {
            tm.reset();
        }
    }
    const std::weak_ptr<TcpServer> wpSelf = shared_from_this();
    const std::weak_ptr<TcpConnection> wpConn = conn;
    if (tm)
    {
        tm->expires_from_now(m_handshakeTimeout);
        tm->async_wait([wpSelf, wpConn, point](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            const auto conn = wpConn.lock();
            if (self && conn)
            {
                self->handleHandshakeResult(conn, point, boost::system::errc::make_error_code(boost::system::errc::timed_out));
            }
        });
    }
    conn->handshake(
        boost::asio::ssl::stream_base::server,
        [wpSelf, wpConn, point](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            const auto conn = wpConn.lock();
            if (self && conn)
            {
                self->handleHandshakeResult(conn, point, code);
            }
        },
        true);
#endif
}

void TcpServer::handleHandshakeResult(const std::shared_ptr<TcpConnection>& conn, const boost::asio::ip::tcp::endpoint& point,
                                      const boost::system::error_code& code)
{
    std::shared_ptr<boost::asio::steady_timer> tm = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexHandshakeMap);
        auto iter = m_handshakeMap.find(conn->getId());
        if (m_handshakeMap.end() == iter)
        {
            return;
        }
        tm = iter->second;
        m_handshakeMap.erase(iter);
    }
    if (tm)
    {
        tm->cancel();
    }
    if (code) /* 握手失败 */
    {
        if (m_onHandshakeFailCallback)
        {
            m_onHandshakeFailCallback(conn->getId(), point, code);
        }
        if (conn->isConnected())
        {
            conn->close();
        }
        else
        {
            {
                std::lock_guard<std::mutex> locker(m_mutexConnectionMap);
                auto iter = m_connectionMap.find(conn->getId());
                if (m_connectionMap.end() == iter)
                {
                    return;
                }
                m_connectionMap.erase(iter);
            }
            if (m_onConnectionCloseCallback)
            {
                m_onConnectionCloseCallback(conn->getId(), point, code);
            }
        }
    }
    else /* 握手成功 */
    {
        if (m_onHandshakeOkCallback)
        {
            m_onHandshakeOkCallback(conn);
        }
    }
}
} // namespace nsocket
