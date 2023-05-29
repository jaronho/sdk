#pragma once
#include <atomic>

#include "session_manager.h"
#include "threading/timer/steady_timer.h"

namespace nac
{
namespace tcli
{
/**
 * @brief 连接状态
 */
enum class ConnectState
{
    idle = 0, /* 未连接(或主动断开) */
    connecting = 1, /* 连接中 */
    connected = 2, /* 已连接 */
    disconnected = 3 /* 连接断开(异常被动断开) */
};

/**
 * @brief 接入服务
 */
class ConnectService final : public std::enable_shared_from_this<ConnectService>
{
public:
    /**
     * @brief 设置数据通道
     * @param dataChannel 数据层
     */
    void setDataChannel(const std::shared_ptr<DataChannel>& dataChannel);

    /**
     * @brief 设置会话管理器
     * @param sessionManager 会话层管理器
     */
    void setSessionManager(const std::shared_ptr<SessionManager>& sessionManager);

    /**
     * @brief 设置连接回调
     * @param callback 回调, 参数: state-连接状态
     */
    void setConnectCallback(const std::function<void(const ConnectState& state)>& callback);

    /**
     * @brief 设置鉴权数据生成器
     * @param generator 生成器
     */
    void setAuthDataGenerator(const std::function<std::string()>& generator);

    /**
     * @brief 设置鉴权结果回调
     * @param callback 回调, 参数: data-服务端鉴权结果, 返回值: true-鉴权成功, false-鉴权失败
     */
    void setAuthResultCallback(const std::function<bool(const std::string& data)>& callback);

    /**
     * @brief 设置心跳数据生成器
     * @param generator 生成器
     */
    void setHeartbeatDataGenerator(const std::function<std::string()>& generator);

    /**
     * @brief 连接(连接成功后鉴权如果需要的话)
     * @param localPort 本地端口, 0-使用自动随机分配的端口
     * @param address 服务器地址
     * @param port 服务器端口
     * @param sslOn 是否开启SSL验证
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt (证书/私钥)文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件(选填), 例如: client.crt
     * @param pkFile 私钥文件(选填), 例如: client.key
     * @param pkPwd 私钥文件密码(选填), 例如: qq123456
     * @param connectTimeout 连接超时(秒, 选填), 为0表示系统默认
     * @param authBizCode 鉴权业务码(选填), 为0时表示不需要鉴权
     * @param authTimeout 鉴权响应超时(秒, 选填), 必须大于0
     * @param heartbeatBizCode 心跳业务码(选填), 为0时表示不需要发送心跳
     * @param heartbeatInterval 心跳间隔(秒, 选填), 必须大于0
     * @param offlineTime 掉线判定时间(秒), 超过该时间未收到服务端数据表示掉线, 必须大于心跳间隔
     * @return true-连接请求中, false-连接失败
     */
    bool connect(unsigned short localPort, const std::string& address, unsigned int port, bool sslOn = false, int sslWay = 1,
                 int certFmt = 2, const std::string& certFile = "", const std::string& pkFile = "", const std::string& pkPwd = "",
                 unsigned int connectTimeout = 0, int32_t authBizCode = 0, unsigned int authTimeout = 30, int32_t heartbeatBizCode = 0,
                 unsigned int heartbeatInterval = 15, unsigned int offlineTime = 61);

    /**
     * @brief 重连, 注意: 主动断开连接再调重连则无效
     * @return true-连接请求中, false-连接失败
     */
    bool reconnect();

    /**
     * @brief 主动断开连接
     */
    void disconnect();

    /**
     * @brief 获取当前连接状态
     * @return 连接状态
     */
    ConnectState getConnectState() const;

private:
    /**
     * @brief 断开连接类型
     */
    enum class DisconnectType
    {
        unknown = 0, /*未知 */
        external_call, /* 外部主动调用断开 */
        connect_timeout, /* 连接超时 */
        auth_fail, /* 鉴权失败 */
        offline /* 掉线 */
    };

private:
    /**
     * @brief 释放连接
     */
    void releaseConnection(const DisconnectType& type);

    /**
     * @brief 响应连接状态变化
     */
    void onConnectStatusChanged(const boost::system::error_code& code);

    /**
     * @brief 响应更新最后接收时间
     */
    void onUpdateLastRecvTime(std::chrono::steady_clock::time_point tp);

