#pragma once
#include <functional>

#include "fileparse/nlohmann/helper.hpp"
#include "impl/connect_service.h"
#include "impl/data_channel.h"
#include "impl/protocol_adapter.h"
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

/**
 * @brief 业务执行器钩子
 * @param name 任务名称
 * @param bizFunc 业务函数
 */
using BizExecutorHook = std::function<void(const std::string& name, const std::function<void()>& bizFunc)>;

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
    bool subscribeAccessMsg(const BizCode& bizCode, const std::function<void(int64_t seqId, const nlohmann::json& data)>& func);

private:
    std::shared_ptr<StateHandler> m_stateHandler = nullptr; /* 连接状态处理器 */
    std::vector<std::shared_ptr<MsgHandler>> m_msgHandlerList; /* 消息处理器列表 */
};

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
    unsigned int connectTimeout = 0; /* 连接超时(秒), 为0表示系统默认 */
    BizCode authBizCode = BizCode(0); /* 鉴权业务码, 为0表示不需要鉴权 */
    unsigned int authTimeout = 30; /* 鉴权超时(秒), 必须大于0 */
    BizCode heartbeatBizCode = BizCode(0); /* 心跳业务码, 为0表示不需要发送心跳 */
    unsigned int heartbeatInterval = 15; /* 心跳间隔(秒), 必须大于0 */
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
     * @brief 启动模块
     * @param bizExecutor 业务处理线程
     * @param bizExecutorHook 业务处理线程钩子(选填), 为空时直接执行处理函数
     */
    static void start(const threading::ExecutorPtr& bizExecutor, const BizExecutorHook& bizExecutorHook = nullptr);

    /**
     * @brief 设置鉴权数据生成器
     * @param generator 生成器, 返回值: 鉴权数据
     */
    static void setAuthDataGenerator(const std::function<nlohmann::json()>& generator);

    /**
     * @brief 设置鉴权结果回调
     * @param callback 回调, 参数: 鉴权结果, 返回值: true-鉴权成功, false-鉴权失败
     */
    static void setAuthResultCallback(const std::function<bool(const nlohmann::json& data)>& callback);

    /**
     * @brief 设置心跳数据生成器
     * @param generator 生成器, 返回值: 心跳数据(一般为空)
     */
    static void setHeartbeatDataGenerator(const std::function<nlohmann::json()>& generator);

    /**
     * @brief 连接(非阻塞)
     * @param cfg 接入配置
     * @return true-启动中, false-启动失败
     */
    static bool connect(const AccessConfig& cfg);

    /**
     * @brief 断开
     */
    static void disconnect();

    /**
     * @brief 发送消息
     * @param bizCode 业务码
     * @param seqId 序列ID, 若填0则内部自动生成
     * @param data 业务数据
     * @param callback 响应回调
     * @param timeout 响应超时(秒), 为0时表示不需要等待服务端响应数据
     * @return 消息序列ID, -1表示发送失败
     */
    static int64_t sendMsg(const BizCode& bizCode, int64_t seqId, const nlohmann::json& data, const RespCallback& callback,
                           unsigned int timeout = 30);

    /**
     * @brief 获取本端端点
     * @return 本端端点
     */
    static boost::asio::ip::tcp::endpoint getLocalEndpoint();

private:
    /**
     * @brief 订阅状态, 在观察者中自动调用, 外部模块不直接调用
     * @param handler 状态处理器
     * @return true-订阅成功, false-订阅失败
     */
    static bool subscribeState(const std::shared_ptr<StateHandler>& handler);

    /**
     * @brief 取消状态订阅, 在观察者析构中自动调用, 外部模块不直接调用
     * @param handler 状态处理器
     */
    static void unsubscribeState(const std::shared_ptr<StateHandler>& handler);

    /**
     * @brief 订阅消息(服务端主动下发的消息), 在观察者中自动调用, 外部模块不直接调用
     * @param bizCode 业务码
     * @param handler 消息处理器
     * @return true-订阅成功, false-订阅失败
     */
    static bool subscribeMsg(int32_t bizCode, const std::shared_ptr<MsgHandler>& handler);

    /**
     * @brief 取消消息订阅, 在观察者析构中自动调用, 外部模块不直接调用
     * @param handler 消息处理器
     */
    static void unsubscribeMsg(const std::shared_ptr<MsgHandler>& handler);

    /**
     * @brief 响应接收消息
     * @param bizCode 业务码
     * @param seqId 消息序列ID
     * @param data 数据
     */
    static void onReceiveMsg(int32_t bizCode, int64_t seqId, const std::string& data);

    /**
     * @brief 响应连接状态变化
     * @param state 连接状态
     */
    static void onConnectStateChanged(const ConnectState& state);

    /**
     * @brief 响应重试定时器
     */
    static void onRetryTimer();
};
} // namespace nac
