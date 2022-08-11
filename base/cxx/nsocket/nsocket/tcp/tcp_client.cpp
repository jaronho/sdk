#include "tcp_client.h"

#include <functional>

namespace nsocket
{
TcpClient::TcpClient(size_t bz)
    : m_tcpConn(nullptr)
    , m_bufferSize(bz)
#if (1 == ENABLE_NSOCKET_OPENSSL)
    , m_sslContext(nullptr)
#endif
    , m_onConnectCallback(nullptr)
    , m_onDataCallback(nullptr)
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
void TcpClient::run(const std::string& host, unsigned int port, const std::shared_ptr<boost::asio::ssl::context>& sslContext, bool async)
#else
void TcpClient::run(const std::string& host, unsigned int port, bool async)
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
            m_tcpConn = std::make_shared<TcpConnection>(std::make_shared<SocketTls>(std::move(socket), *m_sslContext), false, m_bufferSize);
        }
        else /* 不启用TLS */
        {
#endif
            m_tcpConn = std::make_shared<TcpConnection>(std::make_shared<SocketTcp>(std::move(socket)), false, m_bufferSize);
#if (1 == ENABLE_NSOCKET_OPENSSL)
        }
#endif
        const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
        m_tcpConn->setConnectCallback([wpSelf, async](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->handleConnect(code, async);
            }
        });
        m_tcpConn->setDataCallback(m_onDataCallback);
        m_tcpConn->connect(m_endpointIter->endpoint(), async);
        if (RunStatus::none == m_runStatus)
        {
            m_runStatus = RunStatus::start;
        }
        else if (RunStatus::stop == m_runStatus)
        {
            stop();
            return;
        }
        m_ioContext.run();
    }
}

boost::system::error_code TcpClient::send(const std::vector<unsigned char>& data, size_t& sentLength)
{
    auto code = boost::system::errc::make_error_code(boost::system::errc::not_connected);
    sentLength = 0;
    if (isRunning() && !m_ioContext.stopped() && m_tcpConn)
    {
        m_tcpConn->send(data, [&code, &sentLength](const boost::system::error_code& ec, size_t length) {
            code = ec;
            sentLength += length;
        });
    }
    return code;
}

void TcpClient::sendAsync(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb)
{
    if (!isRunning() || m_ioContext.stopped())
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
                if (self->m_tcpConn && self->isRunning())
                {
                    self->m_tcpConn->send(data, onSendCb);
                }
            }
        });
    }
}

void TcpClient::stop()
{
    if (isRunning() && !m_ioContext.stopped())
    {
        const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
        boost::asio::post(m_ioContext, [wpSelf]() {
            const auto self = wpSelf.lock();
            if (self)
            {
                const auto lastStatus = self->m_runStatus.load();
                self->m_runStatus = RunStatus::stop;
                if (RunStatus::start != lastStatus)
                {
                    return;
                }
                if (self->m_tcpConn)
                {
                    self->m_tcpConn->close();
                }
                self->m_ioContext.stop();
            }
        });
    }
    else
    {
        m_runStatus = RunStatus::stop;
    }
}

bool TcpClient::isRunning() const
{
    return RunStatus::start == m_runStatus;
}

void TcpClient::handleConnect(const boost::system::error_code& code, bool async)
{
    if (m_tcpConn)
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
                m_tcpConn->connect(m_endpointIter->endpoint(), async);
            }
        }
        else /* 连接成功 */
        {
            if (m_tcpConn->isEnableSSL()) /* 启用SSL */
            {
#if (1 == ENABLE_NSOCKET_OPENSSL)
                const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
                m_tcpConn->handshake(
                    boost::asio::ssl::stream_base::handshake_type::client,
                    [wpSelf, async](const boost::system::error_code& code) {
                        const auto self = wpSelf.lock();
                        if (self)
                        {
                            if (code) /* 握手失败 */
                            {
                                self->handleConnect(code, async);
                            }
                            else /* 握手成功 */
                            {
                                if (self->m_onConnectCallback)
                                {
                                    self->m_onConnectCallback(code);
                                }
                            }
                        }
                    },
                    async); /* 需要握手 */
#endif
            }
            else /* 没有启用SSL */
            {
                if (m_onConnectCallback)
                {
                    m_onConnectCallback(code);
                }
            }
        }
    }
    else /* 连接为空, 失败 */
    {
        stop();
        if (m_onConnectCallback)
        {
            m_onConnectCallback(code);
        }
    }
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
std::shared_ptr<boost::asio::ssl::context> TcpClient::getSsl1WayContext(const std::string& caFile)
{
    return TcpConnection::makeSsl1WayContextClient(boost::asio::ssl::context::sslv23_client, caFile, true);
}

std::shared_ptr<boost::asio::ssl::context> TcpClient::getSsl2WayContext(const std::string& certFile, const std::string& privateKeyFile,
                                                                        const std::string& privateKeyFilePwd)
{
    return TcpConnection::makeSsl2WayContext(boost::asio::ssl::context::sslv23_client, certFile, privateKeyFile, privateKeyFilePwd, true);
}
#endif
} // namespace nsocket
