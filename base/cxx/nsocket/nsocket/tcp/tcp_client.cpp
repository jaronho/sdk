#include "tcp_client.h"

#include <functional>

namespace nsocket
{
TcpClient::TcpClient(size_t bz)
    : m_tcpSession(nullptr)
    , m_bufferSize(bz)
#if (1 == ENABLE_NSOCKET_OPENSSL)
    , m_sslContext(nullptr)
#endif
    , m_onConnectCallback(nullptr)
    , m_onDataCallback(nullptr)
    , m_runStatus(RunStatus::RUN_NONE)
{
}

void TcpClient::setConnectCallback(const TCP_CONNECT_CALLBACK& onConnectCb)
{
    m_onConnectCallback = onConnectCb;
}

void TcpClient::setDataCallback(const TCP_DATA_CALLBACK& onDataCb)
{
    m_onDataCallback = onDataCb;
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
void TcpClient::run(const std::string& host, unsigned int port, const std::shared_ptr<boost::asio::ssl::context>& sslContext)
#else
void TcpClient::run(const std::string& host, unsigned int port)
#endif
{
    if (isRunning())
    {
        return;
    }
    boost::system::error_code code;
    m_endpoints = boost::asio::ip::tcp::resolver(m_ioContext).resolve(host, std::to_string(port), code);
    m_endpointIter = m_endpoints.begin();
    if (code || m_endpoints.empty())
    {
        if (m_onConnectCallback)
        {
            m_onConnectCallback(code ? code : boost::system::errc::make_error_code(boost::system::errc::address_not_available));
        }
    }
    else
    {
        boost::asio::ip::tcp::socket socket(m_ioContext);
#if (1 == ENABLE_NSOCKET_OPENSSL)
        if (sslContext) /* 启用TLS */
        {
            m_sslContext = sslContext;
            m_tcpSession = std::make_shared<TcpSession>(std::make_shared<SocketTls>(std::move(socket), *m_sslContext), false, m_bufferSize);
        }
        else /* 不启用TLS */
        {
#endif
            m_tcpSession = std::make_shared<TcpSession>(std::make_shared<SocketTcp>(std::move(socket)), false, m_bufferSize);
#if (1 == ENABLE_NSOCKET_OPENSSL)
        }
#endif
        const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
        m_tcpSession->setConnectCallback([wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->handleConnect(code);
            }
        });
        m_tcpSession->setDataCallback(m_onDataCallback);
        m_tcpSession->connect(m_endpointIter->endpoint());
        if (RunStatus::RUN_NONE == m_runStatus)
        {
            m_runStatus = RunStatus::RUN_START;
        }
        else if (RunStatus::RUN_STOP == m_runStatus)
        {
            stop();
            return;
        }
        m_ioContext.run();
    }
}

void TcpClient::send(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb)
{
    if (m_ioContext.stopped())
    {
        if (onSendCb)
        {
            onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
        }
    }
    else
    {
        const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
        boost::asio::post(m_ioContext, [wpSelf, data, onSendCb]() {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (self->m_tcpSession && self->isRunning())
                {
                    /**
                     *  1.根据boost中async_send说明, data最好保证生命周期存在到回调执行前, 这里将data绑定到回调中.
                     *  2.单元测纯TCP试验没绑到回调时确实会崩.
                     */
                    self->m_tcpSession->send(data, [onSendCb, data](const boost::system::error_code& code, std::size_t length) {
                        if (onSendCb)
                        {
                            onSendCb(code, length);
                        }
                    });
                }
            }
        });
    }
}

void TcpClient::stop()
{
    if (!m_ioContext.stopped())
    {
        const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
        boost::asio::post(m_ioContext, [wpSelf]() {
            const auto self = wpSelf.lock();
            if (self)
            {
                const auto lastStatus = self->m_runStatus;
                self->m_runStatus = RunStatus::RUN_STOP;
                if (RunStatus::RUN_START != lastStatus)
                {
                    return;
                }
                if (self->m_tcpSession)
                {
                    self->m_tcpSession->close();
                }
                self->m_ioContext.stop();
            }
        });
    }
}

bool TcpClient::isRunning() const
{
    return RunStatus::RUN_START == m_runStatus;
}

void TcpClient::handleConnect(const boost::system::error_code& code)
{
    if (m_tcpSession)
    {
        if (code) /* 连接失败 */
        {
            ++m_endpointIter;
            if (m_endpoints.end() == m_endpointIter) /* 没有下一个 */
            {
                stop();
                if (m_onConnectCallback)
                {
                    m_onConnectCallback(code);
                }
            }
            else /* 尝试下一个 */
            {
                m_tcpSession->connect(m_endpointIter->endpoint());
            }
        }
        else /* 连接成功 */
        {
            if (m_tcpSession->isEnableSSL()) /* 启用SSL */
            {
#if (1 == ENABLE_NSOCKET_OPENSSL)
                const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
                m_tcpSession->handshake(boost::asio::ssl::stream_base::handshake_type::client,
                                        [wpSelf](const boost::system::error_code& code) {
                                            const auto self = wpSelf.lock();
                                            if (self)
                                            {
                                                if (code) /* 握手失败 */
                                                {
                                                    self->handleConnect(code);
                                                }
                                                else /* 握手成功 */
                                                {
                                                    if (self->m_onConnectCallback)
                                                    {
                                                        self->m_onConnectCallback(code);
                                                    }
                                                    if (self->m_tcpSession)
                                                    {
                                                        self->m_tcpSession->recv(); /* 握手成功后开始接收数据 */
                                                    }
                                                }
                                            }
                                        }); /* 需要握手 */
#endif
            }
            else /* 没有启用SSL */
            {
                if (m_onConnectCallback)
                {
                    m_onConnectCallback(code);
                }
                m_tcpSession->recv(); /* 连接成功后开始接收数据 */
            }
        }
    }
    else /* 会话为空, 失败 */
    {
        stop();
        if (m_onConnectCallback)
        {
            m_onConnectCallback(code);
        }
    }
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
std::shared_ptr<boost::asio::ssl::context> TcpClient::getSslContext(const std::string& certFile, const std::string& privateKeyFile,
                                                                    const std::string& privateKeyFilePwd)
{
    return TcpSession::makeSslContext(boost::asio::ssl::context::sslv23_client, certFile, privateKeyFile, privateKeyFilePwd);
}
#endif
} // namespace nsocket