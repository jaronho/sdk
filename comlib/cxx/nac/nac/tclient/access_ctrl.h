#pragma once
#include <functional>

#include "impl/connect_service.h"
#include "impl/data_channel.h"
#include "impl/protocol_adapter.h"
#include "impl/session_manager.h"
#include "protocol_adapter_custom.h"
#include "threading/timer/steady_timer.h"

/* 网络接入控制(Network Access Control) */
namespace nac
{
namespace tcli
{
/**
 * @brief 响应回调
 * @param ok 是否成功
 * @param data 数据
 */
using RespCallback = std::function<void(bool ok, const std::string& data)>;

/**
 * @brief 业务执行器钩子
 * @param name 任务名称
 * @param bizFunc 业务函数
 */
using BizExecutorHook = std::function<void(const std::string& name, const std::function<void()>& bizFunc)>;

class StateHandler;
class MsgHandler;
class AccessObserver;

/**
 * @brief 接入配置
 */
struct AccessConfig
{
    unsigned int localPort = 0; /* 本地端口 */
    std::string address; /* 服务器地址 */
    unsigned int port = 0; /* 服务器端口 */
    bool sslOn = false; /* 是否通道加密, true-是, false-否(不需要证书/私钥) */
    int sslWay = 1; /* SSL验证, 1-单向(不需要证书/私钥), 2-双向 */
    int certFmt = 2; /* (证书/私钥)文件格式, 1-DER, 2-PEM */
    std::string certFile; /* 证书文件(全路径), 例如: /home/root/client.crt */
    std::string pkFile; /* 私钥文件(全路径), 例如: /home/root/client.key */
    std::string pkPwd; /* 私钥文件密码, 例如: qq123456 */
    int sendBufSize = -1; /* 发送缓冲区大小(字节), <=0表示系统默认 */
    int recvBufSize = -1; /* 接收缓冲区大小(字节), <=0表示系统默认 */
    int enableNagle = -1; /* 是否启用Nagle算法, <0表示系统默认, 0-禁用, 1-启用 */
    unsigned int connectTimeout = 0; /* 连接超时(秒), 为0表示系统默认 */
    int32_t authBizCode = 0; /* 鉴权业务码, 为0表示不需要鉴权 */
    unsigned int authTimeout = 30; /* 鉴权超时(秒), 必须大于0 */
    int32_t heartbeatBizCode = 0; /* 心跳业务码, 为0表示不需要发送心跳 */
    unsigned int heartbeatInterval = 15; /* 心跳间隔(秒), 必须大于0 */
    bool heartbeatFixedSend = false; /* 心跳是否固定间隔发送, 若不固定则在心跳间隔内未收任何服务端数据包时才发送心跳 */
    unsigned int offlineTime = 61; /* 掉线判定时间(秒), 超过该时间未收到服务端数据表示掉线, 必须大于心跳间隔 */
    std::vector<unsigned int> retryInterval = {1}; /* 重试间隔(秒), 示例: {1, 1, 1, 1, 1, 2, 4, 8, 10} */
};

/**
 * @brief 接入控制(对外接口)
 */
class AccessCtrl final
{
    friend AccessObserver;

public:
    /**
     * @brief 启动模块(注意: 需要最先调用)
     * @param adapter 协议适配器
     * @param bizExecutor 业务处理线程
     * @param bizExecutorHook 业务处理线程钩子(选填), 为空时直接执行处理函数
     */
    void start(const std::shared_ptr<ProtocolAdapter>& adapter, const threading::ExecutorPtr& bizExecutor,
               const BizExecutorHook& bizExecutorHook = nullptr);

    /**
     * @brief 设置数据包版本不匹配回调
     * @param callback 回调, 参数: localVersion-本地版本号, pktVersion-数据包版本号
     */
    void setPacketVersionMismatchCallback(const PACKET_VERSION_MISMATCH_CALLBACK& callback);

    /**
     * @brief 设置数据包长度异常回调
     * @param callback 回调, 参数: maxLength-最大长度, pktLength-数据包长度
     */
    void setPacketLengthAbnormalCallback(const PACKET_LENGTH_ABNORMAL_CALLBACK& callback);

    /**
     * @brief 设置鉴权数据生成器
     * @param generator 生成器, 返回值: 鉴权数据
     */
    void setAuthDataGenerator(const std::function<std::string()>& generator);

