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
                                                const boost::system::error_code& code) { handleConnectionClose(sid); });
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
    }
}

void Server::handleConnectionData(const std::weak_ptr<TcpSession>& wpSession, const std::vector<unsigned char>& data)
{
    const auto tcpSession = wpSession.lock();
    if (tcpSession)
    {
    }
}

void Server::handleConnectionClose(int64_t sid) {}
} // namespace ws
} // namespace nsocket
