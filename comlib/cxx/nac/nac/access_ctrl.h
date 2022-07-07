#pragma once
#include <list>
#include <map>

#include "fileparse/nlohmann/helper.hpp"
#include "impl/connect_service.h"
#include "impl/data_channel.h"
#include "impl/protocol_adapter.hpp"
#include "impl/session_manager.h"
#include "threading/timer/steady_timer.h"

/* 网络接入控制(Network Access Control) */
namespace nac
{
/**
 * @brief 业务码(需要外部定义)
 */
enum class BizCode;

/**
 * @brief 响应回调
 * @param ok 是否成功
 * @param data 数据
 */
using RespCallback = std::function<void(bool ok, const nlohmann::json& data)>;

class StateHandler;
class MsgHandler;

/**
 * @brief 观察者(基类), 如果要订阅接入的相关状态和消息, 则只能通过观察者
 */
class AccessObserver
{
public:
    ~AccessObserver();

protected:
    /**
     * @brief 订阅网络连接状态
     * @param func 连接状态处理函数
     * @return true-订阅成功, false-订阅失败
     */
    bool subscribeAccessState(const std::function<void(const ConnectState& state)>& func);

    /**
     * @brief 订阅网络接入消息
     * @param bizCode 业务码
     * @param func 消息处理函数
     * @return true-订阅成功, false-订阅失败
     */
    bool subscribeAccessMsg(const BizCode& bizCode, const std::function<void(unsigned long long seqId, const nlohmann::json& data)>& func);

private:
    std::shared_ptr<StateHandler> m_stateHandler = nullptr; /* 连接状态处理器 */
    std::vector<std::shared_ptr<MsgHandler>> m_msgHandlerList; /* 消息处理器列表 */
};

/**
 * @brief 接入配置
 */
struct AccessConfig
{
    std::string address; /* 服务器地址 */
    unsigned int port = 0; /* 服务器端口 */
    std::string certFile; /* 证书文件(全路径), 例如: /home/root/client.crt */
    std::string privateKeyFile; /* 私钥文件(全路径), 例如: /home/root/client.key */
    std::string privateKeyFilePwd; /* 私钥文件密码, 例如: qq123456 */
    unsigned int connectTimeout = 0; /* 连接超时(秒), 为0表示系统默认 */
    BizCode authBizCode = BizCode(0); /* 鉴权业务码, 为0表示不需要鉴权 */
    unsigned int authTimeout = 30; /* 鉴权超时(秒), 必须大于0 */
    BizCode heartbeatBizCode = BizCode(0); /* 心跳业务码, 为0表示不需要发送心跳 */
    unsigned int heartbeatInterval = 15; /* 心跳间隔(秒), 必须大于0 */
    unsigned int heartbeatFixedInterval = 0; /* 心跳固定间隔(秒), 在该周期内至少发送一次心跳, <=心跳间隔则根据心跳间隔定时发送 */
    std::vector<unsigned int> retryInterval = {1}; /* 重试间隔(秒), 示例: {1, 1, 1, 1, 1, 2, 4, 8, 10} */
};

/**
 * @brief 接入控制(对外接口), 需要依赖定时器`Timer`执行`runOnce`
 */
class AccessCtrl final
{
    friend AccessObserver;

private:
    AccessCtrl();

public:
    static AccessCtrl& getInstance();

    /**
     * @brief 设置鉴权数据生成器
     * @param generator 生成器, 返回值: 鉴权数据
     */
    void setAuthDataGenerator(const std::function<nlohmann::json()>& generator);

    /**
     * @brief 设置鉴权结果回调
     * @param callback 回调, 参数: 鉴权结果, 返回值: true-鉴权成功, false-鉴权失败
     */
    void setAuthResultCallback(const std::function<bool(const nlohmann::json& data)>& callback);

    /**
     * @brief 设置心跳数据生成器
     * @param generator 生成器, 返回值: 心跳数据(一般为空)
     */
    void setHeartbeatDataGenerator(const std::function<nlohmann::json()>& generator);

    /**
     * @brief 启动(非阻塞)
     * @param cfg 接入配置
     * @return true-启动中, false-启动失败
     */
    bool start(const AccessConfig& cfg);

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 发送消息
     * @param bizCode 业务码
     * @param seqId 序列ID, 若填0则内部自动生成
     * @param data 业务数据
     * @param callback 响应回调
     * @param timeout 响应超时(秒), 为0时表示不需要响应
     * @return 消息序列ID, -1表示发送失败
     */
    int64_t sendMsg(const BizCode& bizCode, unsigned long long seqId, const nlohmann::json& data, const RespCallback& callback,
                    unsigned int timeout = 30);

private:
    /**
     * @brief 订阅状态, 在观察者中自动调用, 外部模块不直接调用
     * @param handler 状态处理器
     * @return true-订阅成功, false-订阅失败
     */
    bool subscribeState(const std::shared_ptr<StateHandler>& handler);

    /**
     * @brief 取消状态订阅, 在观察者析构中自动调用, 外部模块不直接调用
     * @param handler 状态处理器
     */
    void unsubscribeState(const std::shared_ptr<StateHandler>& handler);

    /**
     * @brief 订阅消息(服务端主动下发的消息), 在观察者中自动调用, 外部模块不直接调用
     * @param bizCode 业务码
     * @param handler 消息处理器
     * @return true-订阅成功, false-订阅失败
     */
    bool subscribeMsg(const BizCode& bizCode, const std::shared_ptr<MsgHandler>& handler);

    /**
     * @brief 取消消息订阅, 在观察者析构中自动调用, 外部模块不直接调用
     * @param handler 消息处理器
     */
    void unsubscribeMsg(const std::shared_ptr<MsgHandler>& handler);

    /**
     * @brief 响应接收消息
     * @param bizCode 业务码
     * @param seqId 消息序列ID
     * @param data 数据
     */
    void onReceiveMsg(unsigned int bizCode, int64_t seqId, const std::string& data);

    /**
     * @brief 响应连接状态变化
     * @param state 连接状态
     */
    void onConnectStateChanged(const ConnectState& state);

    /**
     * @brief 响应重试定时器
     */
    void onRetryTimer();

private:
    std::shared_ptr<DataChannel> m_dataChannel; /* 数据通道 */
    std::shared_ptr<ProtocolAdapter> m_protocolAdapter; /* 协议适配器 */
    std::shared_ptr<SessionManager> m_sessionManager; /* 会话管理器 */
    std::shared_ptr<ConnectService> m_connectService; /* 连接服务 */
    threading::SteadyTimerPtr m_retryTimer = nullptr; /* 重试(自动重连)定时器 */
    std::mutex m_mutexStateHandlerList;
    std::list<std::weak_ptr<StateHandler>> m_stateHandlerList; /* 状态处理器列表 */
    std::mutex m_mutexMsgHandlerMap;
    std::map<BizCode, std::list<std::weak_ptr<MsgHandler>>> m_msgHandlerMap; /* 消息处理器列表 */
    AccessConfig m_cfg; /* 接入配置 */
};
} // namespace nac
