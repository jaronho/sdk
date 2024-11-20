#include "server.h"

#include "request.h"

namespace nsocket
{
namespace ws
{
Server::Server(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, bool reuseAddr, size_t bz,
               const std::chrono::steady_clock::duration& handshakeTimeout)
{
    m_tcpServer = std::make_shared<TcpServer>(name, threadCount, host, port, reuseAddr, bz, handshakeTimeout);
}

Server::~Server()
{
    m_tcpServer->stop();
}

void Server::setConnectingCallback(const WS_SRV_CONNECTING_CALLBACK& cb)
{
    m_onConnectingCallback = cb;
}

void Server::setOpenCallback(const WS_SRV_OPEN_CALLBACK& cb)
{
    m_onOpenCallback = cb;
}

void Server::setPingCallback(const WS_SRV_PING_CALLBACK& cb)
{
    m_onPingCallback = cb;
}

void Server::setPongCallback(const WS_SRV_PONG_CALLBACK& cb)
{
    m_onPongCallback = cb;
}

void Server::setMessager(const std::shared_ptr<SrvMessager>& msger)
{
    m_messager = msger;
}

void Server::setCloseCallback(const WS_SRV_CLOSE_CALLBACK& cb)
{
    m_onCloseCallback = cb;
}

bool Server::run(bool sslOn, int sslWay, int certFmt, const std::string& certFile, const std::string& pkFile, const std::string& pkPwd,
                 std::string* errDesc)
{
    const std::weak_ptr<Server> wpSelf = shared_from_this();
    m_tcpServer->setNewConnectionCallback([wpSelf](const std::weak_ptr<TcpConnection>& wpConn) {
        const auto self = wpSelf.lock();
        if (self)
        {
            if (!self->m_tcpServer->isEnableSSL())
            {
                self->handleNewConnection(wpConn);
            }
        }
    });
    m_tcpServer->setHandshakeOkCallback([wpSelf](const std::weak_ptr<nsocket::TcpConnection>& wpConn) {
        const auto self = wpSelf.lock();
        if (self)
        {
            self->handleNewConnection(wpConn);
        }
    });
    m_tcpServer->setConnectionDataCallback([wpSelf](const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data) {
        const auto self = wpSelf.lock();
        if (self)
        {
            self->handleConnectionData(wpConn, data);
        }
    });
    m_tcpServer->setConnectionCloseCallback(
        [wpSelf](uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->handleConnectionClose(cid, point, code);
            }
        });
    return m_tcpServer->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd, errDesc);
}

void Server::stop()
{
    m_tcpServer->stop();
}

bool Server::isRunning()
{
    return m_tcpServer->isRunning();
}

std::unordered_map<uint64_t, std::weak_ptr<Session>> Server::getSessionMap()
{
    std::unordered_map<uint64_t, std::weak_ptr<Session>> sessionMap;
    std::lock_guard<std::mutex> locker(m_mutexSessionMap);
    for (auto iter = m_sessionMap.begin(); m_sessionMap.end() != iter; ++iter)
    {
        sessionMap.insert(std::make_pair(iter->first, iter->second));
    }
    return sessionMap;
}

void Server::handleNewConnection(const std::weak_ptr<TcpConnection>& wpConn)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        if (m_sessionMap.end() == m_sessionMap.find(conn->getId()))
        {
            auto session = std::make_shared<Session>();
            session->m_wpConn = wpConn;
            session->m_req = std::make_shared<Request>();
            session->m_frame = std::make_shared<Frame>();
            m_sessionMap.insert(std::make_pair(conn->getId(), session));
        }
    }
}

