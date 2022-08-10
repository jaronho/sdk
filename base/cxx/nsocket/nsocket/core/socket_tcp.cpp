#include "socket_tcp.h"

namespace nsocket
{
SocketTcp::SocketTcp(boost::asio::ip::tcp::socket socket) : m_socket(std::move(socket)) {}

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
        if (async)
        {
            m_socket.async_connect(point, onConnectCb);
        }
        else
        {
            boost::system::error_code code;
            m_socket.connect(point, code);
            if (onConnectCb)
            {
                onConnectCb(code);
            }
        }
    }
}

void SocketTcp::send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb, bool async)
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
            if (async)
            {
                m_socket.async_send(data, onSendCb);
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

void SocketTcp::bind(const boost::asio::ip::tcp::endpoint& point, boost::system::error_code& code)
{
    m_socket.bind(point, code);
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

boost::asio::ip::tcp::endpoint SocketTcp::getRemoteEndpoint() const
{
    boost::system::error_code code;
    return m_socket.remote_endpoint(code);
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
SocketTls::SocketTls(boost::asio::ip::tcp::socket socket, boost::asio::ssl::context& sslContext) : sslStream(std::move(socket), sslContext)
{
}

void SocketTls::connect(const boost::asio::ip::tcp::endpoint& point, const TCP_CONNECT_CALLBACK& onConnectCb, bool async)
{
    if (sslStream.lowest_layer().is_open())
    {
        if (onConnectCb)
        {
            onConnectCb(boost::system::errc::make_error_code(boost::system::errc::success));
        }
    }
    else
    {
        if (async)
        {
            sslStream.lowest_layer().async_connect(point, onConnectCb);
        }
        else
        {
            boost::system::error_code code;
            sslStream.lowest_layer().connect(point, code);
            if (onConnectCb)
            {
                onConnectCb(code);
            }
        }
    }
}

void SocketTls::send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb, bool async)
{
    if (sslStream.lowest_layer().is_open())
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
            if (async)
            {
                sslStream.async_write_some(boost::asio::buffer(data), onSendCb);
            }
            else
            {
                boost::system::error_code code;
                auto length = sslStream.write_some(data, code);
                if (onSendCb)
                {
                    onSendCb(code, length);
                }
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
    if (sslStream.lowest_layer().is_open())
    {
        sslStream.async_read_some(boost::asio::buffer(data), onRecvCb);
    }
    else if (onRecvCb)
    {
        onRecvCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void SocketTls::bind(const boost::asio::ip::tcp::endpoint& host, boost::system::error_code& code)
{
    sslStream.lowest_layer().bind(host, code);
}

void SocketTls::close()
{
    if (sslStream.lowest_layer().is_open())
    {
        boost::system::error_code code;
        sslStream.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, code);
        sslStream.lowest_layer().close(code);
    }
}

bool SocketTls::isOpened() const
{
    return sslStream.lowest_layer().is_open();
}

boost::asio::ip::tcp::endpoint SocketTls::getRemoteEndpoint() const
{
    boost::system::error_code code;
    return sslStream.lowest_layer().remote_endpoint(code);
}

void SocketTls::handshake(boost::asio::ssl::stream_base::handshake_type type, const TLS_HANDSHAKE_CALLBACK& onHandshakeCb, bool async)
{
    if (sslStream.lowest_layer().is_open())
    {
        if (async)
        {
            sslStream.async_handshake(type, onHandshakeCb);
        }
        else
        {
            boost::system::error_code code;
            sslStream.handshake(type, code);
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
