#include "server.h"

namespace nsocket
{
namespace http
{
Server::Server(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, bool reuseAddr, size_t bz,
               const std::chrono::steady_clock::duration& handshakeTimeout)
    : m_name(name)
    , m_threadCount(threadCount)
    , m_host(host)
    , m_port(port)
    , m_reuseAddr(reuseAddr)
    , m_bufferSize(bz)
    , m_handshakeTimeout(handshakeTimeout)
{
}

Server::~Server()
{
    stop();
}

void Server::setDefaultRouterCallback(const std::function<void(uint64_t cid, const REQUEST_PTR& req, const Connector& conn)>& cb)
{
    std::lock_guard<std::mutex> locker(m_mutexDefaultRouterCb);
    m_defaultRouterCb = cb;
}

std::vector<std::string> Server::addRouter(const std::vector<Method>& methods, const std::vector<std::string>& uriList,
                                           const std::shared_ptr<Router>& router)
{
    std::vector<std::string> repeatUriList;
    {
        std::lock_guard<std::mutex> locker(m_mutexRouterMap);
        for (auto uri : uriList)
        {
            if (uri.empty() || '/' != uri[0])
            {
                uri.insert(uri.begin(), '/');
            }
            if (m_routerMap.end() == m_routerMap.find(uri)) /* 新的URI */
            {
                router->m_methods = methods;
                m_routerMap.insert(std::make_pair(uri, router));
            }
            else /* URI已添加过 */
            {
                repeatUriList.emplace_back(uri);
            }
        }
    }
    return repeatUriList;
}

bool Server::run(bool sslOn, int sslWay, int certFmt, const std::string& certFile, const std::string& pkFile, const std::string& pkPwd,
                 std::string* errDesc)
{
    auto tcpServer = std::make_shared<TcpServer>(m_name, m_threadCount, m_host, m_port, m_reuseAddr, m_bufferSize, m_handshakeTimeout);
    if (!tcpServer->isValid(errDesc))
    {
        return false;
    }
    tcpServer->setNewConnectionCallback([&, tcpServer](const std::weak_ptr<TcpConnection>& wpConn) {
        if (!tcpServer->isEnableSSL())
        {
            handleNewConnection(wpConn);
        }
    });
    tcpServer->setHandshakeOkCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn) { handleNewConnection(wpConn); });
    tcpServer->setConnectionDataCallback(
        [&](const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data) { handleConnectionData(wpConn, data); });
    tcpServer->setConnectionCloseCallback([&](uint64_t cid, const boost::asio::ip::tcp::endpoint& point,
                                              const boost::system::error_code& code) { handleConnectionClose(cid); });
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpServer);
        m_tcpServer = tcpServer;
    }
    return tcpServer->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
}

void Server::stop()
{
    std::shared_ptr<TcpServer> tcpServer = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpServer);
        tcpServer = m_tcpServer;
        m_tcpServer.reset();
    }
    if (tcpServer)
    {
        tcpServer->stop();
    }
}

bool Server::isRunning()
{
    std::shared_ptr<TcpServer> tcpServer = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpServer);
        tcpServer = m_tcpServer;
    }
    if (tcpServer && tcpServer->isRunning())
    {
        return true;
    }
    return false;
}

void Server::handleNewConnection(const std::weak_ptr<TcpConnection>& wpConn)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        if (m_sessionMap.end() == m_sessionMap.find(conn->getId()))
        {
            auto session = std::make_shared<Session>();
            session->wpConn = wpConn;
            session->req = std::make_shared<Request>();
            session->req->host = conn->getRemoteEndpoint().address().to_string();
            session->req->port = (int)conn->getRemoteEndpoint().port();
            m_sessionMap.insert(std::make_pair(conn->getId(), session));
        }
    }
}

void Server::handleConnectionData(const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::shared_ptr<Session> session = nullptr;
        {
            /* 限定锁区间, 避免阻塞其他连接, 提高并发性 */
            std::lock_guard<std::mutex> locker(m_mutexSessionMap);
            auto iter = m_sessionMap.find(conn->getId());
            if (m_sessionMap.end() != iter)
            {
                session = iter->second;
            }
        }
        if (session)
        {
            int used = session->req->parse(
                data.data(), data.size(), [&]() { handleReqHead(session); },
                [&](size_t offset, const unsigned char* data, int dataLen) { handleReqContent(session, offset, data, dataLen); },
                [&]() { handleReqFinish(session); });
            if (used <= 0) /* 解析失败 */
            {
                conn->close();
            }
        }
    }
}

