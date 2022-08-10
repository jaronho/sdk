#include "server.h"

namespace nsocket
{
namespace http
{
Server::Server(const std::string& name, size_t threadCount, const std::string& host, unsigned int port, bool reuseAddr, size_t bz)
{
    m_tcpServer = std::make_shared<TcpServer>(name, threadCount, host, port, reuseAddr, bz);
    m_tcpServer->setNewConnectionCallback([&](const std::weak_ptr<TcpConnection>& wpConn) { handleNewConnection(wpConn); });
    m_tcpServer->setConnectionDataCallback(
        [&](const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data) { handleConnectionData(wpConn, data); });
    m_tcpServer->setConnectionCloseCallback([&](int64_t cid, const boost::asio::ip::tcp::endpoint& point,
                                                const boost::system::error_code& code) { handleConnectionClose(cid); });
}

bool Server::isValid(std::string* errorMsg) const
{
    if (m_tcpServer && m_tcpServer->isValid(errorMsg))
    {
        return true;
    }
    return false;
}

bool Server::isRunning() const
{
    if (m_tcpServer && m_tcpServer->isRunning())
    {
        return true;
    }
    return false;
}

void Server::setRouterNotFoundCallback(const std::function<void(const REQUEST_PTR& req)>& cb)
{
    m_routerNotFoundCb = cb;
}

std::vector<std::string> Server::addRouter(const std::vector<Method>& methods, const std::vector<std::string>& uriList,
                                           std::shared_ptr<Router> router)
{
    std::vector<std::string> repeatUriList;
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
    return repeatUriList;
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
bool Server::run(const std::shared_ptr<boost::asio::ssl::context>& sslContext)
#else
bool Server::run()
#endif
{
    if (m_tcpServer)
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        return m_tcpServer->run(sslContext);
#else
        return m_tcpServer->run();
#endif
    }
    return false;
}

void Server::handleNewConnection(const std::weak_ptr<TcpConnection>& wpConn)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::lock_guard<std::mutex> locker(m_mutex);
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
            std::lock_guard<std::mutex> locker(m_mutex);
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

void Server::handleConnectionClose(int64_t cid)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    auto iter = m_sessionMap.find(cid);
    if (m_sessionMap.end() != iter)
    {
        m_sessionMap.erase(iter);
    }
}

void Server::handleReqHead(const std::shared_ptr<Session>& session)
{
    auto req = session->req;
#if 0
    /* 信息打印 */
    printf("\n======================================================================\n");
    printf("===  Method: %s\n", req->method.c_str());
    printf("===     Uri: %s\n", req->uri.c_str());
    if (!req->queries.empty())
    {
        printf("=== Queries:\n");
        for (auto iter = req->queries.begin(); req->queries.end() != iter; ++iter)
        {
            printf("             [%s] = %s\n", iter->first.c_str(), iter->second.c_str());
        }
    }
    printf("=== Version: %s\n", req->version.c_str());
    if (!req->headers.empty())
    {
        printf("=== Headers:\n");
        for (auto iter = req->headers.begin(); req->headers.end() != iter; ++iter)
        {
            printf("             [%s] = %s\n", iter->first.c_str(), iter->second.c_str());
        }
    }
    if (req->getContentLength() > 0)
    {
        printf("=== Content:\n");
#endif
    const auto conn = session->wpConn.lock();
    if (conn)
    {
        /* 路由 */
        auto iter = m_routerMap.find(req->uri);
        if (m_routerMap.end() == iter) /* 未找到 */
        {
            if (m_routerNotFoundCb)
            {
                m_routerNotFoundCb(req);
            }
        }
        else /* 找到 */
        {
            /* 扩展数据接收缓冲区 */
            int bufferSize = req->getContentLength();
            if (bufferSize > 1024)
            {
                static const int MAX_BUFFER_SIZE = 65536; /* 64Kb */
                if (bufferSize > MAX_BUFFER_SIZE) /* 限制上限 */
                {
                    bufferSize = MAX_BUFFER_SIZE;
                }
                conn->resizeBuffer(bufferSize);
            }
            /* 判断是否允许请求的方法 */
            if (iter->second->m_methods.empty())
            {
                req->isMethodAllowed = true;
            }
            else
            {
                req->isMethodAllowed = false;
                for (auto method : iter->second->m_methods)
                {
                    if (case_insensitive_equal(req->method, method_desc(method)))
                    {
                        req->isMethodAllowed = true;
                        break;
                    }
                }
            }
            if (req->isMethodAllowed) /* 允许方法, 响应头数据 */
            {
                iter->second->onReqHead(conn->getId(), req);
            }
            else /* 方法不允许 */
            {
                iter->second->onMethodNotAllowed(conn->getId(), req);
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
            /* 路由 */
            auto iter = m_routerMap.find(session->req->uri);
            if (m_routerMap.end() != iter)
            {
                iter->second->onReqContent(conn->getId(), session->req, offset, data, dataLen);
            }
        }
    }
}

void Server::handleReqFinish(const std::shared_ptr<Session>& session)
{
    const auto conn = session->wpConn.lock();
    if (conn)
    {
        std::shared_ptr<Response> resp = nullptr;
        if (session->req->isMethodAllowed) /* 允许方法 */
        {
            /* 路由 */
            auto iter = m_routerMap.find(session->req->uri);
            if (m_routerMap.end() == iter) /* 找不到路由 */
            {
                resp = std::make_shared<Response>();
                resp->statusCode = StatusCode::client_error_not_found;
            }
            else
            {
                iter->second->onResponse(conn->getId(), session->req, [&, wpConn = session->wpConn](const std::shared_ptr<Response>& resp) {
                    sendResponse(wpConn, resp);
                });
                return;
            }
        }
        else /* 方法不允许 */
        {
            resp = std::make_shared<Response>();
            resp->statusCode = StatusCode::client_error_method_not_allowed;
        }
        /* 发送响应数据 */
        sendResponse(session->wpConn, resp);
    }
}

void Server::sendResponse(const std::weak_ptr<TcpConnection>& wpConn, std::shared_ptr<Response> resp)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        if (!resp)
        {
            resp = std::make_shared<Response>();
            resp->statusCode = StatusCode::success_ok;
        }
        std::vector<unsigned char> data;
        Response::pack(*resp, data);
        conn->sendAsync(data, [&, wpConn](const boost::system::error_code& code, size_t length) {
            const auto conn = wpConn.lock();
            if (conn)
            {
                conn->close(); /* 响应结束后, 需要关闭连接(有些客户端不会主动关闭连接), TODO: 后续可能要做Keep-Alive */
            }
        });
    }
}
} // namespace http
} // namespace nsocket
