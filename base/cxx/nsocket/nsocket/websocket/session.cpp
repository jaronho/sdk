#include "session.h"

namespace nsocket
{
namespace ws
{
uint64_t Session::getId() const
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        return conn->getId();
    }
    return 0;
}

std::string Session::getClientHost() const
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        return conn->getRemoteEndpoint().address().to_string();
    }
    return "";
}

int Session::getClientPort() const
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        return (int)conn->getRemoteEndpoint().port();
    }
    return 0;
}

std::string Session::getUri() const
{
    return m_req->uri;
}

void Session::sendText(const std::string& text, bool isFin, const TCP_SEND_CALLBACK& onSendCb)
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createTextFrame(data, text, false, isFin);
        conn->send(data, [&, onSendCb](const boost::system::error_code& code, size_t length) {
            if (onSendCb)
            {
                onSendCb(code, length);
            }
            const auto conn = m_wpConn.lock();
            if (conn)
            {
                if (code && boost::system::errc::not_connected != code) /* 发送失败 */
                {
                    conn->close();
                }
            }
        });
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void Session::sendBytes(const std::vector<unsigned char>& bytes, bool isFin, const TCP_SEND_CALLBACK& onSendCb)
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createBinaryFrame(data, bytes, false, isFin);
        conn->send(data, [&, onSendCb](const boost::system::error_code& code, size_t length) {
            if (onSendCb)
            {
                onSendCb(code, length);
            }
            const auto conn = m_wpConn.lock();
            if (conn)
            {
                if (code && boost::system::errc::not_connected != code) /* 发送失败 */
                {
                    conn->close();
                }
            }
        });
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void Session::sendPing(const TCP_SEND_CALLBACK& onSendCb)
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createPingFrame(data, false);
        conn->send(data, [&, onSendCb](const boost::system::error_code& code, size_t length) {
            if (onSendCb)
            {
                onSendCb(code, length);
            }
            const auto conn = m_wpConn.lock();
            if (conn)
            {
                if (code && boost::system::errc::not_connected != code) /* 发送失败 */
                {
                    conn->close();
                }
            }
        });
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void Session::sendPong(const TCP_SEND_CALLBACK& onSendCb)
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createPongFrame(data, false);
        conn->send(data, [&, onSendCb](const boost::system::error_code& code, size_t length) {
            if (onSendCb)
            {
                onSendCb(code, length);
            }
            const auto conn = m_wpConn.lock();
            if (conn)
            {
                if (code && boost::system::errc::not_connected != code) /* 发送失败 */
                {
                    conn->close();
                }
            }
        });
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void Session::sendClose(const CloseCode& code, const TCP_SEND_CALLBACK& onSendCb)
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createCloseFrame(data, code, false);
        conn->send(data, [&, onSendCb](const boost::system::error_code& code, size_t length) {
            if (onSendCb)
            {
                onSendCb(code, length);
            }
            const auto conn = m_wpConn.lock();
            if (conn) /* 无需判断发送结果, 直接断开连接 */
            {
                conn->close();
            }
        });
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

bool Session::isMsgText() const
{
    return m_isMsgText;
}
} // namespace ws
} // namespace nsocket