    /**
     * @brief 设置鉴权结果回调
     * @param callback 回调, 参数: data-鉴权结果, 返回值: true-鉴权成功, false-鉴权失败
     */
    void setAuthResultCallback(const std::function<bool(const std::string& data)>& callback);

    /**
     * @brief 设置心跳数据生成器
     * @param generator 生成器, 返回值: 心跳数据(一般为空)
     */
    void setHeartbeatDataGenerator(const std::function<std::string()>& generator);

    /**
     * @brief 连接
     * @param cfg 接入配置
     * @return true-启动中, false-启动失败
     */
    bool connect(const AccessConfig& cfg);

    /**
     * @brief 断开
     */
    void disconnect();

    /**
     * @brief 设置参数
     * @param heartbeatInterval 心跳间隔(秒), 必须大于0
     * @param heartbeatFixedSend 心跳是否根据间隔固定发送, 若不固定则在心跳间隔内未收任何服务端数据包时才发送心跳
     * @param offlineTime 掉线判定时间(秒), 超过该时间未收到服务端数据表示掉线, 必须大于心跳间隔
     * @return true-成功, false-失败
     */
    bool setParam(unsigned int heartbeatInterval, bool heartbeatFixedSend, unsigned int offlineTime);

    /**
     * @brief 发送消息
     * @param bizCode 业务码
     * @param seqId 序列ID, 若填0则内部自动生成
     * @param data 业务数据
     * @param callback 响应回调
     * @param timeout 响应超时(秒), 为0时表示不需要等待服务端响应数据
     * @return 消息序列ID, -1表示发送失败
     */
    int64_t sendMsg(int32_t bizCode, int64_t seqId, const std::string& data, const RespCallback& callback = nullptr,
                    unsigned int timeout = 5);

    /**
     * @brief 获取本端端点
     * @return 本端端点
     */
    boost::asio::ip::tcp::endpoint getLocalEndpoint();

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
    bool subscribeMsg(int32_t bizCode, const std::shared_ptr<MsgHandler>& handler);

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
    void onReceiveMsg(int32_t bizCode, int64_t seqId, const std::string& data);

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
    std::shared_ptr<DataChannel> m_dataChannel = nullptr; /* 数据通道 */
    std::shared_ptr<ProtocolAdapter> m_protocolAdapter = nullptr; /* 协议适配器 */
    std::shared_ptr<SessionManager> m_sessionManager = nullptr; /* 会话管理器 */
    std::shared_ptr<ConnectService> m_connectService = nullptr; /* 连接服务 */
    threading::ExecutorPtr m_bizExecutor = nullptr; /* 业务处理线程 */
    BizExecutorHook m_bizExecutorHook = nullptr; /* 业务处理线程钩子 */
    std::mutex m_mutexStateHandlerList;
    std::list<std::weak_ptr<StateHandler>> m_stateHandlerList; /* 状态处理器列表 */
    std::mutex m_mutexMsgHandlerMap;
    std::map<int32_t, std::list<std::weak_ptr<MsgHandler>>> m_msgHandlerMap; /* 消息处理器列表 */
    std::mutex m_mutexCfg;
    AccessConfig m_cfg; /* 接入配置 */
    std::mutex m_mutexRetryTimer;
    threading::SteadyTimerPtr m_retryTimer = nullptr; /* 重试(自动重连)定时器 */
    std::atomic<ConnectState> m_connectState = {ConnectState::idle}; /* 连接状态 */
};

/**
 * @brief 观察者(基类), 如果要订阅接入的相关状态和消息, 则只能通过观察者
 */
class AccessObserver
{
public:
    /**
     * @brief 构造函数
     * @param accessCtrl 接入控制
     */
    AccessObserver(const std::shared_ptr<AccessCtrl>& accessCtrl);
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
    bool subscribeAccessMsg(int32_t bizCode, const std::function<void(int64_t seqId, const std::string& data)>& func);

private:
    std::weak_ptr<AccessCtrl> m_wpAccessCtrl; /* 接入控制 */
    std::mutex m_mutexHandler;
    std::shared_ptr<StateHandler> m_stateHandler = nullptr; /* 连接状态处理器 */
    std::vector<std::shared_ptr<MsgHandler>> m_msgHandlerList; /* 消息处理器列表 */
};
} // namespace tcli
} // namespace nac
