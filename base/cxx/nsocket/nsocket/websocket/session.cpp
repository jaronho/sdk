#include "session.h"

namespace nsocket
{
namespace ws
{
void Session::sendText(const std::string& text, bool isFin)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createTextFrame(data, text, nullptr, isFin);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = wpConn.lock();
            if (conn)
            {
                if (code) /* ����ʧ�� */
                {
                    conn->close();
                }
            }
        });
    }
}

void Session::sendBytes(const std::vector<unsigned char>& bytes, bool isFin)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createBinaryFrame(data, bytes, nullptr, isFin);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = wpConn.lock();
            if (conn)
            {
                if (code) /* ����ʧ�� */
                {
                    conn->close();
                }
            }
        });
    }
}

void Session::sendClose(const CloseCode& code)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createCloseFrame(data, code);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = wpConn.lock();
            if (conn) /* �����жϷ��ͽ��, ֱ�ӶϿ����� */
            {
                conn->close();
            }
        });
    }
}

void Session::sendPing()
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createPingFrame(data);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = wpConn.lock();
            if (conn)
            {
                if (code) /* ����ʧ�� */
                {
                    conn->close();
                }
            }
        });
    }
}

void Session::sendPong()
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::vector<unsigned char> data;
        Frame::createPongFrame(data);
        conn->send(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto conn = wpConn.lock();
            if (conn)
            {
                if (code) /* ����ʧ�� */
                {
                    conn->close();
                }
            }
        });
    }
}
} // namespace ws
} // namespace nsocket
