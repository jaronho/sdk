#include "tcp_server.h"

namespace nsocket
{
TcpServer::TcpServer(const std::string& host, unsigned int port)
#if (1 == ENABLE_SOCKET_OPENSSL)
    : m_sslContext(nullptr)
#endif
{
    try
    {
        m_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(
            m_ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host.c_str()), port));
    }
    catch (...)
    {
        m_acceptor.reset();
    }
}

#if (1 == ENABLE_SOCKET_OPENSSL)
void TcpServer::run(const std::shared_ptr<boost::asio::ssl::context>& sslContext)
#else
void TcpServer::run()
#endif
{
    if (m_acceptor)
    {
#if (1 == ENABLE_SOCKET_OPENSSL)
        m_sslContext = sslContext;
#endif
        doAccept();
        m_ioContext.run();
    }
}

void TcpServer::stop()
{
    m_ioContext.stop();
}

void TcpServer::setNewConnectionCallback(const TCP_CONN_NEW_CALLBACK& onNewCb)
{
    m_onNewConnectionCallback = onNewCb;
}

void TcpServer::setRecvConnectionDataCallback(const TCP_CONN_RECV_DATA_CALLBACK& onRecvDataCb)
{
    m_onRecvConnectionDataCallback = onRecvDataCb;
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
            if (!code) /* 有新连接进来 */
            {
                /* 创建新会话 */
                std::shared_ptr<TcpSession> session;
#if (1 == ENABLE_SOCKET_OPENSSL)
                if (self->m_sslContext) /* 启用TLS */
                {
                    session = std::make_shared<TcpSession>(std::make_shared<SocketTls>(std::move(socket), *(self->m_sslContext)));
                }
                else /* 不启用TLS */
                {
#endif
                    session = std::make_shared<TcpSession>(std::make_shared<SocketTcp>(std::move(socket)));
#if (1 == ENABLE_SOCKET_OPENSSL)
                }
#endif
                auto remoteEndpoint = session->getRemoteEndpoint();
                /* 创建发送处理句柄 */
                auto sendHandler = [wpSelf, session](const std::vector<unsigned char>& data, const TCP_CONN_SEND_CALLBACK& onSendCb) {
                    const auto self = wpSelf.lock();
                    if (self)
                    {
                        self->doSend(session, data, onSendCb);
                    }
                };
                /* 设置连接回调 */
                session->setConnectCallback([wpSelf, remoteEndpoint, sendHandler](const boost::system::error_code& code) {
                    if (code) /* 断开连接 */
                    {
                        const auto self = wpSelf.lock();
                        if (self && self->m_onConnectionCloseCallback)
                        {
                            self->m_onConnectionCloseCallback(remoteEndpoint, code);
                        }
                    }
                });
                /* 设置接收数据回调 */
                session->setRecvDataCallback([wpSelf, remoteEndpoint, sendHandler](const std::vector<unsigned char>& data) {
                    const auto self = wpSelf.lock();
                    if (self && self->m_onRecvConnectionDataCallback)
                    {
                        self->m_onRecvConnectionDataCallback(remoteEndpoint, data, sendHandler);
                    }
                });
                /* 开始会话 */
#if (1 == ENABLE_SOCKET_OPENSSL)
                if (self->m_sslContext) /* 启用TLS */
                {
                    session->handshake(boost::asio::ssl::stream_base::server,
                                       [wpSelf, remoteEndpoint, session, sendHandler](const boost::system::error_code& code) {
                                           if (!code) /* 握手成功 */
                                           {
                                               const auto self = wpSelf.lock();
                                               if (self && self->m_onNewConnectionCallback)
                                               {
                                                   self->m_onNewConnectionCallback(remoteEndpoint, sendHandler);
                                               }
                                               if (session)
                                               {
                                                   session->recv(); /* 开始接收数据 */
                                               }
                                           }
                                       }); /* 需要握手 */
                }
                else /* 不启用TLS */
                {
#endif
                    if (self->m_onNewConnectionCallback)
                    {
                        self->m_onNewConnectionCallback(remoteEndpoint, sendHandler);
                    }
                    session->recv(); /* 开始接收数据 */
#if (1 == ENABLE_SOCKET_OPENSSL)
                }
#endif
            }
            /* 继续接收下一个连接 */
            self->doAccept();
        }
    });
}

void TcpServer::doSend(const std::shared_ptr<TcpSession>& connection, const std::vector<unsigned char>& data,
                       const TCP_CONN_SEND_CALLBACK& onSendCb)
{
    if (connection)
    {
        const std::weak_ptr<TcpServer> wpSelf = shared_from_this();
        connection->send(data, [wpSelf, connection, onSendCb](const boost::system::error_code& code, std::size_t length) {
            if (connection)
            {
                auto remoteEndpoint = connection->getRemoteEndpoint();
                if (onSendCb)
                {
                    onSendCb(remoteEndpoint, code, length);
                }
                if (code) /* 发送错误 */
                {
                    connection->close(); /* 断开连接 */
                    const auto self = wpSelf.lock();
                    if (self && self->m_onConnectionCloseCallback)
                    {
                        self->m_onConnectionCloseCallback(remoteEndpoint, code);
                    }
                }
            }
        });
    }
}
} // namespace nsocket
