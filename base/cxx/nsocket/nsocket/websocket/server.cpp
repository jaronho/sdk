#include "server.h"

namespace nsocket
{
namespace ws
{
Server::Server(const std::string& host, unsigned int port)
{
    m_tcpServer = std::make_shared<TcpServer>(host, port, true, 1024);
    m_tcpServer->setNewConnectionCallback([&](const std::weak_ptr<TcpSession>& wpSession) { handleNewConnection(wpSession); });
    m_tcpServer->setConnectionDataCallback(
        [&](const std::weak_ptr<TcpSession>& wpSession, const std::vector<unsigned char>& data) { handleConnectionData(wpSession, data); });
    m_tcpServer->setConnectionCloseCallback([&](int64_t sid, const boost::asio::ip::tcp::endpoint& point,
                                                const boost::system::error_code& code) { handleConnectionClose(sid, point, code); });
}

void Server::setRequestCallback(const WS_REQUEST_CALLBACK& cb)
{
    m_onRequestCallback = cb;
}

void Server::setOpenCallback(const WS_OPEN_CALLBACK& cb)
{
    m_onOpenCallback = cb;
}

void Server::setCloseCallback(const WS_CLOSE_CALLBACK& cb)
{
    m_onCloseCallback = cb;
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

void Server::handleNewConnection(const std::weak_ptr<TcpSession>& wpSession)
{
    const auto tcpSession = wpSession.lock();
    if (tcpSession)
    {
        auto point = tcpSession->getRemoteEndpoint();
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("============================== on new connection [%lld] [%s:%d]\n", tcpSession->getId(), clientHost.c_str(), clientPort);
        std::lock_guard<std::mutex> locker(m_mutex);
        if (m_sessionMap.end() == m_sessionMap.find(tcpSession->getId()))
        {
            auto session = std::make_shared<Session>();
            session->wpTcpSession = wpSession;
            session->req = std::make_shared<Request>();
            m_sessionMap.insert(std::make_pair(tcpSession->getId(), session));
        }
    }
}

void Server::handleConnectionData(const std::weak_ptr<TcpSession>& wpSession, const std::vector<unsigned char>& data)
{
    const auto tcpSession = wpSession.lock();
    if (tcpSession)
    {
        auto point = tcpSession->getRemoteEndpoint();
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("++++++++++ on recv data [%lld] [%s:%d], length: %d\n", tcpSession->getId(), clientHost.c_str(), clientPort,
               (int)data.size());
        /* 以十六进制格式打印数据 */
        printf("+++++ [hex format]\n");
        for (size_t i = 0; i < data.size(); ++i)
        {
            printf("%02X ", data[i]);
        }
        printf("\n");
        /* 以字符串格式打印数据 */
        printf("+++++ [string format]\n");
        std::string str(data.begin(), data.end());
        printf("%s", str.c_str());
        printf("\n");
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_sessionMap.find(tcpSession->getId());
        if (m_sessionMap.end() == iter)
        {
            return;
        }
        auto session = iter->second;
        if (session->req->isEnding()) /* 请求处理完毕, 后续收到的都是业务数据 */
        {
            printf("=========== xxxxx\n");
        }
        else /* 请求未处理结束, 需要继续解析 */
        {
            int used = session->req->parse(data.data(), data.size(), [&]() { handleRequest(session); });
            if (used <= 0) /* 解析失败 */
            {
                tcpSession->close();
            }
        }
    }
}

void Server::handleConnectionClose(int64_t sid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)
{
    std::string clientHost = point.address().to_string().c_str();
    int clientPort = (int)point.port();
    if (code)
    {
        printf("-------------------- on connection closed [%lld] [%s:%d] fail, %d, %s\n", sid, clientHost.c_str(), clientPort, code.value(),
               code.message().c_str());
    }
    else
    {
        printf("-------------------- on connection closed [%lld] [%s:%d]\n", sid, clientHost.c_str(), clientPort);
    }
    std::lock_guard<std::mutex> locker(m_mutex);
    auto iter = m_sessionMap.find(sid);
    if (m_sessionMap.end() != iter)
    {
        m_sessionMap.erase(iter);
    }
    auto session = iter->second;
    if (m_onCloseCallback)
    {
        m_onCloseCallback(session);
    }
}

void Server::handleRequest(const std::shared_ptr<Session>& session)
{
    const auto tcpSession = session->wpTcpSession.lock();
    if (tcpSession)
    {
        /* 收到客户端请求 */
        std::shared_ptr<Response> resp = nullptr;
        if (m_onRequestCallback)
        {
            resp = m_onRequestCallback(session);
        }
        if (!resp)
        {
            resp = std::make_shared<Response>();
        }
        std::vector<unsigned char> data;
        resp->create(data, session->req->getSecWebSocketKey());
        printf("+++++++++++++++++++++++++++++++++ response\n");
        printf("%s\n", std::string(data.begin(), data.end()).c_str());
        /* 响应客户端, 用于通知客户端WebSocket连接建立成功 */
        std::weak_ptr<Session> wpSession = session;
        tcpSession->send(data,
                         [&, wpSession, wpTcpSession = session->wpTcpSession](const boost::system::error_code& code, std::size_t length) {
                             if (code) /* 失败, 则需要关闭连接 */
                             {
                                 const auto tcpSession = wpTcpSession.lock();
                                 if (tcpSession)
                                 {
                                     tcpSession->close(); /* 响应结束后, 需要关闭连接(某些客户端不会主动关闭连接) */
                                 }
                             }
                             else /* 成功, 表示连接建立成功 */
                             {
                                 const auto session = wpSession.lock();
                                 if (session)
                                 {
                                     if (m_onOpenCallback)
                                     {
                                         m_onOpenCallback(session);
                                     }
                                 }
                             }
                         });
    }
}
} // namespace ws
} // namespace nsocket