    /**
     * @brief 响应更新最后发送时间
     */
    void onUpdateLastSendTime(std::chrono::steady_clock::time_point tp);

    /**
     * @brief 发送鉴权消息
     */
    void sendAuthMsg();

    /**
     * @brief 响应鉴权结果
     */
    void onAuthResult(bool ok, const std::string& data);

    /**
     * @brief 开始连接超时定时器
     */
    void startTimetoutTimer();

    /**
     * @brief 响应连接超时定时器
     */
    void onTimetoutTimer();

    /**
     * @brief 停止连接超时定时器
     */
    void stopTimeoutTimer();

    /**
     * @brief 开始心跳定时器
     */
    void startHeartbeatTimer();

    /**
     * @brief 响应心跳定时器
     */
    void onHeartbeatTimer();

    /**
     * @brief 发送心跳消息
     */
    void sendHeartbeatMsg();

    /**
     * @brief 开始掉线检测定时器
     */
    void startOfflineCheckTimer();

    /**
     * @brief 响应掉线检测定时器
     */
    void onOfflineCheckTimer();

    /**
     * @brief 停止所有定时器
     */
    void stopAllTimer();

    /**
     * @brief 更连接状态
     */
    void updateConnectState(const ConnectState& state);

private:
    std::vector<threading::ScopedSignalConnection> m_connections; /* 信号连接 */
    std::weak_ptr<DataChannel> m_wpDataChannel; /* 数据通道 */
    std::weak_ptr<SessionManager> m_wpSessionManager; /* 会话管理器 */
    std::function<void(const ConnectState& state)> m_connectCb = nullptr; /* 连接回调 */
    std::function<std::string()> m_authDataGenerator = nullptr; /* 鉴权数据生成器 */
    std::function<bool(const std::string& data)> m_authResultCb = nullptr; /* 鉴权结果回调 */
    std::function<std::string()> m_heartbeatDataGenerator = nullptr; /* 心跳数据生成器 */
    std::atomic<std::chrono::steady_clock::time_point> m_lastRecvTime = {std::chrono::steady_clock::now()}; /* 最近接收时间 */
    std::atomic<std::chrono::steady_clock::time_point> m_lastSendTime = {std::chrono::steady_clock::now()}; /* 最近发送时间 */
    std::atomic<std::chrono::steady_clock::time_point> m_lastHeartbeatTime = {std::chrono::steady_clock::now()}; /* 最近心跳时间 */
    unsigned int m_localPort = 0; /* 本地端口 */
    std::string m_address; /* 服务器地址 */
    unsigned int m_port = 0; /* 服务器端口 */
    bool m_enableTls = false; /* 是否进行通道加密 */
    int m_sslWay = 1; /* SSL验证, 1-单向, 2-双向 */
    int m_certFmt = 2; /* (证书/私钥)文件格式, 1-DER, 2-PEM */
    std::string m_certFile; /* 证书文件 */
    std::string m_privateKeyFile; /* 私钥文件 */
    std::string m_privateKeyFilePwd; /* 私钥文件密码 */
    unsigned int m_connectTimeout = 0; /* 连接超时(秒) */
    int32_t m_authBizCode = 0; /* 鉴权业务码 */
    unsigned int m_authTimeout = 30; /* 鉴权响应超时(秒), 必须大于0 */
    int32_t m_heartbeatBizCode = 0; /* 心跳业务码 */
    unsigned int m_heartbeatInterval = 15; /* 心跳间隔(秒), 必须大于0, 在该周期内未向服务端发送数据则发送心跳 */
    unsigned int m_offlineTime = 61; /* 掉线判定时间(秒), 超过该时间未收到服务端数据表示掉线 */
    std::shared_ptr<threading::SteadyTimer> m_timeoutTimer = nullptr; /* 连接超时定时器 */
    std::shared_ptr<threading::SteadyTimer> m_heartbeatTimer = nullptr; /* 心跳定时器 */
    std::shared_ptr<threading::SteadyTimer> m_offlineCheckTimer = nullptr; /* 掉线检查定时器 */
    std::atomic<ConnectState> m_connectState = {ConnectState::idle}; /* 连接状态 */
    std::atomic<DisconnectType> m_disconnectType = {DisconnectType::unknown}; /* 断开连接类型 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};
} // namespace tcli
} // namespace nac
