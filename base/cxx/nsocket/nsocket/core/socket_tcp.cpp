#include "socket_tcp.h"

namespace nsocket
{
SocketTcp::SocketTcp(boost::asio::ip::tcp::socket socket) : m_socket(std::move(socket)) {}

SocketTcp::~SocketTcp()
{
    close();
}

void SocketTcpBase::setNonBlock(bool nonBlock)
{
    m_nonBlock = (nonBlock ? 1 : 0);
}

void SocketTcpBase::setSendBufferSize(int bufferSize)
{
    m_sendBufferSize = bufferSize;
}

void SocketTcpBase::setRecvBufferSize(int bufferSize)
{
    m_recvBufferSize = bufferSize;
}

void SocketTcpBase::setNagleEnable(bool enable)
{
    m_enableNagle = (enable ? 1 : 0);
}

void SocketTcpBase::setLocalPort(uint16_t port)
{
    m_localPort = port;
}

void SocketTcp::connect(const boost::asio::ip::tcp::endpoint& point, const TCP_CONNECT_CALLBACK& onConnectCb, bool async)
{
    if (m_socket.is_open())
    {
        if (onConnectCb)
        {
            onConnectCb(boost::system::errc::make_error_code(boost::system::errc::success));
        }
    }
    else
    {
        m_remotePoint = point;
        boost::system::error_code code;
        m_socket.open(boost::asio::ip::tcp::v4(), code);
        if (code)
        {
            if (onConnectCb)
            {
                onConnectCb(code);
            }
        }
        else
        {
            do
            {
                if (m_nonBlock >= 0)
                {
                    m_socket.non_blocking(m_nonBlock > 0 ? true : false, code);
                    if (code)
                    {
                        break;
                    }
                }
                if (m_sendBufferSize > 0)
                {
                    m_socket.set_option(boost::asio::socket_base::send_buffer_size(m_sendBufferSize), code);
                    if (code)
                    {
                        break;
                    }
                }
                if (m_recvBufferSize > 0)
                {
                    m_socket.set_option(boost::asio::socket_base::receive_buffer_size(m_recvBufferSize), code);
                    if (code)
                    {
                        break;
                    }
                }
                if (m_enableNagle >= 0)
                {
                    m_socket.set_option(boost::asio::ip::tcp::no_delay(m_enableNagle > 0 ? false : true), code);
                    if (code)
                    {
                        break;
                    }
                }
                if (m_localPort > 0)
                {
                    m_socket.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_localPort), code);
                    if (code)
                    {
                        break;
                    }
                }
            } while (0);
            if (code)
            {
                m_socket.close();
                if (onConnectCb)
                {
                    onConnectCb(code);
                }
            }
            else
            {
                if (async)
                {
                    m_socket.async_connect(point, onConnectCb);
                }
                else
                {
                    m_socket.connect(point, code);
                    if (onConnectCb)
                    {
                        onConnectCb(code);
                    }
                }
            }
        }
    }
}

boost::asio::io_context& SocketTcp::getIoContext()
{
    return static_cast<boost::asio::io_context&>(m_socket.get_executor().context());
}

