#include "socket_udp.h"

namespace nsocket
{
SocketUdp::SocketUdp(boost::asio::ip::udp::socket socket) : m_socket(std::move(socket)) {}

SocketUdp::~SocketUdp()
{
    close();
}

void SocketUdp::open(const boost::asio::ip::udp::endpoint& point, const UDP_OPEN_CALLBACK& onOpenCb)
{
    if (m_socket.is_open())
    {
        if (onOpenCb)
        {
            onOpenCb(boost::system::errc::make_error_code(boost::system::errc::success));
        }
    }
    else
    {
        m_localPoint = point;
        boost::system::error_code code;
        m_socket.open(boost::asio::ip::udp::v4(), code);
        if (code)
        {
            if (onOpenCb)
            {
                onOpenCb(code);
            }
        }
        else
        {
            m_socket.bind(point, code);
            if (code)
            {
                m_socket.close();
            }
            if (onOpenCb)
            {
                onOpenCb(code);
            }
        }
    }
}

void SocketUdp::send(const boost::asio::ip::udp::endpoint& point, const boost::asio::const_buffer& data, const UDP_SEND_CALLBACK& onSendCb)
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
            auto length = m_socket.send_to(data, point, boost::asio::socket_base::message_flags(0), code);
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

void SocketUdp::recv(const boost::asio::mutable_buffer& data, const UDP_RECV_CALLBACK& onRecvCb)
{
    if (m_socket.is_open())
    {
        m_socket.async_receive_from(data, m_remotePoint, [&, onRecvCb](const boost::system::error_code& code, size_t length) {
            if (onRecvCb)
            {
                onRecvCb(m_remotePoint, code, length);
            }
        });
    }
    else if (onRecvCb)
    {
        onRecvCb(boost::asio::ip::udp::endpoint(), boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void SocketUdp::close()
{
    if (m_socket.is_open())
    {
        boost::system::error_code code;
        m_socket.shutdown(boost::asio::ip::udp::socket::shutdown_both, code);
        m_socket.close(code);
    }
}

bool SocketUdp::isOpened() const
{
    return m_socket.is_open();
}

boost::asio::ip::udp::endpoint SocketUdp::getLocalEndpoint() const
{
    boost::system::error_code code;
    auto point = m_socket.local_endpoint(code);
    if (code)
    {
        return m_localPoint;
    }
    return point;
}

bool SocketUdp::isNonBlock() const
{
    return m_socket.non_blocking();
}

bool SocketUdp::setNonBlock(bool nonBlock)
{
    if (m_socket.is_open())
    {
        boost::system::error_code code;
        m_socket.non_blocking(nonBlock, code);
        if (!code)
        {
            return true;
        }
    }
    return false;
}

size_t SocketUdp::getSendBufferSize() const
{
    if (m_socket.is_open())
    {
        boost::asio::socket_base::send_buffer_size opt;
        m_socket.get_option(opt);
        return opt.value();
    }
    return 0;
}

bool SocketUdp::setSendBufferSize(size_t bufferSize)
{
    if (m_socket.is_open() && bufferSize > 0)
    {
        boost::system::error_code code;
        m_socket.set_option(boost::asio::socket_base::send_buffer_size(bufferSize), code);
        if (!code)
        {
            return true;
        }
    }
    return false;
}

size_t SocketUdp::getRecvBufferSize() const
{
    if (m_socket.is_open())
    {
        boost::asio::socket_base::receive_buffer_size opt;
        m_socket.get_option(opt);
        return opt.value();
    }
    return 0;
}

bool SocketUdp::setRecvBufferSize(size_t bufferSize)
{
    if (m_socket.is_open() && bufferSize > 0)
    {
        boost::system::error_code code;
        m_socket.set_option(boost::asio::socket_base::receive_buffer_size(bufferSize), code);
        if (!code)
        {
            return true;
        }
    }
    return false;
}
} // namespace nsocket
