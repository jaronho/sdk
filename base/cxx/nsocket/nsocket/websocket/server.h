#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "../tcp/tcp_server.h"
#include "messager.h"
#include "request.h"
#include "response.h"
#include "session.h"

namespace nsocket
{
namespace ws
{
/**
 * @brief 连接中回调
 * @param wpSession 会话
 * @return 服务端发给客户端的响应
 */
using WS_CONNECTING_CALLBACK = std::function<std::shared_ptr<Response>(const std::weak_ptr<Session>& wpSession)>;

/**
 * @brief 连接打开回调
 * @param wpSession 会话
 */
using WS_OPEN_CALLBACK = std::function<void(const std::weak_ptr<Session>& wpSession)>;

/**
 * @brief ping回调
 * @param wpSession 会话
 */
using WS_PING_CALLBACK = std::function<void(const std::weak_ptr<Session>& wpSession)>;

/**
 * @brief pong回调
 * @param wpSession 会话
 */
using WS_PONG_CALLBACK = std::function<void(const std::weak_ptr<Session>& wpSession)>;

/**
 * @brief 连接关闭回调
 * @param cid 连接ID
 */
using WS_CLOSE_CALLBACK = std::function<void(int64_t cid)>;

/**
 * @brief WebSocket服务器
 */
class Server final : public std::enable_shared_from_this<Server>
{
public:
    /**
     * @brief 构造函数
     * @param name 服务器名称
     * @param threadCount 线程个数
     * @param host 主机地址
     * @param port 端口
     */
    Server(const std::string& name, size_t threadCount, const std::string& host, unsigned int port);

    /**
     * @brief 设置连接中回调
     * @param cb 连接中回调
     */
    void setConnectingCallback(const WS_CONNECTING_CALLBACK& cb);

    /**
     * @brief 设置连接打开回调
     * @param cb 连接打开回调
     */
    void setOpenCallback(const WS_OPEN_CALLBACK& cb);

    /**
     * @brief 设置ping回调
     * @param cb ping回调
     */
    void setPingCallback(const WS_PING_CALLBACK& cb);

    /**
     * @brief 设置pong回调
     * @param cb pong回调
     */
    void setPongCallback(const WS_PONG_CALLBACK& cb);

    /**
     * @brief 设置消息接收者
     * @param msger 消息接收者
     */
    void setMessager(const std::shared_ptr<Messager>& msger);

    /**
     * @brief 设置连接关闭回调
     * @param cb 连接关闭回调
     */
    void setCloseCallback(const WS_CLOSE_CALLBACK& cb);

    /**
     * @brief 运行
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    void run(const std::shared_ptr<boost::asio::ssl::context>& sslContext = nullptr);
#else
    void run();
#endif

    /**
     * @brief 获取会话表
     * @return 会话表
     */
    std::unordered_map<int64_t, std::weak_ptr<Session>> getSessionMap();

private:
    /**
     * @brief 处理新连接
     */
    void handleNewConnection(const std::weak_ptr<TcpConnection>& wpConn);

    /**
     * @brief 处理连接数据
     */
    void handleConnectionData(const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data);

    /**
     * @brief 处理连接断开
     */
    void handleConnectionClose(int64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code);

    /**
     * @brief 处理请求
     */
    void handleRequest(const std::shared_ptr<Session>& session);

    /**
     * @brief 处理帧头
     */
    void handleFrameHead(const std::shared_ptr<Session>& session);

    /**
     * @brief 处理帧负载
     */
    void handleFramePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen);

    /**
     * @brief 处理帧结束
     */
    void handleFrameFinish(const std::shared_ptr<Session>& session);

private:
    std::shared_ptr<TcpServer> m_tcpServer; /* TCP服务器 */
    std::mutex m_mutex;
    std::unordered_map<int64_t, std::shared_ptr<Session>> m_sessionMap; /* 会话表 */
    WS_CONNECTING_CALLBACK m_onConnectingCallback; /* 连接中回调 */
    WS_OPEN_CALLBACK m_onOpenCallback; /* 连接打开回调 */
    WS_PING_CALLBACK m_onPingCallback; /* ping回调 */
    WS_PONG_CALLBACK m_onPongCallback; /* pong回调 */
    std::shared_ptr<Messager> m_messager; /* 消息接收者 */
    WS_CLOSE_CALLBACK m_onCloseCallback; /* 连接关闭回调 */
};
} // namespace ws
} // namespace nsocket