#include "sync_tcp_client.h"

namespace nsocket
{
SyncTcpClient::SyncTcpClient(size_t bz) : m_socket(m_ioContext)
{
    m_recvBuf.resize(bz > 256 ? bz : 256);
}

SyncTcpClient::~SyncTcpClient()
{
    stop();
}

void SyncTcpClient::setNonBlock(bool nonBlock)
{
    m_nonBlock = (nonBlock ? 1 : 0);
}

void SyncTcpClient::setSendBufferSize(int bufferSize)
{
    m_sendBufferSize = bufferSize;
}

void SyncTcpClient::setRecvBufferSize(int bufferSize)
{
    m_recvBufferSize = bufferSize;
}

void SyncTcpClient::setNagleEnable(bool enable)
{
    m_enableNagle = (enable ? 1 : 0);
}

void SyncTcpClient::setLocalPort(uint16_t port)
{
    m_localPort = port;
}

boost::system::error_code SyncTcpClient::connect(const std::string& host, uint16_t port, const std::chrono::steady_clock::duration& timeout)
{
    auto ec = std::make_shared<boost::system::error_code>();
    auto endpoints = boost::asio::ip::tcp::resolver(m_ioContext).resolve(host, std::to_string(port), *ec);
    if (*ec || endpoints.empty())
    {
        return *ec ? *ec : boost::system::errc::make_error_code(boost::system::errc::address_not_available);
    }
    m_socket.open(boost::asio::ip::tcp::v4(), *ec);
    if (!(*ec))
    {
        do
        {
            if (m_nonBlock >= 0)
            {
                m_socket.non_blocking(m_nonBlock > 0 ? true : false, *ec);
                if (*ec)
                {
                    break;
                }
            }
            if (m_sendBufferSize > 0)
            {
                m_socket.set_option(boost::asio::socket_base::send_buffer_size(m_sendBufferSize), *ec);
                if (*ec)
                {
                    break;
                }
            }
            if (m_recvBufferSize > 0)
            {
                m_socket.set_option(boost::asio::socket_base::receive_buffer_size(m_recvBufferSize), *ec);
                if (*ec)
                {
                    break;
                }
            }
            if (m_enableNagle >= 0)
            {
                m_socket.set_option(boost::asio::ip::tcp::no_delay(m_enableNagle > 0 ? false : true), *ec);
                if (*ec)
                {
                    break;
                }
            }
            if (m_localPort > 0)
            {
                m_socket.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_localPort), *ec);
            }
            if (*ec)
            {
                break;
            }
        } while (0);
        if (*ec)
        {
            m_socket.close();
        }
        else
        {
            m_socket.async_connect(endpoints.begin()->endpoint(), [ec](const boost::system::error_code& code) { *ec = code; });
            if (!runImpl(timeout))
            {
                *ec = boost::system::errc::make_error_code(boost::system::errc::timed_out);
            }
        }
    }
    return *ec;
}

boost::system::error_code SyncTcpClient::send(const std::vector<unsigned char>& data, size_t* sentLength)
{
    if (sentLength)
    {
        *sentLength = 0;
    }
    if (data.empty())
    {
        return boost::system::errc::make_error_code(boost::system::errc::no_message_available);
    }
    if (m_socket.is_open())
    {
        boost::system::error_code code;
        size_t sentLen = 0;
        while (sentLen < data.size()) /* 循环发送所有数据 */
        {
            auto len = m_socket.send(boost::asio::buffer(data.data() + sentLen, data.size() - sentLen),
                                     boost::asio::socket_base::message_flags(0), code);
            sentLen += len;
            if (code)
            {
                break;
            }
        }
        if (sentLength)
        {
            *sentLength = sentLen;
        }
        return code;
    }
    return boost::system::errc::make_error_code(boost::system::errc::not_connected);
}

boost::system::error_code SyncTcpClient::recv(std::vector<unsigned char>& data, const std::chrono::steady_clock::duration& timeout)
{
    data.clear();
    if (m_socket.is_open())
    {
        auto ec = std::make_shared<boost::system::error_code>();
        auto recvLength = std::make_shared<size_t>(0);
        m_socket.async_receive(boost::asio::buffer(m_recvBuf), [ec, recvLength](const boost::system::error_code& code, size_t length) {
            *ec = code;
            *recvLength = length;
        });
        if (!runImpl(timeout))
        {
            *ec = boost::system::errc::make_error_code(boost::system::errc::timed_out);
        }
        if (!*ec)
        {
            const unsigned char* rawData = (const unsigned char*)m_recvBuf.data();
            if (rawData && *recvLength > 0)
            {
                data.insert(data.end(), rawData, rawData + *recvLength);
            }
        }
        return *ec;
    }
    return boost::system::errc::make_error_code(boost::system::errc::not_connected);
}

void SyncTcpClient::stop()
{
    if (m_socket.is_open())
    {
        boost::system::error_code code;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, code);
        m_socket.close(code);
    }
    m_ioContext.stop();
}

bool SyncTcpClient::isNonBlock() const
{
    return m_socket.non_blocking();
}

int SyncTcpClient::getSendBufferSize() const
{
    boost::asio::socket_base::send_buffer_size opt;
    m_socket.get_option(opt);
    return opt.value();
}

int SyncTcpClient::getRecvBufferSize() const
{
    boost::asio::socket_base::receive_buffer_size opt;
    m_socket.get_option(opt);
    return opt.value();
}

bool SyncTcpClient::isNagleEnable() const
{
    boost::asio::ip::tcp::no_delay opt;
    m_socket.get_option(opt);
    return opt.value();
}

bool SyncTcpClient::runImpl(const std::chrono::steady_clock::duration& timeout)
{
    m_ioContext.restart();
    m_ioContext.run_for(timeout);
    if (!m_ioContext.stopped()) /* 超时 */
    {
        return false;
    }
    return true;
}
} // namespace nsocket
