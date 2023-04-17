#include "tcp_client.h"

namespace nsocket
{
TcpClient::TcpClient(size_t bz) : m_bufferSize(bz) {}

TcpClient::~TcpClient()
{
    stop();
}

void TcpClient::setConnectCallback(const TCP_CONNECT_CALLBACK& onConnectCb)
{
    m_onConnectCallback = onConnectCb;
}

void TcpClient::setDataCallback(const TCP_DATA_CALLBACK& onDataCb)
{
    m_onDataCallback = onDataCb;
}

void TcpClient::setLocalPort(uint16_t port)
{
    m_localPort = port;
}

void TcpClient::run(const std::string& host, uint16_t port, bool sslOn, int sslWay, int certFmt, const std::string& certFile,
                    const std::string& pkFile, const std::string& pkPwd)
{
    sslWay = (1 == sslWay || 2 == sslWay) ? sslWay : 1;
    certFmt = (1 == certFmt || 2 == certFmt) ? certFmt : 2;
    if (RunStatus::running == m_runStatus)
    {
        return;
    }
    boost::system::error_code code;
    m_endpoints = boost::asio::ip::tcp::resolver(m_ioContext).resolve(host, std::to_string(port), code);
    if (code || m_endpoints.empty())
    {
        if (m_onConnectCallback)
        {
            m_onConnectCallback(code ? code : boost::system::errc::make_error_code(boost::system::errc::address_not_available));
        }
    }
    else
    {
        m_endpointIter = m_endpoints.begin();
        boost::asio::ip::tcp::socket socket(m_ioContext);
        std::shared_ptr<SocketTcpBase> socketPtr = nullptr;
#if (1 == ENABLE_NSOCKET_OPENSSL)
        if (sslOn)
        {
            if (1 == sslWay)
            {
                m_sslContext = TcpConnection::makeSsl1WayContextClient(boost::asio::ssl::context::sslv23_client, true);
            }
            else
            {
                m_sslContext = TcpConnection::makeSsl2WayContext(boost::asio::ssl::context::sslv23_client,
                                                                 1 == certFmt ? boost::asio::ssl::context::file_format::asn1
                                                                              : boost::asio::ssl::context::file_format::pem,
                                                                 certFile, pkFile, pkPwd, true);
            }
            if (m_sslContext)
            {
                socketPtr = std::make_shared<SocketTls>(std::move(socket), *m_sslContext);
            }
        }
#endif
        if (!socketPtr)
        {
            socketPtr = std::make_shared<SocketTcp>(std::move(socket));
        }
        const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
        m_tcpConn = std::make_shared<TcpConnection>(socketPtr, false, m_bufferSize);
        m_tcpConn->setConnectCallback([wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->handleConnect(code);
            }
        });
        m_tcpConn->setDataCallback(m_onDataCallback);
        m_tcpConn->setLocalPort(m_localPort);
        m_tcpConn->connect(m_endpointIter->endpoint(), true);
        if (RunStatus::idle == m_runStatus)
        {
            m_runStatus = RunStatus::running;
            m_ioContext.run();
        }
        stop();
    }
}

boost::system::error_code TcpClient::send(const std::vector<unsigned char>& data, size_t& sentLength)
{
    auto code = boost::system::errc::make_error_code(boost::system::errc::not_connected);
    sentLength = 0;
    if (RunStatus::running == m_runStatus && !m_ioContext.stopped() && m_tcpConn)
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
    if (RunStatus::running == m_runStatus && !m_ioContext.stopped() && m_tcpConn)
    {
        const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
        boost::asio::post(m_ioContext, [wpSelf, data, onSendCb]() {
            const auto self = wpSelf.lock();
            if (self && RunStatus::running == self->m_runStatus && !self->m_ioContext.stopped() && self->m_tcpConn)
            {
                self->m_tcpConn->send(data, onSendCb);
            }
            else if (onSendCb)
            {
                onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
            }
        });
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void TcpClient::stop()
{
    if (RunStatus::running == m_runStatus)
    {
        if (m_ioContext.stopped())
        {
            m_runStatus = RunStatus::stop;
            if (m_tcpConn)
            {
                m_tcpConn->close();
            }
        }
        else
        {
            const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
            boost::asio::post(m_ioContext, [wpSelf]() {
                const auto self = wpSelf.lock();
                if (self)
                {
                    const auto lastStatus = self->m_runStatus.load();
                    self->m_runStatus = RunStatus::stop;
                    if (RunStatus::running == lastStatus)
                    {
                        self->m_ioContext.stop();
                        if (self->m_tcpConn)
                        {
                            self->m_tcpConn->close();
                        }
                    }
                }
            });
        }
    }
}

bool TcpClient::isEnableSSL() const
{
    if (m_tcpConn)
    {
        return m_tcpConn->isEnableSSL();
    }
    return false;
}

bool TcpClient::isRunning() const
{
    return (RunStatus::running == m_runStatus);
}

boost::asio::ip::tcp::endpoint TcpClient::getLocalEndpoint() const
{
    if (m_tcpConn)
    {
        return m_tcpConn->getLocalEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}

boost::asio::ip::tcp::endpoint TcpClient::getRemoteEndpoint() const
{
    if (m_tcpConn)
    {
        return m_tcpConn->getRemoteEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}

void TcpClient::handleConnect(const boost::system::error_code& code)
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
                m_tcpConn->setLocalPort(m_localPort);
                m_tcpConn->connect(m_endpointIter->endpoint(), true);
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
                            }
                        }
                    },
                    true); /* 需要握手 */
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
} // namespace nsocket
