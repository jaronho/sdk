#include "session.h"

namespace nsocket
{
namespace ws
{
void Session::sendText(const std::string& text, bool isFin)
{
    const auto tcpSession = wpTcpSession.lock();
    if (tcpSession)
    {
        std::vector<unsigned char> data;
        Frame::createTextFrame(data, text, nullptr, isFin);
        tcpSession->send(data, [&](const boost::system::error_code& code, std::size_t length) {});
    }
}

void Session::sendBytes(const std::vector<unsigned char>& bytes, bool isFin)
{
    const auto tcpSession = wpTcpSession.lock();
    if (tcpSession)
    {
        std::vector<unsigned char> data;
        Frame::createBinaryFrame(data, bytes, nullptr, isFin);
        tcpSession->send(data, [&](const boost::system::error_code& code, std::size_t length) {});
    }
}

void Session::sendClose(const CloseCode& code)
{
    const auto tcpSession = wpTcpSession.lock();
    if (tcpSession)
    {
        std::vector<unsigned char> data;
        Frame::createCloseFrame(data, code);
        tcpSession->send(data, [&](const boost::system::error_code& code, std::size_t length) {
            const auto tcpSession = wpTcpSession.lock();
            if (tcpSession)
            {
                tcpSession->close();
            }
        });
    }
}

void Session::sendPing()
{
    const auto tcpSession = wpTcpSession.lock();
    if (tcpSession)
    {
        std::vector<unsigned char> data;
        Frame::createPingFrame(data);
        tcpSession->send(data, [&](const boost::system::error_code& code, std::size_t length) {});
    }
}

void Session::sendPong()
{
    const auto tcpSession = wpTcpSession.lock();
    if (tcpSession)
    {
        std::vector<unsigned char> data;
        Frame::createPongFrame(data);
        tcpSession->send(data, [&](const boost::system::error_code& code, std::size_t length) {});
    }
}
} // namespace ws
} // namespace nsocket
