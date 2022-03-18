#include "session.h"

namespace nsocket
{
namespace ws
{
int64_t Session::getId() const
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

void Session::sendText(const std::string& text, bool isFin)
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createTextFrame(data, text, false, isFin);
        conn->sendAsync(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = m_wpConn.lock();
            if (conn)
            {
                if (code) /* 发送失败 */
                {
                    conn->close();
                }
            }
        });
    }
}

void Session::sendBytes(const std::vector<unsigned char>& bytes, bool isFin)
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createBinaryFrame(data, bytes, false, isFin);
        conn->sendAsync(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = m_wpConn.lock();
            if (conn)
            {
                if (code) /* 发送失败 */
                {
                    conn->close();
                }
            }
        });
    }
}

void Session::sendClose(const CloseCode& code)
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createCloseFrame(data, code, false);
        conn->sendAsync(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = m_wpConn.lock();
            if (conn) /* 无需判断发送结果, 直接断开连接 */
            {
                conn->close();
            }
        });
    }
}

void Session::sendPing()
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createPingFrame(data, false);
        conn->sendAsync(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = m_wpConn.lock();
            if (conn)
            {
                if (code) /* 发送失败 */
                {
                    conn->close();
                }
            }
        });
    }
}

void Session::sendPong()
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createPongFrame(data, false);
        conn->sendAsync(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = m_wpConn.lock();
            if (conn)
            {
                if (code) /* 发送失败 */
                {
                    conn->close();
                }
            }
        });
    }
}

bool Session::isMsgText() const
{
    return m_isMsgText;
}

} // namespace ws
} // namespace nsocket