void Server::handleConnectionData(const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        std::shared_ptr<Session> session = nullptr;
        {
            /* 限定锁区间, 避免阻塞其他连接, 提高并发性 */
            std::lock_guard<std::mutex> locker(m_mutexSessionMap);
            auto iter = m_sessionMap.find(conn->getId());
            if (m_sessionMap.end() != iter)
            {
                session = iter->second;
            }
        }
        if (!session)
        {
            return;
        }
        int used = 0;
        auto frameHeadCb = [&]() { handleFrameHead(session); };
        auto framePayloadCb = [&](size_t offset, const unsigned char* data, int dataLen) {
            handleFramePayload(session, offset, data, dataLen);
        };
        auto frameFinishCb = [&]() { handleFrameFinish(session); };
        if (session->m_req->isParseEnd()) /* 请求处理完毕, 后续收到的都是帧数据 */
        {
            used = session->m_frame->parse(data.data(), data.size(), frameHeadCb, framePayloadCb, frameFinishCb);
        }
        else /* 请求未处理结束, 需要继续解析 */
        {
            used = session->m_req->parse(data.data(), data.size(), [&]() { handleRequest(session); });
            int remainLen = data.size() - used;
            if (remainLen > 0 && session->m_req->isParseEnd()) /* 有剩余数据, 则视为帧数据(该情况几乎不会出现, 这里只是防御性处理) */
            {
                const unsigned char* remainData = data.data() + used;
                used = session->m_frame->parse(remainData, remainLen, frameHeadCb, framePayloadCb, frameFinishCb);
            }
        }
        if (used <= 0) /* 解析失败 */
        {
            session->sendClose(CloseCode::close_protocol_error);
        }
    }
}

void Server::handleConnectionClose(uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)
{
    {
        /* 限定锁区间, 避免阻塞其他连接, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        auto iter = m_sessionMap.find(cid);
        if (m_sessionMap.end() != iter)
        {
            m_sessionMap.erase(iter);
        }
    }
    if (m_onCloseCallback)
    {
        m_onCloseCallback(cid, point, code);
    }
}

void Server::handleRequest(const std::shared_ptr<Session>& session)
{
    const auto conn = session->m_wpConn.lock();
    if (conn)
    {
        /* 收到客户端请求 */
        std::shared_ptr<Response> resp = nullptr;
        if (m_onConnectingCallback)
        {
            resp = m_onConnectingCallback(session);
        }
        if (!resp)
        {
            resp = std::make_shared<Response>();
        }
        std::vector<unsigned char> data;
        Response::create(*resp, session->m_req->getSecWebSocketKey(), data);
        /* 响应客户端, 用于通知客户端WebSocket连接建立成功 */
        const std::weak_ptr<Server> wpSelf = shared_from_this();
        std::weak_ptr<Session> wpSession = session;
        conn->send(data, [wpSelf, wpSession](const boost::system::error_code& code, size_t length) {
            const auto self = wpSelf.lock();
            if (self)
            {
                const auto session = wpSession.lock();
                if (session)
                {
                    if (code) /* 失败, 则需要关闭连接 */
                    {
                        session->sendClose(CloseCode::close_no_status);
                    }
                    else /* 成功, 表示连接建立成功 */
                    {
                        if (self->m_onOpenCallback)
                        {
                            self->m_onOpenCallback(session);
                        }
                    }
                }
            }
        });
    }
}

void Server::handleFrameHead(const std::shared_ptr<Session>& session)
{
    if (1 == session->m_frame->opcode || 2 == session->m_frame->opcode) /* 首帧 */
    {
        session->m_isMsgText = (1 == session->m_frame->opcode);
        if (m_messager)
        {
            m_messager->onMessageBegin(session);
        }
    }
}

void Server::handleFramePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen)
{
    if (0 == session->m_frame->opcode || 1 == session->m_frame->opcode || 2 == session->m_frame->opcode)
    {
        if (m_messager)
        {
            m_messager->onMessagePayload(session, offset, data, dataLen);
        }
    }
}

void Server::handleFrameFinish(const std::shared_ptr<Session>& session)
{
    const auto conn = session->m_wpConn.lock();
    if (conn)
    {
        if (0 == session->m_frame->opcode || 1 == session->m_frame->opcode || 2 == session->m_frame->opcode)
        {
            if (1 == session->m_frame->fin) /* 尾帧 */
            {
                if (m_messager)
                {
                    m_messager->onMessageEnd(session);
                }
            }
        }
        else if (0x8 == session->m_frame->opcode) /* 客户端关闭连接 */
        {
            session->sendClose(CloseCode::close_normal);
        }
        else if (0x9 == session->m_frame->opcode) /* ping */
        {
            if (m_onPingCallback)
            {
                m_onPingCallback(session);
            }
            else
            {
                session->sendPong(); /* 需要回复pong给客户端 */
            }
        }
        else if (0xA == session->m_frame->opcode) /* pong */
        {
            if (m_onPongCallback)
            {
                m_onPongCallback(session);
            }
        }
    }
}
} // namespace ws
} // namespace nsocket
