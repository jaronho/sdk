#include "tcp_server.h"

namespace nsocket
{
TcpServer::TcpServer(const std::string& host, unsigned int port, bool reuseAddr, size_t bz)
#if (1 == ENABLE_NSOCKET_OPENSSL)
    : m_sslContext(nullptr)
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

#if (1 == ENABLE_NSOCKET_OPENSSL)
void TcpServer::run(const std::shared_ptr<boost::asio::ssl::context>& sslContext)
#else
void TcpServer::run()
#endif
{
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
        m_ioContext.run();
    }
}

void TcpServer::stop()
{
    if (m_acceptor)
    {
        m_acceptor->close();
        m_acceptor = nullptr;
    }
    m_ioContext.stop();
    m_sessionMap.clear();
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
            if (!code) /* 有新连接进来 */
            {
                /* 创建新会话 */
                std::shared_ptr<TcpSession> session;
#if (1 == ENABLE_NSOCKET_OPENSSL)
                if (self->m_sslContext) /* 启用TLS */
                {
                    session = std::make_shared<TcpSession>(std::make_shared<SocketTls>(std::move(socket), *(self->m_sslContext)), true,
                                                           self->m_bufferSize);
                }
                else /* 不启用TLS */
                {
#endif
                    session = std::make_shared<TcpSession>(std::make_shared<SocketTcp>(std::move(socket)), true, self->m_bufferSize);
#if (1 == ENABLE_NSOCKET_OPENSSL)
                }
#endif
                if (self->m_sessionMap.end() == self->m_sessionMap.find(session->getId()))
                {
                    self->m_sessionMap.insert(std::make_pair(session->getId(), session));
                }
                const std::weak_ptr<TcpSession> wpSession = session;
                /* 设置连接回调 */
                session->setConnectCallback([wpSelf, wpSession](const boost::system::error_code& code) {
                    if (code) /* 断开连接 */
                    {
                        const auto self = wpSelf.lock();
                        const auto session = wpSession.lock();
                        if (self && session)
                        {
                            auto iter = self->m_sessionMap.find(session->getId());
                            if (self->m_sessionMap.end() != iter)
                            {
                                self->m_sessionMap.erase(iter);
                            }
                            if (self->m_onConnectionCloseCallback)
                            {
                                self->m_onConnectionCloseCallback(session->getId(), session->getRemoteEndpoint(), code);
                            }
                        }
                    }
                });
                /* 设置数据回调 */
                session->setDataCallback([wpSelf, wpSession](const std::vector<unsigned char>& data) {
                    const auto self = wpSelf.lock();
                    if (self && self->m_onConnectionDataCallback)
                    {
                        self->m_onConnectionDataCallback(wpSession, data);
                    }
                });
                /* 开始会话 */
#if (1 == ENABLE_NSOCKET_OPENSSL)
                if (self->m_sslContext) /* 启用TLS */
                {
                    session->handshake(boost::asio::ssl::stream_base::server, [wpSelf, wpSession](const boost::system::error_code& code) {
                        if (!code) /* 握手成功 */
                        {
                            const auto session = wpSession.lock();
                            if (session)
                            {
                                const auto self = wpSelf.lock();
                                if (self && self->m_onNewConnectionCallback)
                                {
                                    self->m_onNewConnectionCallback(wpSession);
                                }
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
                        self->m_onNewConnectionCallback(wpSession);
                    }
                    session->recv(); /* 开始接收数据 */
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
    return TcpSession::makeSslContext(boost::asio::ssl::context::sslv23_server, certFile, privateKeyFile, privateKeyFilePwd);
}
#endif
} // namespace nsocket
