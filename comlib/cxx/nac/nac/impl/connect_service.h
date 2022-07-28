#pragma once
#include <atomic>
#include <mutex>

#include "session_manager.h"
#include "threading/timer/steady_timer.h"

namespace nac
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
     * @param address 服务器地址
     * @param port 服务器端口
     * @param certFile 证书文件(选填), 例如: client.crt
     * @param privateKeyFile 私钥文件(选填), 例如: client.key
     * @param privateKeyFilePwd 私钥文件密码(选填), 例如: qq123456
     * @param connectTimeout 连接超时(秒, 选填), 为0表示系统默认
     * @param authBizCode 鉴权业务码(选填), 为0时表示不需要鉴权
     * @param authTimeout 鉴权响应超时(秒, 选填), 必须大于0
     * @param heartbeatBizCode 心跳业务码(选填), 为0时表示不需要发送心跳
     * @param heartbeatInterval 心跳间隔(秒, 选填), 必须大于0
     * @param heartbeatFixedInterval 心跳固定间隔(秒, 选填), 在该周期内至少发送一次心跳, <=心跳间隔则根据心跳间隔定时发送
     * @return true-连接请求中, false-连接失败
     */
    bool connect(const std::string& address, unsigned int port, const std::string& certFile = "", const std::string& privateKeyFile = "",
                 const std::string& privateKeyFilePwd = "", unsigned int connectTimeout = 0, unsigned int authBizCode = 0,
                 unsigned int authTimeout = 30, unsigned int heartbeatBizCode = 0, unsigned int heartbeatInterval = 15,
                 unsigned int heartbeatFixedInterval = 60);

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
    void onConnectStatusChanged(bool isConnected);

    /**
     * @brief 响应更新最后接收时间
     */
    void onUpdateLastRecvTime();

    /**
     * @brief 响应更新最后发送时间
     */
    void onUpdateLastSendTime();

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
    std::atomic<int64_t> m_lastRecvTime = {0}; /* 最近接收数据时间, epoch至今的毫秒数 */
    std::atomic<int64_t> m_lastSendTime = {0}; /* 最近发送数据时间, epoch至今的毫秒数 */
    std::atomic<int64_t> m_lastSendHeartbeatTime = {0}; /* 最近发送心跳的时间, epoch至今的毫秒数 */
    std::mutex m_mutexLastRecvDateTime;
    std::string m_lastRecvDateTime; /* 最近接收数据日期, 年月日时分秒, 用于日志打印 */
    std::mutex m_mutexLastSendDateTime;
    std::string m_lastSendDateTime; /* 最近发送数据日期, 年月日时分秒, 用于日志打印 */
    std::mutex m_mutexLastSendHeartbeatDateTime;
    std::string m_lastSendHeartbeatDateTime; /* 最近发送心跳日期, 年月日时分秒, 用于日志打印 */
    std::string m_address; /* 服务器地址 */
    unsigned int m_port = 0; /* 服务器端口 */
    std::string m_certFile; /* 证书文件 */
    std::string m_privateKeyFile; /* 私钥文件 */
    std::string m_privateKeyFilePwd; /* 私钥文件密码 */
    unsigned int m_connectTimeout = 0; /* 连接超时(秒) */
    unsigned int m_authBizCode = 0; /* 鉴权业务码 */
    unsigned int m_authTimeout = 30; /* 鉴权响应超时(秒), 必须大于0 */
    unsigned int m_heartbeatBizCode = 0; /* 心跳业务码 */
    unsigned int m_heartbeatInterval = 15; /* 心跳间隔(秒), 必须大于0, 在该周期内未向服务端发送数据则发送心跳 */
    unsigned int m_heartbeatFixedInterval = 60; /* 心跳固定间隔(秒), 在该周期内至少发送一次心跳, <=心跳间隔则根据心跳间隔定时发送 */
    unsigned int m_offlineTime = 61; /* 掉线检测时间(秒), 超过该时间未收到服务端数据表示掉线, 动态计算: (心跳最大间隔 + 1) */
    std::shared_ptr<threading::SteadyTimer> m_timeoutTimer = nullptr; /* 连接超时定时器 */
    std::shared_ptr<threading::SteadyTimer> m_heartbeatTimer = nullptr; /* 心跳定时器 */
    std::shared_ptr<threading::SteadyTimer> m_offlineCheckTimer = nullptr; /* 掉线检查定时器 */
    std::atomic<ConnectState> m_connectState = {ConnectState::idle}; /* 连接状态 */
    std::atomic<DisconnectType> m_disconnectType = {DisconnectType::unknown}; /* 断开连接类型 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};
} // namespace nac
