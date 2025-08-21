#include "server.h"

namespace nsocket
{
namespace ftp
{
Server::Server(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, bool reuseAddr, size_t bz,
               const std::chrono::steady_clock::duration& handshakeTimeout)
    : m_dataBufferSize(bz)
{
    m_tcpServer = std::make_shared<TcpServer>(name, threadCount, host, port, reuseAddr, bz, handshakeTimeout);
}

Server::~Server()
{
    m_tcpServer->stop();
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        m_sessionMap.clear();
    }
}

void Server::setRootPath(const std::string& rootPath)
{
    {
        std::lock_guard<std::mutex> locker(m_mutexRootPath);
        m_rootPath = rootPath;
    }
}

bool Server::run(bool sslOn, int sslWay, int certFmt, const std::string& certFile, const std::string& pkFile, const std::string& pkPwd,
                 std::string* errDesc)
{
    const std::weak_ptr<Server> wpSelf = shared_from_this();
    m_tcpServer->setNewConnectionCallback([wpSelf](const std::weak_ptr<TcpConnection>& wpConn) {
        const auto& self = wpSelf.lock();
        if (self)
        {
            if (!self->m_tcpServer->isEnableSSL())
            {
                self->handleNewConnection(wpConn);
            }
        }
    });
    m_tcpServer->setHandshakeOkCallback([wpSelf](const std::weak_ptr<nsocket::TcpConnection>& wpConn) {
        const auto& self = wpSelf.lock();
        if (self)
        {
            self->handleNewConnection(wpConn);
        }
    });
    m_tcpServer->setConnectionDataCallback([wpSelf](const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data) {
        const auto& self = wpSelf.lock();
        if (self)
        {
            self->handleConnectionData(wpConn, data);
        }
    });
    m_tcpServer->setConnectionCloseCallback(
        [wpSelf](uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
            const auto& self = wpSelf.lock();
            if (self)
            {
                self->handleConnectionClose(cid);
            }
        });
    return m_tcpServer->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd, errDesc);
}

void Server::stop()
{
    m_tcpServer->stop();
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        m_sessionMap.clear();
    }
}

bool Server::isRunning()
{
    return m_tcpServer->isRunning();
}

void Server::handleNewConnection(const std::weak_ptr<TcpConnection>& wpConn)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        auto cid = conn->getId();
        std::shared_ptr<Session> session = nullptr;
        std::string rootPath;
        {
            std::lock_guard<std::mutex> locker(m_mutexRootPath);
            rootPath = m_rootPath;
        }
        {
            std::lock_guard<std::mutex> lock(m_mutexSessionMap);
            if (m_sessionMap.end() == m_sessionMap.find(cid))
            {
                session = std::make_shared<Session>(wpConn, rootPath, m_dataBufferSize);
                m_sessionMap.insert(std::make_pair(cid, session));
            }
        }
        if (session)
        {
            session->start();
        }
    }
}

void Server::handleConnectionData(const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        auto cid = conn->getId();
        std::shared_ptr<Session> session = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_mutexSessionMap);
            auto iter = m_sessionMap.find(cid);
            if (m_sessionMap.end() != iter)
            {
                session = iter->second;
            }
        }
        if (session)
        {
            session->onCommandRecv(data);
        }
    }
}

void Server::handleConnectionClose(uint64_t cid)
{
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        auto iter = m_sessionMap.find(cid);
        if (m_sessionMap.end() != iter)
        {
            m_sessionMap.erase(iter);
        }
    }
}
} // namespace ftp
} // namespace nsocket
