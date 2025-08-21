#include "session.h"

#include "server.h"

namespace nsocket
{
namespace ftp
{
Session::Session(const std::weak_ptr<TcpConnection>& wpConn, const std::string& rootPath, size_t bz)
    : m_wpConn(wpConn), m_dataBufferSize(bz), m_rootPath(rootPath), m_currentPath("/")
{
#ifdef _WIN32
    /* Windows下把根目录里的/换成\, 并去掉末尾反斜杠 */
    std::replace(m_rootPath.begin(), m_rootPath.end(), '/', '\\');
    if (!m_rootPath.empty() && '\\' == m_rootPath.back())
    {
        m_rootPath.pop_back();
    }
#endif
}

Session::~Session()
{
    closeDataConnection();
}

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

void Session::onCommandRecv(const std::vector<unsigned char>& data)
{
    m_cmdBuffer.append(data.begin(), data.end());
    while (1)
    {
        auto pos = m_cmdBuffer.find("\r\n"); /* 查找完整命令(以\r\n结束) */
        if (std::string::npos == pos)
        {
            break;
        }
        auto cmdLine = m_cmdBuffer.substr(0, pos);
        m_cmdBuffer.erase(0, pos + 2);
        handleCommand(cmdLine);
    }
}

void Session::start()
{
    sendReply(ReplyCode::Ready, "FTP Server ready");
}

void Session::sendReply(const ReplyCode& code, const std::string& msg)
{
    auto conn = m_wpConn.lock();
    if (conn)
    {
        std::string reply = std::to_string((int)(code)) + " " + msg + "\r\n";
        conn->send(std::vector<unsigned char>(reply.begin(), reply.end()), nullptr);
    }
}

void Session::handleCommand(const std::string& cmdLine)
{
    if (cmdLine.empty())
    {
        return;
    }
    std::string cmd, arg;
    auto pos = cmdLine.find(' ');
    if (std::string::npos == pos)
    {
        cmd = cmdLine;
    }
    else
    {
        cmd = cmdLine.substr(0, pos);
        arg = cmdLine.substr(pos + 1);
    }
    switch (parseCommand(cmd))
    {
    case Command::USER:
        handleUser(arg);
        break;
    case Command::PASS:
        handlePass(arg);
        break;
    case Command::QUIT:
        handleQuit();
        break;
    case Command::PORT:
        handlePort(arg);
        break;
    case Command::PASV:
        handlePasv();
        break;
    case Command::TYPE:
        handleType(arg);
        break;
    case Command::RETR:
        handleRetr(arg);
        break;
    case Command::STOR:
        handleStor(arg);
        break;
    case Command::LIST:
        handleList(arg);
        break;
    case Command::NLST:
        handleNlst(arg);
        break;
    case Command::CWD:
        handleCwd(arg);
        break;
    case Command::CDUP:
        handleCdup();
        break;
    case Command::PWD:
        handlePwd();
        break;
    case Command::MKD:
        handleMkd(arg);
        break;
    case Command::RMD:
        handleRmd(arg);
        break;
    case Command::DELE:
        handleDele(arg);
        break;
    case Command::RNFR:
        handleRnfr(arg);
        break;
    case Command::RNTO:
        handleRnto(arg);
        break;
    case Command::SYST:
        handleSyst();
        break;
    case Command::NOOP:
        handleNoop();
        break;
    default:
        sendReply(ReplyCode::BadCommand, "Unknown command");
        break;
    }
}

void Session::handleUser(const std::string& arg) {}

void Session::handlePass(const std::string& arg) {}

void Session::handleQuit()
{
    sendReply(ReplyCode::Closing, "Goodbye");
    auto conn = m_wpConn.lock();
    if (conn)
    {
        conn->close();
    }
}

void Session::handlePort(const std::string& arg) {}

void Session::handlePasv() {}

void Session::handleType(const std::string& arg)
{
    if ("A" == arg || "a" == arg)
    {
    }
    else if ("I" == arg || "i" == arg)
    {
    }
    else
    {
    }
}

void Session::handleRetr(const std::string& arg) {}

void Session::handleStor(const std::string& arg) {}

void Session::handleList(const std::string& arg) {}

void Session::handleNlst(const std::string& arg) {}

void Session::handleCwd(const std::string& arg) {}

void Session::handleCdup() {}

void Session::handlePwd() {}

void Session::handleMkd(const std::string& arg) {}

void Session::handleRmd(const std::string& arg) {}

void Session::handleDele(const std::string& arg) {}

void Session::handleRnfr(const std::string& arg) {}

void Session::handleRnto(const std::string& arg) {}

void Session::handleSyst() {}

void Session::handleNoop() {}

void Session::setupDataConnection()
{
    if (m_dataConn && m_dataConn->isConnected())
    {
        sendReply(ReplyCode::DataConnOpen, "Data connection open");
    }
    else
    {
        sendReply(ReplyCode::FileUnavailable, "No data connection");
    }
}

void Session::closeDataConnection()
{
    if (m_dataConn && m_dataConn->isConnected())
    {
        m_dataConn->close();
        m_dataConn.reset();
    }
}

void Session::onDataConnectionRecv(const std::vector<unsigned char>& data) {}

void Session::sendFileData(const std::string& path) {}

void Session::recvFileData(const std::string& path) {}
} // namespace ftp
} // namespace nsocket
