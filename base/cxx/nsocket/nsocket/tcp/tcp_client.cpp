#include "tcp_client.h"

namespace nsocket
{
TcpClient::TcpClient(uint16_t localPort, size_t bz) : m_localPort(localPort), m_bufferSize(bz) {}

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

void TcpClient::run(const std::string& host, uint16_t port, bool sslOn, int sslWay, int certFmt, const std::string& certFile,
                    const std::string& pkFile, const std::string& pkPwd)
{
    sslWay = (1 == sslWay || 2 == sslWay) ? sslWay : 1;
    certFmt = (1 == certFmt || 2 == certFmt) ? certFmt : 2;
    if (RunStatus::idle == m_runStatus)
    {
        m_runStatus = RunStatus::running;
        boost::system::error_code code;
        auto endpoints = boost::asio::ip::tcp::resolver(m_ioContext).resolve(host, std::to_string(port), code);
        if (code || endpoints.empty())
        {
            if (m_onConnectCallback)
            {
                m_onConnectCallback(code ? code : boost::system::errc::make_error_code(boost::system::errc::address_not_available));
            }
        }
        else
        {
            boost::asio::ip::tcp::socket socket(m_ioContext);
            std::shared_ptr<SocketTcpBase> socketPtr = nullptr;
#if (1 == ENABLE_NSOCKET_OPENSSL)
            if (sslOn)
            {
                std::shared_ptr<boost::asio::ssl::context> sslContext = nullptr;
                if (1 == sslWay)
                {
                    sslContext = TcpConnection::makeSsl1WayContextClient(boost::asio::ssl::context::sslv23_client, true);
                }
                else
                {
                    sslContext = TcpConnection::makeSsl2WayContext(boost::asio::ssl::context::sslv23_client,
                                                                   1 == certFmt ? boost::asio::ssl::context::file_format::asn1
                                                                                : boost::asio::ssl::context::file_format::pem,
                                                                   certFile, pkFile, pkPwd, true);
                }
                if (sslContext)
                {
                    socketPtr = std::make_shared<SocketTls>(std::move(socket), *sslContext);
                }
                {
                    std::lock_guard<std::mutex> locker(m_mutex);
                    m_sslContext = sslContext;
                }
            }
#endif
            if (!socketPtr)
            {
                socketPtr = std::make_shared<SocketTcp>(std::move(socket));
            }
            const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
            auto tcpConn = std::make_shared<TcpConnection>(socketPtr, false, m_bufferSize);
            tcpConn->setConnectCallback([wpSelf](const boost::system::error_code& code) {
                const auto self = wpSelf.lock();
                if (self)
                {
                    self->handleConnect(code);
                }
            });
            tcpConn->setDataCallback(m_onDataCallback);
            tcpConn->setLocalPort(m_localPort);
            {
                std::lock_guard<std::mutex> locker(m_mutex);
                m_tcpConn = tcpConn;
            }
            m_ioContext.stop();
            if (RunStatus::running == m_runStatus)
            {
                tcpConn->connect(endpoints.begin()->endpoint(), true);
                m_ioContext.restart();
                m_ioContext.run();
            }
        }
    }
}

boost::system::error_code TcpClient::send(const std::vector<unsigned char>& data, size_t& sentLength)
{
    auto code = boost::system::errc::make_error_code(boost::system::errc::not_connected);
    sentLength = 0;
    if (RunStatus::running == m_runStatus && !m_ioContext.stopped())
    {
        std::shared_ptr<TcpConnection> tcpConn = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tcpConn = m_tcpConn;
        }
        if (tcpConn)
        {
            tcpConn->send(data, [&code, &sentLength](const boost::system::error_code& ec, size_t length) {
                code = ec;
                sentLength += length;
            });
        }
    }
    return code;
}

void TcpClient::sendAsync(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb)
{
    if (RunStatus::running == m_runStatus && !m_ioContext.stopped())
    {
        std::shared_ptr<TcpConnection> tcpConn = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tcpConn = m_tcpConn;
        }
        const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
        boost::asio::post(m_ioContext, [wpSelf, tcpConn, data, onSendCb]() {
            const auto self = wpSelf.lock();
            if (self && RunStatus::running == self->m_runStatus && !self->m_ioContext.stopped() && tcpConn)
            {
                tcpConn->send(data, onSendCb);
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
        m_runStatus = RunStatus::idle;
        std::shared_ptr<TcpConnection> tcpConn = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tcpConn = m_tcpConn;
        }
        if (m_ioContext.stopped())
        {
            if (tcpConn)
            {
                tcpConn->close();
            }
        }
        else
        {
            const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
            boost::asio::post(m_ioContext, [wpSelf, tcpConn]() {
                if (tcpConn)
                {
                    tcpConn->close();
                }
                const auto self = wpSelf.lock();
                if (self && RunStatus::idle == self->m_runStatus)
                {
                    self->m_ioContext.stop();
                }
            });
        }
    }
}

bool TcpClient::isEnableSSL()
{
    std::shared_ptr<TcpConnection> tcpConn = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        tcpConn = m_tcpConn;
    }
    if (tcpConn)
    {
        return tcpConn->isEnableSSL();
    }
    return false;
}

bool TcpClient::isRunning()
{
    return (RunStatus::running == m_runStatus);
}

boost::asio::ip::tcp::endpoint TcpClient::getLocalEndpoint()
{
    std::shared_ptr<TcpConnection> tcpConn = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        tcpConn = m_tcpConn;
    }
    if (tcpConn)
    {
        return tcpConn->getLocalEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}

boost::asio::ip::tcp::endpoint TcpClient::getRemoteEndpoint()
{
    std::shared_ptr<TcpConnection> tcpConn = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        tcpConn = m_tcpConn;
    }
    if (tcpConn)
    {
        return tcpConn->getRemoteEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}

void TcpClient::handleConnect(const boost::system::error_code& code)
{
    std::shared_ptr<TcpConnection> tcpConn = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        tcpConn = m_tcpConn;
    }
    if (tcpConn)
    {
        if (code) /* 连接失败 */
        {
            stop();
            if (m_onConnectCallback)
            {
                m_onConnectCallback(code);
            }
        }
        else /* 连接成功 */
        {
            if (tcpConn->isEnableSSL()) /* 启用SSL */
            {
#if (1 == ENABLE_NSOCKET_OPENSSL)
                const std::weak_ptr<TcpClient> wpSelf = shared_from_this();
                tcpConn->handshake(
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
