#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "../tcp/tcp_server.h"
#include "response.h"
#include "session.h"
#include "srv_messager.h"

namespace nsocket
{
namespace ws
{
/**
 * @brief 连接中回调
 * @param wpSession 会话
 * @return 服务端发给客户端的响应
 */
using WS_SRV_CONNECTING_CALLBACK = std::function<std::shared_ptr<Response>(const std::weak_ptr<Session>& wpSession)>;

/**
 * @brief 连接打开回调
 * @param wpSession 会话
 */
using WS_SRV_OPEN_CALLBACK = std::function<void(const std::weak_ptr<Session>& wpSession)>;

/**
 * @brief ping回调
 * @param wpSession 会话
 */
using WS_SRV_PING_CALLBACK = std::function<void(const std::weak_ptr<Session>& wpSession)>;

/**
 * @brief pong回调
 * @param wpSession 会话
 */
using WS_SRV_PONG_CALLBACK = std::function<void(const std::weak_ptr<Session>& wpSession)>;

/**
 * @brief 连接关闭回调
 * @param cid 连接ID
 * @param point 远端端点
 * @param code 错误码
 */
using WS_SRV_CLOSE_CALLBACK =
    std::function<void(uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)>;

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
     * @param reuseAddr 是否允许复用端口(选填), 默认不复用
     * @param bz 数据缓冲区大小(字节, 选填)
     * @param handshakeTimeout 握手超时时间(选填), 单位: 毫秒
     */
    Server(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, bool reuseAddr = false, size_t bz = 4096,
           size_t handshakeTimeout = 3000);

    virtual ~Server();

    /**
     * @brief 设置连接中回调
     * @param cb 连接中回调
     */
    void setConnectingCallback(const WS_SRV_CONNECTING_CALLBACK& cb);

    /**
     * @brief 设置连接打开回调
     * @param cb 连接打开回调
     */
    void setOpenCallback(const WS_SRV_OPEN_CALLBACK& cb);

    /**
     * @brief 设置ping回调
     * @param cb ping回调
     */
    void setPingCallback(const WS_SRV_PING_CALLBACK& cb);

    /**
     * @brief 设置pong回调
     * @param cb pong回调
     */
    void setPongCallback(const WS_SRV_PONG_CALLBACK& cb);

    /**
     * @brief 设置消息接收者
     * @param msger 消息接收者
     */
    void setMessager(const std::shared_ptr<SrvMessager>& msger);

    /**
     * @brief 设置连接关闭回调
     * @param cb 连接关闭回调
     */
    void setCloseCallback(const WS_SRV_CLOSE_CALLBACK& cb);

    /**
     * @brief 运行(非阻塞)
     * @param sslOn 是否开启SSL, true-是, false-否
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt (证书/私钥)文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件, 例如: client.crt
     * @param pkFile 私钥文件, 例如; client.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     * @return true-运行中, false-运行失败(服务对象无效导致)
     */
    bool run(bool sslOn = false, int sslWay = 1, int certFmt = 2, const std::string& certFile = "", const std::string& pkFile = "",
             const std::string& pkPwd = "");

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 是否有效
     * @param errorMsg 错误消息(选填)
     * @return true-有效, false-无效
     */
    bool isValid(std::string* errorMsg = nullptr) const;

    /**
     * @brief 是否运行中
     * @return true-运行中, false-非运行中
     */
    bool isRunning() const;

    /**
     * @brief 获取会话表
     * @return 会话表
     */
    std::unordered_map<uint64_t, std::weak_ptr<Session>> getSessionMap();

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
    void handleConnectionClose(uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code);

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
    std::unordered_map<uint64_t, std::shared_ptr<Session>> m_sessionMap; /* 会话表 */
    WS_SRV_CONNECTING_CALLBACK m_onConnectingCallback; /* 连接中回调 */
    WS_SRV_OPEN_CALLBACK m_onOpenCallback; /* 连接打开回调 */
    WS_SRV_PING_CALLBACK m_onPingCallback; /* ping回调 */
    WS_SRV_PONG_CALLBACK m_onPongCallback; /* pong回调 */
    std::shared_ptr<SrvMessager> m_messager; /* 消息接收者 */
    WS_SRV_CLOSE_CALLBACK m_onCloseCallback; /* 连接关闭回调 */
};
} // namespace ws
} // namespace nsocket
