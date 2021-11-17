#include "server.h"

namespace nsocket
{
namespace ws
{
#define WS_DEBUG 0
Server::Server(const std::string& host, unsigned int port)
{
    m_tcpServer = std::make_shared<TcpServer>(host, port, true, 1024);
    m_tcpServer->setNewConnectionCallback([&](const std::weak_ptr<TcpConnection>& wpConn) { handleNewConnection(wpConn); });
    m_tcpServer->setConnectionDataCallback(
        [&](const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data) { handleConnectionData(wpConn, data); });
    m_tcpServer->setConnectionCloseCallback([&](int64_t cid, const boost::asio::ip::tcp::endpoint& point,
                                                const boost::system::error_code& code) { handleConnectionClose(cid, point, code); });
}

void Server::setConnectingCallback(const WS_CONNECTING_CALLBACK& cb)
{
    m_onConnectingCallback = cb;
}

void Server::setOpenCallback(const WS_OPEN_CALLBACK& cb)
{
    m_onOpenCallback = cb;
}

void Server::setPingCallback(const WS_PING_CALLBACK& cb)
{
    m_onPingCallback = cb;
}

void Server::setPongCallback(const WS_PONG_CALLBACK& cb)
{
    m_onPongCallback = cb;
}

void Server::setMessager(const std::shared_ptr<Messager>& msger)
{
    m_messager = msger;
}

void Server::setCloseCallback(const WS_CLOSE_CALLBACK& cb)
{
    m_onCloseCallback = cb;
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
void Server::run(const std::shared_ptr<boost::asio::ssl::context>& sslContext)
#else
void Server::run()
#endif
{
    if (m_tcpServer)
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        m_tcpServer->run(sslContext);
#else
        m_tcpServer->run();
#endif
    }
}

std::unordered_map<int64_t, std::weak_ptr<Session>> Server::getSessionMap()
{
    std::unordered_map<int64_t, std::weak_ptr<Session>> sessionMap;
    std::lock_guard<std::mutex> locker(m_mutex);
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
#if 1 == WS_DEBUG
        auto point = conn->getRemoteEndpoint();
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("============================== on new connection [%lld] [%s:%d]\n", conn->getId(), clientHost.c_str(), clientPort);
#endif
        std::lock_guard<std::mutex> locker(m_mutex);
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
            std::lock_guard<std::mutex> locker(m_mutex);
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
        auto frameHeadCb = [&]() {
            /* 扩展数据接收缓冲区 */
            int bufferSize = session->m_frame->payloadLen;
            if (bufferSize > 1024)
            {
                static const int MAX_BUFFER_SIZE = 65536; /* 64Kb */
                if (bufferSize > MAX_BUFFER_SIZE) /* 限制上限 */
                {
                    bufferSize = MAX_BUFFER_SIZE;
                }
                conn->resizeBuffer(bufferSize);
            }
            handleFrameHead(session);
        };
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

void Server::handleConnectionClose(int64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)
{
#if 1 == WS_DEBUG
    std::string clientHost = point.address().to_string().c_str();
    int clientPort = (int)point.port();
    if (code)
    {
        printf("-------------------- on connection closed [%lld] [%s:%d] fail, %d, %s\n", cid, clientHost.c_str(), clientPort, code.value(),
               code.message().c_str());
    }
    else
    {
        printf("-------------------- on connection closed [%lld] [%s:%d]\n", cid, clientHost.c_str(), clientPort);
    }
#endif
    {
        /* 限定锁区间, 避免阻塞其他连接, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_sessionMap.find(cid);
        if (m_sessionMap.end() != iter)
        {
            m_sessionMap.erase(iter);
        }
    }
    if (m_onCloseCallback)
    {
        m_onCloseCallback(cid);
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
        std::weak_ptr<Session> wpSession = session;
        conn->send(data, [&, wpSession](const boost::system::error_code& code, std::size_t length) {
            const auto session = wpSession.lock();
            if (session)
            {
                if (code) /* 失败, 则需要关闭连接 */
                {
                    session->sendClose(CloseCode::close_no_status);
                }
                else /* 成功, 表示连接建立成功 */
                {
                    if (m_onOpenCallback)
                    {
                        m_onOpenCallback(session);
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
#if 1 == WS_DEBUG
    const auto conn = session->m_wpConn.lock();
    if (conn)
    {
        auto point = conn->getRemoteEndpoint();
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("++++++++++ on recv frame [%lld] [%s:%d], offset: %zu, length: %d\n", conn->getId(), clientHost.c_str(), clientPort, offset,
               dataLen);
    }
#endif
}

void Server::handleFrameFinish(const std::shared_ptr<Session>& session)
{
    const auto conn = session->m_wpConn.lock();
    if (conn)
    {
#if 1 == WS_DEBUG
        auto point = conn->getRemoteEndpoint();
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("========== on recv frame [%lld] [%s:%d] finish\n", conn->getId(), clientHost.c_str(), clientPort);
#endif
        if (0 == session->m_frame->opcode || 1 == session->m_frame->opcode || 2 == session->m_frame->opcode)
        {
            if (1 == session->m_frame->fin) /* 尾帧 */
            {
                /* 调回数据接收缓冲区空间 */
                if (conn->getBufferSize() > 1024)
                {
                    conn->resizeBuffer(1024);
                }
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
