#include "session.h"

namespace nsocket
{
namespace ws
{
int64_t Session::getId()
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        return conn->getId();
    }
    return 0;
}

void Session::sendText(const std::string& text, bool isFin)
{
    const auto conn = m_wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createTextFrame(data, text, nullptr, isFin);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
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
        Frame::createBinaryFrame(data, bytes, nullptr, isFin);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
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
        Frame::createCloseFrame(data, code);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
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
        Frame::createPingFrame(data);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
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
        Frame::createPongFrame(data);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
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

bool Session::isMsgText()
{
    return m_isMsgText;
}

} // namespace ws
} // namespace nsocket