void Server::handleConnectionClose(uint64_t cid)
{
    std::lock_guard<std::mutex> locker(m_mutexSessionMap);
    auto iter = m_sessionMap.find(cid);
    if (m_sessionMap.end() != iter)
    {
        m_sessionMap.erase(iter);
    }
}

void Server::handleReqHead(const std::shared_ptr<Session>& session)
{
    const auto conn = session->wpConn.lock();
    if (conn)
    {
        std::shared_ptr<Router> router = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexRouterMap);
            auto iter = m_routerMap.find(session->req->uri);
            if (m_routerMap.end() != iter)
            {
                router = iter->second;
            }
        }
        if (router) /* 找到路由 */
        {
            /* 判断是否允许请求的方法 */
            if (router->m_methods.empty())
            {
                session->req->isMethodAllowed = true;
            }
            else
            {
                session->req->isMethodAllowed = false;
                for (auto method : router->m_methods)
                {
                    if (case_insensitive_equal(session->req->method, method_desc(method)))
                    {
                        session->req->isMethodAllowed = true;
                        break;
                    }
                }
            }
            if (session->req->isMethodAllowed) /* 允许方法, 响应头数据 */
            {
                router->onReqHead(conn->getId(), session->req);
            }
        }
    }
}

void Server::handleReqContent(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen)
{
    if (session->req->isMethodAllowed) /* 允许方法 */
    {
        const auto conn = session->wpConn.lock();
        if (conn)
        {
            std::shared_ptr<Router> router = nullptr;
            {
                std::lock_guard<std::mutex> locker(m_mutexRouterMap);
                auto iter = m_routerMap.find(session->req->uri);
                if (m_routerMap.end() != iter)
                {
                    router = iter->second;
                }
            }
            if (router) /* 找到路由 */
            {
                router->onReqContent(conn->getId(), session->req, offset, data, dataLen);
            }
        }
    }
}

void Server::handleReqFinish(const std::shared_ptr<Session>& session)
{
    const auto conn = session->wpConn.lock();
    if (conn)
    {
        std::shared_ptr<Router> router = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexRouterMap);
            auto iter = m_routerMap.find(session->req->uri);
            if (m_routerMap.end() != iter)
            {
                router = iter->second;
            }
        }
        std::shared_ptr<Response> resp = nullptr;
        if (router) /* 找到路由 */
        {
            if (session->req->isMethodAllowed) /* 允许方法 */
            {
                router->onResponse(conn->getId(), session->req,
                                   Connector([&, wpConn = session->wpConn](const std::vector<unsigned char>& data,
                                                                           const TCP_SEND_CALLBACK& cb) { sendResponse(wpConn, data, cb); },
                                             [&, wpConn = session->wpConn]() { closeConnection(wpConn); }));
                return;
            }
            else if (router->methodNotAllowedCb) /* 方法不允许 */
            {
                router->methodNotAllowedCb(
                    conn->getId(), session->req,
                    Connector([&, wpConn = session->wpConn](const std::vector<unsigned char>& data,
                                                            const TCP_SEND_CALLBACK& cb) { sendResponse(wpConn, data, cb); },
                              [&, wpConn = session->wpConn]() { closeConnection(wpConn); }));
                return;
            }
            resp = std::make_shared<Response>();
            resp->statusCode = StatusCode::client_error_method_not_allowed;
        }
        else /* 找不到路由(进入默认路由) */
        {
            std::function<void(uint64_t cid, const REQUEST_PTR& req, const Connector& conn)> defaultRouterCb = nullptr;
            {
                std::lock_guard<std::mutex> locker(m_mutexDefaultRouterCb);
                defaultRouterCb = m_defaultRouterCb;
            }
            if (defaultRouterCb)
            {
                defaultRouterCb(conn->getId(), session->req,
                                Connector([&, wpConn = session->wpConn](const std::vector<unsigned char>& data,
                                                                        const TCP_SEND_CALLBACK& cb) { sendResponse(wpConn, data, cb); },
                                          [&, wpConn = session->wpConn]() { closeConnection(wpConn); }));
                return;
            }
            resp = std::make_shared<Response>();
            resp->statusCode = StatusCode::client_error_not_found;
        }
        /* 发送响应数据 */
        sendResponse(session->wpConn, resp ? resp->pack() : std::vector<unsigned char>(),
                     [&, wpConn = session->wpConn](const boost::system::error_code& code, size_t length) {
                         closeConnection(wpConn); /* 关闭连接 */
                     });
    }
}

void Server::sendResponse(const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& cb)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        conn->send(data, cb);
    }
}

void Server::closeConnection(const std::weak_ptr<TcpConnection>& wpConn)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        conn->close(); /* 响应结束后, 需要关闭连接(有些客户端不会主动关闭连接) */
    }
}
} // namespace http
} // namespace nsocket
