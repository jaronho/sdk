#include "server.h"

namespace nsocket
{
namespace http
{
Server::Server(const std::string& host, unsigned int port)
{
    m_tcpServer = std::make_shared<TcpServer>(host, port, true, 1024);
    m_tcpServer->setNewConnectionCallback([&](const std::weak_ptr<TcpConnection>& wpConn) { handleNewConnection(wpConn); });
    m_tcpServer->setConnectionDataCallback(
        [&](const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data) { handleConnectionData(wpConn, data); });
    m_tcpServer->setConnectionCloseCallback([&](int64_t cid, const boost::asio::ip::tcp::endpoint& point,
                                                const boost::system::error_code& code) { handleConnectionClose(cid); });
}

void Server::setRouterNotFoundCallback(const std::function<void(const REQUEST_PTR& req)>& cb)
{
    m_routerNotFoundCb = cb;
}

void Server::addRouter(const std::string& uri, const std::shared_ptr<Router>& router)
{
    if (m_routerMap.end() == m_routerMap.find(uri))
    {
        m_routerMap.insert(std::make_pair(uri, router));
    }
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
void Server::run(const std::shared_ptr<boost::asio::ssl::context>& sslContext)
#else
void Server::run()
#endif
{
    if (m_tcpServer)
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        m_tcpServer->run(sslContext);
#else
        m_tcpServer->run();
#endif
    }
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
            /* 响应头数据 */
            iter->second->onReqHead(conn->getId(), req);
        }
    }
}

void Server::handleReqContent(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen)
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

void Server::handleReqFinish(const std::shared_ptr<Session>& session)
{
    const auto conn = session->wpConn.lock();
    if (conn)
    {
        /* 路由 */
        auto iter = m_routerMap.find(session->req->uri);
        std::shared_ptr<Response> resp = nullptr;
        if (m_routerMap.end() == iter)
        {
            resp = std::make_shared<Response>();
            resp->statusCode = StatusCode::client_error_not_found;
        }
        else
        {
            resp = iter->second->onResponse(conn->getId(), session->req);
            if (!resp)
            {
                resp = std::make_shared<Response>();
                resp->statusCode = StatusCode::success_ok;
            }
        }
        /* 响应 */
        std::vector<unsigned char> data;
        Response::create(*resp, data);
        conn->send(data, [&, wpConn = session->wpConn](const boost::system::error_code& code, std::size_t length) {
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
