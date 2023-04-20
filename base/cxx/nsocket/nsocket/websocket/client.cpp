#include "client.h"

namespace nsocket
{
namespace ws
{
Client::Client(uint16_t localPort, size_t bz)
{
    m_tcpClient = std::make_shared<nsocket::TcpClient>(localPort, bz);
    m_tcpClient->setConnectCallback([&](const boost::system::error_code& code) { handleConnect(code); });
    m_tcpClient->setDataCallback([&](const std::vector<unsigned char>& data) { handleData(data); });
}

Client::~Client()
{
    stop();
}

void Client::setConnectingCallback(const WS_CLI_CONNECTING_CALLBACK& cb)
{
    m_onConnectingCallback = cb;
}

void Client::setOpenCallback(const WS_CLI_OPEN_CALLBACK& cb)
{
    m_onOpenCallback = cb;
}

void Client::setPingCallback(const WS_CLI_PING_CALLBACK& cb)
{
    m_onPingCallback = cb;
}

void Client::setPongCallback(const WS_CLI_PONG_CALLBACK& cb)
{
    m_onPongCallback = cb;
}

void Client::setMessager(const std::shared_ptr<CliMessager>& msger)
{
    m_messager = msger;
}

void Client::setCloseCallback(const WS_CLI_CLOSE_CALLBACK& cb)
{
    m_onCloseCallback = cb;
}

void Client::run(const std::string& hostPortPath, uint16_t defaultPort, bool sslOn, int sslWay, int certFmt, const std::string& certFile,
                 const std::string& pkFile, const std::string& pkPwd)
{
    size_t beg = 0;
    if (std::string::npos != hostPortPath.find("ws://"))
    {
        beg = 5;
    }
    else if (std::string::npos != hostPortPath.find("wss://"))
    {
        beg = 6;
    }
    /* 解析远端地址/端口/URI */
    std::string host;
    uint16_t port;
    auto hostEndPos = hostPortPath.find(':', beg);
    auto portEndPos = hostPortPath.find('/', beg);
    if (std::string::npos == hostEndPos)
    {
        hostEndPos = portEndPos;
        port = defaultPort;
    }
    else
    {
        try
        {
            if (std::string::npos == portEndPos)
            {
                port = static_cast<uint16_t>(stoul(hostPortPath.substr(hostEndPos + 1)));
            }
            else
            {
                port = static_cast<uint16_t>(stoul(hostPortPath.substr(hostEndPos + 1, portEndPos - (hostEndPos + 1))));
            }
        }
        catch (...)
        {
            port = 0;
        }
    }
    host = (std::string::npos == hostEndPos) ? hostPortPath.substr(beg) : hostPortPath.substr(beg, hostEndPos - beg);
    m_hostPort = hostPortPath.substr(beg, portEndPos - beg);
    m_uri = (std::string::npos == portEndPos) ? "/" : hostPortPath.substr(portEndPos);
    m_tcpClient->run(host, port, sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
}

void Client::sendText(const std::string& text, bool isFin, const TCP_SEND_CALLBACK& onSendCb)
{
    std::vector<unsigned char> data;
    Frame::createTextFrame(data, text, true, isFin);
    m_tcpClient->sendAsync(data, [&, onSendCb](const boost::system::error_code& code, size_t length) {
        if (onSendCb)
        {
            onSendCb(code, length);
        }
        if (code && boost::system::errc::not_connected != code) /* 发送失败 */
        {
            stop();
        }
    });
}

void Client::sendBytes(const std::vector<unsigned char>& bytes, bool isFin, const TCP_SEND_CALLBACK& onSendCb)
{
    std::vector<unsigned char> data;
    Frame::createBinaryFrame(data, bytes, true, isFin);
    m_tcpClient->sendAsync(data, [&](const boost::system::error_code& code, size_t length) {
        if (onSendCb)
        {
            onSendCb(code, length);
        }
        if (code && boost::system::errc::not_connected != code) /* 发送失败 */
        {
            stop();
        }
    });
}

void Client::sendPing(const TCP_SEND_CALLBACK& onSendCb)
{
    std::vector<unsigned char> data;
    Frame::createPingFrame(data, true);
    m_tcpClient->sendAsync(data, [&](const boost::system::error_code& code, size_t length) {
        if (onSendCb)
        {
            onSendCb(code, length);
        }
        if (code && boost::system::errc::not_connected != code) /* 发送失败 */
        {
            stop();
        }
    });
}

void Client::sendPong(const TCP_SEND_CALLBACK& onSendCb)
{
    std::vector<unsigned char> data;
    Frame::createPongFrame(data, true);
    m_tcpClient->sendAsync(data, [&](const boost::system::error_code& code, size_t length) {
        if (onSendCb)
        {
            onSendCb(code, length);
        }
        if (code && boost::system::errc::not_connected != code) /* 发送失败 */
        {
            stop();
        }
    });
}

void Client::sendClose(const CloseCode& code, const TCP_SEND_CALLBACK& onSendCb)
{
    std::vector<unsigned char> data;
    Frame::createCloseFrame(data, code, true);
    m_tcpClient->sendAsync(data, [&](const boost::system::error_code& code, size_t length) {
        if (onSendCb)
        {
            onSendCb(code, length);
        }
        stop(); /* 无需判断发送结果, 直接断开连接 */
    });
}

bool Client::isRunning() const
{
    return m_tcpClient->isRunning();
}

boost::asio::ip::tcp::endpoint Client::getLocalEndpoint() const
{
    return m_tcpClient->getLocalEndpoint();
}

boost::asio::ip::tcp::endpoint Client::getRemoteEndpoint() const
{
    return m_tcpClient->getRemoteEndpoint();
}

std::string Client::getUri() const
{
    return m_uri;
}

void Client::stop()
{
    m_tcpClient->stop();
}

void Client::handleConnect(const boost::system::error_code& code)
{
    if (code) /* 连接失败 */
    {
        m_onCloseCallback(code);
    }
    else /* 连接成功 */
    {
        std::shared_ptr<Request> req = nullptr;
        if (m_onConnectingCallback)
        {
            req = m_onConnectingCallback();
        }
        if (!req)
        {
            req = std::make_shared<Request>();
        }
        req->uri = m_uri;
        std::vector<unsigned char> data;
        Request::create(*req, m_hostPort, m_secWebSocketKey, data);
        m_resp = std::make_shared<Response>();
        m_frame = std::make_shared<Frame>();
        /* 发送首次请求 */
        size_t sentLength;
        auto code = m_tcpClient->send(data, sentLength);
        if (code) /* 失败, 则需要关闭连接 */
        {
            stop();
        }
    }
}

void Client::handleData(const std::vector<unsigned char>& data)
{
    int used = 0;
    auto frameHeadCb = [&]() { handleFrameHead(); };
    auto framePayloadCb = [&](size_t offset, const unsigned char* data, int dataLen) { handleFramePayload(offset, data, dataLen); };
    auto frameFinishCb = [&]() { handleFrameFinish(); };
    if (m_resp->isParseEnd()) /* 响应处理完毕, 后续收到的都是帧数据 */
    {
        used = m_frame->parse(data.data(), data.size(), frameHeadCb, framePayloadCb, frameFinishCb);
    }
    else /* 响应未处理结束, 需要继续解析 */
    {
        used = m_resp->parse(data.data(), data.size(), m_secWebSocketKey, [&]() { handleResponse(); });
        int remainLen = data.size() - used;
        if (remainLen > 0 && m_resp->isParseEnd()) /* 有剩余数据, 则视为帧数据(该情况几乎不会出现, 这里只是防御性处理) */
        {
            const unsigned char* remainData = data.data() + used;
            used = m_frame->parse(remainData, remainLen, frameHeadCb, framePayloadCb, frameFinishCb);
        }
    }
    if (used <= 0) /* 解析失败 */
    {
        sendClose(CloseCode::close_protocol_error);
    }
}

void Client::handleResponse()
{
    if (m_onOpenCallback)
    {
        m_onOpenCallback();
    }
}

void Client::handleFrameHead()
{
    if (1 == m_frame->opcode || 2 == m_frame->opcode) /* 首帧 */
    {
        m_isMsgText = (1 == m_frame->opcode);
        if (m_messager)
        {
            m_messager->onMessageBegin(m_isMsgText);
        }
    }
}

void Client::handleFramePayload(size_t offset, const unsigned char* data, int dataLen)
{
    if (0 == m_frame->opcode || 1 == m_frame->opcode || 2 == m_frame->opcode)
    {
        if (m_messager)
        {
            m_messager->onMessagePayload(m_isMsgText, offset, data, dataLen);
        }
    }
}

void Client::handleFrameFinish()
{
    if (0 == m_frame->opcode || 1 == m_frame->opcode || 2 == m_frame->opcode)
    {
        if (1 == m_frame->fin) /* 尾帧 */
        {
            if (m_messager)
            {
                m_messager->onMessageEnd(m_isMsgText);
            }
        }
    }
    else if (0x8 == m_frame->opcode) /* 服务端关闭连接 */
    {
        sendClose(CloseCode::close_normal);
    }
    else if (0x9 == m_frame->opcode) /* ping */
    {
        if (m_onPingCallback)
        {
            m_onPingCallback();
        }
        else
        {
            sendPong(); /* 需要回复pong给服务端 */
        }
    }
    else if (0xA == m_frame->opcode) /* pong */
    {
        if (m_onPongCallback)
        {
            m_onPongCallback();
        }
    }
}
} // namespace ws
} // namespace nsocket