void SocketTcp::send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb)
{
    if (m_socket.is_open())
    {
        if (0 == data.size())
        {
            if (onSendCb)
            {
                onSendCb(boost::system::errc::make_error_code(boost::system::errc::no_message_available), 0);
            }
        }
        else
        {
            boost::system::error_code code;
            auto length = m_socket.send(data, boost::asio::socket_base::message_flags(0), code);
            if (onSendCb)
            {
                onSendCb(code, length);
            }
        }
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void SocketTcp::recv(const boost::asio::mutable_buffer& data, const TCP_RECV_CALLBACK& onRecvCb)
{
    if (m_socket.is_open())
    {
        m_socket.async_receive(data, onRecvCb);
    }
    else if (onRecvCb)
    {
        onRecvCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void SocketTcp::close()
{
    if (m_socket.is_open())
    {
        boost::system::error_code code;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, code);
        m_socket.close(code);
    }
}

bool SocketTcp::isOpened() const
{
    return m_socket.is_open();
}

bool SocketTcp::isNonBlock() const
{
    return m_socket.non_blocking();
}

int SocketTcp::getSendBufferSize() const
{
    boost::asio::socket_base::send_buffer_size opt;
    m_socket.get_option(opt);
    return opt.value();
}

int SocketTcp::getRecvBufferSize() const
{
    boost::asio::socket_base::receive_buffer_size opt;
    m_socket.get_option(opt);
    return opt.value();
}

bool SocketTcp::isNagleEnable() const
{
    boost::asio::ip::tcp::no_delay opt;
    m_socket.get_option(opt);
    return (opt.value() ? false : true);
}

boost::asio::ip::tcp::endpoint SocketTcp::getLocalEndpoint() const
{
    boost::system::error_code code;
    auto point = m_socket.local_endpoint(code);
    if (code && m_localPort > 0)
    {
        point.port(m_localPort);
    }
    return point;
}

boost::asio::ip::tcp::endpoint SocketTcp::getRemoteEndpoint() const
{
    boost::system::error_code code;
    auto point = m_socket.remote_endpoint(code);
    if (code)
    {
        return m_remotePoint;
    }
    return point;
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
SocketTls::SocketTls(boost::asio::ip::tcp::socket socket, boost::asio::ssl::context& sslContext)
    : m_sslStream(std::move(socket), sslContext)
{
}

SocketTls::~SocketTls()
{
    close();
}

void SocketTls::connect(const boost::asio::ip::tcp::endpoint& point, const TCP_CONNECT_CALLBACK& onConnectCb, bool async)
{
    m_remotePoint = point;
    if (m_sslStream.lowest_layer().is_open())
    {
        if (onConnectCb)
        {
            onConnectCb(boost::system::errc::make_error_code(boost::system::errc::success));
        }
    }
    else
    {
        boost::system::error_code code;
        m_sslStream.lowest_layer().open(boost::asio::ip::tcp::v4(), code);
        if (code)
        {
            if (onConnectCb)
            {
                onConnectCb(code);
            }
        }
        else
        {
            do
            {
                if (m_nonBlock >= 0)
                {
                    m_sslStream.lowest_layer().non_blocking(m_nonBlock > 0 ? true : false, code);
                    if (code)
                    {
                        break;
                    }
                }
                if (m_sendBufferSize > 0)
                {
                    m_sslStream.lowest_layer().set_option(boost::asio::socket_base::send_buffer_size(m_sendBufferSize), code);
                    if (code)
                    {
                        break;
                    }
                }
                if (m_recvBufferSize > 0)
                {
                    m_sslStream.lowest_layer().set_option(boost::asio::socket_base::receive_buffer_size(m_recvBufferSize), code);
                    if (code)
                    {
                        break;
                    }
                }
                if (m_enableNagle >= 0)
                {
                    m_sslStream.lowest_layer().set_option(boost::asio::ip::tcp::no_delay(m_enableNagle > 0 ? false : true), code);
                    if (code)
                    {
                        break;
                    }
                }
                if (m_localPort > 0)
                {
                    m_sslStream.lowest_layer().bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_localPort), code);
                }
                if (code)
                {
                    break;
                }
            } while (0);
            if (code)
            {
                if (onConnectCb)
                {
                    onConnectCb(code);
                }
            }
            else
            {
                if (async)
                {
                    m_sslStream.lowest_layer().async_connect(point, onConnectCb);
                }
                else
                {
                    m_sslStream.lowest_layer().connect(point, code);
                    if (onConnectCb)
                    {
                        onConnectCb(code);
                    }
                }
            }
        }
    }
}

boost::asio::io_context& SocketTls::getIoContext()
{
    return static_cast<boost::asio::io_context&>(m_sslStream.lowest_layer().get_executor().context());
}

void SocketTls::send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb)
{
    if (m_sslStream.lowest_layer().is_open())
    {
        if (0 == data.size())
        {
            if (onSendCb)
            {
                onSendCb(boost::system::errc::make_error_code(boost::system::errc::no_message_available), 0);
            }
        }
        else
        {
            boost::system::error_code code;
            auto length = m_sslStream.write_some(data, code);
            if (onSendCb)
            {
                onSendCb(code, length);
            }
        }
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void SocketTls::recv(const boost::asio::mutable_buffer& data, const TCP_RECV_CALLBACK& onRecvCb)
{
    if (m_sslStream.lowest_layer().is_open())
    {
        m_sslStream.async_read_some(data, onRecvCb);
    }
    else if (onRecvCb)
    {
        onRecvCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void SocketTls::close()
{
    if (m_sslStream.lowest_layer().is_open())
    {
        boost::system::error_code code;
        m_sslStream.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, code);
        m_sslStream.lowest_layer().close(code);
    }
}

bool SocketTls::isOpened() const
{
    return m_sslStream.lowest_layer().is_open();
}

bool SocketTls::isNonBlock() const
{
    return m_sslStream.lowest_layer().non_blocking();
}

int SocketTls::getSendBufferSize() const
{
    boost::asio::socket_base::send_buffer_size opt;
    m_sslStream.lowest_layer().get_option(opt);
    return opt.value();
}

int SocketTls::getRecvBufferSize() const
{
    boost::asio::socket_base::receive_buffer_size opt;
    m_sslStream.lowest_layer().get_option(opt);
    return opt.value();
}

bool SocketTls::isNagleEnable() const
{
    boost::asio::ip::tcp::no_delay opt;
    m_sslStream.lowest_layer().get_option(opt);
    return (opt.value() ? false : true);
}

boost::asio::ip::tcp::endpoint SocketTls::getLocalEndpoint() const
{
    boost::system::error_code code;
    auto point = m_sslStream.lowest_layer().local_endpoint(code);
    if (code && m_localPort > 0)
    {
        point.port(m_localPort);
    }
    return point;
}

boost::asio::ip::tcp::endpoint SocketTls::getRemoteEndpoint() const
{
    boost::system::error_code code;
    auto point = m_sslStream.lowest_layer().remote_endpoint(code);
    if (code)
    {
        return m_remotePoint;
    }
    return point;
}

void SocketTls::handshake(boost::asio::ssl::stream_base::handshake_type type, const TLS_HANDSHAKE_CALLBACK& onHandshakeCb, bool async)
{
    if (m_sslStream.lowest_layer().is_open())
    {
        if (async)
        {
            m_sslStream.async_handshake(type, onHandshakeCb);
        }
        else
        {
            boost::system::error_code code;
            m_sslStream.handshake(type, code);
            if (onHandshakeCb)
            {
                onHandshakeCb(code);
            }
        }
    }
    else if (onHandshakeCb)
    {
        onHandshakeCb(boost::system::errc::make_error_code(boost::system::errc::not_connected));
    }
}
#endif
} // namespace nsocket
