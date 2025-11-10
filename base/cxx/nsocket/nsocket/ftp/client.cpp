#include "client.h"

namespace nsocket
{
namespace ftp
{
Client::Client(uint16_t localPort, size_t bz) : m_localPort(localPort), m_bufferSize(bz) {}

Client::~Client()
{
    stop();
}

void Client::setNonBlock(bool nonBlock)
{
    m_nonBlock = (nonBlock ? 1 : 0);
}

void Client::setSendBufferSize(int bufferSize)
{
    m_sendBufferSize = bufferSize;
}

void Client::setRecvBufferSize(int bufferSize)
{
    m_recvBufferSize = bufferSize;
}

void Client::setNagleEnable(bool enable)
{
    m_enableNagle = (enable ? 1 : 0);
}

void Client::run(const std::string& host, uint16_t port, bool sslOn, int sslWay, int certFmt, const std::string& certFile,
                 const std::string& pkFile, const std::string& pkPwd)
{
    auto tcpClient = std::make_shared<nsocket::TcpClient>(m_localPort, m_bufferSize);
    const std::weak_ptr<Client> wpSelf = shared_from_this();
    tcpClient->setConnectCallback([wpSelf](const boost::system::error_code& code) {
        const auto self = wpSelf.lock();
        if (self)
        {
            self->handleConnect(code);
        }
    });
    tcpClient->setDataCallback([wpSelf](const std::vector<unsigned char>& data) {
        const auto self = wpSelf.lock();
        if (self)
        {
            self->handleData(data);
        }
    });
    if (m_nonBlock >= 0)
    {
        tcpClient->setNonBlock(m_nonBlock > 0 ? true : false);
    }
    if (m_sendBufferSize > 0)
    {
        tcpClient->setSendBufferSize(m_sendBufferSize);
    }
    if (m_recvBufferSize > 0)
    {
        tcpClient->setRecvBufferSize(m_recvBufferSize);
    }
    if (m_enableNagle >= 0)
    {
        tcpClient->setNagleEnable(m_enableNagle > 0 ? true : false);
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpClient);
        m_tcpClient = tcpClient;
    }
    tcpClient->run(host, port, sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
}

bool Client::isRunning()
{
    std::shared_ptr<TcpClient> tcpClient = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpClient);
        tcpClient = m_tcpClient;
    }
    if (tcpClient && tcpClient->isRunning())
    {
        return true;
    }
    return false;
}

bool Client::isNonBlock()
{
    std::shared_ptr<TcpClient> tcpClient = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpClient);
        tcpClient = m_tcpClient;
    }
    if (tcpClient)
    {
        return tcpClient->isNonBlock();
    }
    return false;
}

int Client::getSendBufferSize()
{
    std::shared_ptr<TcpClient> tcpClient = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpClient);
        tcpClient = m_tcpClient;
    }
    if (tcpClient)
    {
        return tcpClient->getSendBufferSize();
    }
    return -1;
}

int Client::getRecvBufferSize()
{
    std::shared_ptr<TcpClient> tcpClient = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpClient);
        tcpClient = m_tcpClient;
    }
    if (tcpClient)
    {
        return tcpClient->getRecvBufferSize();
    }
    return -1;
}

bool Client::isNagleEnable()
{
    std::shared_ptr<TcpClient> tcpClient = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpClient);
        tcpClient = m_tcpClient;
    }
    if (tcpClient)
    {
        return tcpClient->isNagleEnable();
    }
    return false;
}

boost::asio::ip::tcp::endpoint Client::getLocalEndpoint()
{
    std::shared_ptr<TcpClient> tcpClient = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpClient);
        tcpClient = m_tcpClient;
    }
    if (tcpClient)
    {
        return tcpClient->getLocalEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}

boost::asio::ip::tcp::endpoint Client::getRemoteEndpoint()
{
    std::shared_ptr<TcpClient> tcpClient = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpClient);
        tcpClient = m_tcpClient;
    }
    if (tcpClient)
    {
        return m_tcpClient->getRemoteEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}

bool Client::login(const std::string& user, const std::string& password)
{
    std::string response;
    if (sendCommand("USER " + user, ReplyCode::user_ok, &response))
    {
        return sendCommand("PASS " + password, ReplyCode::login_ok, &response);
    }
    return false;
}

void Client::stop()
{
    std::shared_ptr<TcpClient> tcpClient = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpClient);
        tcpClient = m_tcpClient;
        m_tcpClient.reset();
    }
    if (tcpClient)
    {
        tcpClient->stop();
    }
}

void Client::handleConnect(const boost::system::error_code& code)
{
    if (code) /* 连接失败 */
    {
    }
    else /* 连接成功 */
    {
    }
}

void Client::handleData(const std::vector<unsigned char>& data)
{
    {
        std::lock_guard<std::mutex> locker(m_mutexCmdBuffer);
        m_cmdBuffer.append(data.begin(), data.end());
    }
    while (1)
    {
        std::string line;
        {
            std::lock_guard<std::mutex> locker(m_mutexCmdBuffer);
            auto pos = m_cmdBuffer.find("\r\n"); /* 解析FTP响应: 以\r\n分隔 */
            if (std::string::npos == pos)
            {
                break;
            }
            line = m_cmdBuffer.substr(0, pos);
            m_cmdBuffer.erase(0, pos + 2);
        }
    }
}

bool Client::waitForReply(const ReplyCode& expectedCode, std::string* response, size_t timeout)
{
    timeout = (0 == timeout) ? 2000 : timeout;
    std::unique_lock<std::mutex> locker(m_mutexReply);
    auto status = m_cvReply.wait_for(locker, std::chrono::milliseconds(timeout),
                                     [&, expectedCode]() { return (!m_replyQueue.empty() && expectedCode == m_replyQueue.front().first); });
    if (status)
    {
        if (response)
        {
            *response = m_replyQueue.front().second;
        }
        m_replyQueue.pop();
        return true;
    }
    return false;
}

bool Client::sendCommand(const std::string& cmd, const ReplyCode& expectedCode, std::string* response)
{
    std::shared_ptr<TcpClient> tcpClient;
    {
        std::lock_guard<std::mutex> lock(m_mutexTcpClient);
        tcpClient = m_tcpClient;
    }
    if (!tcpClient)
    {
        return false;
    }
    /* 发送FTP命令 */
    std::string cmdData = cmd + "\r\n";
    size_t sent;
    if (tcpClient->send(std::vector<unsigned char>(cmdData.begin(), cmdData.end()), sent))
    {
        return false;
    }
    /* 等待响应 */
    return waitForReply(expectedCode, response);
}
} // namespace ftp
} // namespace nsocket
