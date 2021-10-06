#include "tcp_session.h"

namespace nsocket
{
TcpSession::TcpSession(const std::shared_ptr<SocketTcpBase>& socket, bool alreadyConnected) : m_socketTcpBase(socket)
{
#if (1 == ENABLE_SOCKET_OPENSSL)
    m_isEnableTLS = (std::dynamic_pointer_cast<SocketTls>(m_socketTcpBase) ? true : false);
#else
    m_isEnableTLS = false;
#endif
    m_isConnected = alreadyConnected;
    m_recvBuf.resize(1024);
}

TcpSession::~TcpSession()
{
    close();
}

void TcpSession::setConnectCallback(const TCP_CONNECT_CALLBACK& onConnectCb)
{
    m_onConnectCallback = onConnectCb;
}

void TcpSession::setRecvDataCallback(const TCP_RECV_DATA_CALLBACK& onRecvDataCb)
{
    m_onRecvDataCallback = onRecvDataCb;
}

void TcpSession::connect(const boost::asio::ip::tcp::endpoint& point)
{
    if (m_socketTcpBase)
    {
        const std::weak_ptr<TcpSession> wpSelf = shared_from_this();
        m_socketTcpBase->connect(point, [wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (code) /* 连接失败 */
                {
                    self->close();
                    if (self->m_onConnectCallback)
                    {
                        self->m_onConnectCallback(code);
                    }
                }
                else /* 连接成功 */
                {
                    if (self->m_isEnableTLS) /* TLS, 需要握手 */
                    {
                        if (self->m_onConnectCallback)
                        {
                            self->m_onConnectCallback(code);
                        }
                    }
                    else /* TCP, 成功后开始接收数据 */
                    {
                        self->m_isConnected = true;
                        if (self->m_onConnectCallback)
                        {
                            self->m_onConnectCallback(code);
                        }
                    }
                }
            }
        });
    }
    else if (m_onConnectCallback)
    {
        m_onConnectCallback(boost::system::errc::make_error_code(boost::system::errc::not_a_socket));
    }
}

#if (1 == ENABLE_SOCKET_OPENSSL)
void TcpSession::handshake(boost::asio::ssl::stream_base::handshake_type type, const TLS_HANDSHAKE_CALLBACK& onHandshakeCb)
{
    std::shared_ptr<SocketTls> tlsPtr = std::dynamic_pointer_cast<SocketTls>(m_socketTcpBase);
    if (tlsPtr)
    {
        const std::weak_ptr<TcpSession> wpSelf = shared_from_this();
        tlsPtr->handshake(type, [wpSelf, onHandshakeCb](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (code) /* 握手失败 */
                {
                    self->close();
                    if (onHandshakeCb)
                    {
                        onHandshakeCb(code);
                    }
                }
                else /* 握手成功 */
                {
                    self->m_isConnected = true;
                    if (onHandshakeCb)
                    {
                        onHandshakeCb(code);
                    }
                }
            }
        });
    }
    else
    {
        close();
        if (onHandshakeCb)
        {
            onHandshakeCb(boost::system::errc::make_error_code(boost::system::errc::not_a_socket));
        }
    }
}
#endif

void TcpSession::send(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb)
{
    if (m_socketTcpBase && m_isConnected)
    {
        m_socketTcpBase->send(boost::asio::buffer(data.data(), data.size()), onSendCb);
    }
    else
    {
        if (onSendCb)
        {
            onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
        }
    }
}

void TcpSession::TcpSession::recv()
{
    if (m_socketTcpBase)
    {
        const std::weak_ptr<TcpSession> wpSelf = shared_from_this();
        m_socketTcpBase->recv(boost::asio::buffer(m_recvBuf), [wpSelf](const boost::system::error_code& code, std::size_t length) {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (code) /* 接收失败 */
                {
                    self->close();
                    if (self->m_onConnectCallback)
                    {
                        self->m_onConnectCallback(code);
                    }
                }
                else /* 接收成功 */
                {
                    if (self->m_onRecvDataCallback)
                    {
                        std::vector<unsigned char> data;
                        const unsigned char* rawData = (const unsigned char*)self->m_recvBuf.data();
                        if (rawData && length > 0)
                        {
                            data.insert(data.end(), rawData, rawData + length);
                        }
                        self->m_onRecvDataCallback(data);
                    }
                    self->recv(); /* 继续接收 */
                }
            }
        });
    }
    else
    {
        close();
        if (m_onConnectCallback)
        {
            m_onConnectCallback(boost::system::errc::make_error_code(boost::system::errc::not_a_socket));
        }
    }
}

void TcpSession::close()
{
    m_isConnected = false;
    if (m_socketTcpBase)
    {
        m_socketTcpBase->close();
    }
}

bool TcpSession::isEnableTLS() const
{
    return m_isEnableTLS;
}

bool TcpSession::isConnected() const
{
    return m_isConnected;
}

boost::asio::ip::tcp::endpoint TcpSession::getRemoteEndpoint() const
{
    if (m_socketTcpBase)
    {
        return m_socketTcpBase->getRemoteEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}
} // namespace nsocket
