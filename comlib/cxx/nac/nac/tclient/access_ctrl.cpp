#include "access_ctrl.h"

#include <list>
#include <map>
#include <stdexcept>

namespace nac
{
namespace tcli
{
/**
 * @brief 状态处理器(内部使用)
 */
class StateHandler
{
public:
    StateHandler(const std::function<void(const ConnectState& state)>& func) : m_func(func) {}

    void operator()(const ConnectState& state)
    {
        if (m_func)
        {
            m_func(state);
        }
    }

private:
    std::function<void(const ConnectState& state)> m_func = nullptr; /* 状态处理函数 */
};

/**
 * @brief 消息处理器(内部使用)
 */
class MsgHandler
{
public:
    MsgHandler(const std::function<void(int64_t seqId, const std::string& data)>& func) : m_func(func) {}

    void operator()(int64_t seqId, const std::string& data)
    {
        if (m_func)
        {
            m_func(seqId, data);
        }
    }

private:
    std::function<void(int64_t seqId, const std::string& data)> m_func = nullptr; /* 消息处理函数 */
};

AccessObserver::~AccessObserver()
{
    if (m_stateHandler)
    {
        AccessCtrl::unsubscribeState(m_stateHandler);
        m_stateHandler.reset();
    }
    for (auto& handler : m_msgHandlerList)
    {
        AccessCtrl::unsubscribeMsg(handler);
        handler.reset();
    }
    m_msgHandlerList.clear();
}

bool AccessObserver::subscribeAccessState(const std::function<void(const ConnectState& state)>& func)
{
    if (func)
    {
        auto handler = std::make_shared<StateHandler>(func);
        if (AccessCtrl::subscribeState(handler))
        {
            m_stateHandler = handler;
            return true;
        }
    }
    return false;
}

bool AccessObserver::subscribeAccessMsg(int32_t bizCode, const std::function<void(int64_t seqId, const std::string& data)>& func)
{
    if (func)
    {
        auto handler = std::make_shared<MsgHandler>(func);
        if (AccessCtrl::subscribeMsg(bizCode, handler))
        {
            m_msgHandlerList.emplace_back(handler);
            return true;
        }
    }
    return false;
}

static std::shared_ptr<DataChannel> s_dataChannel = nullptr; /* 数据通道 */
static std::shared_ptr<ProtocolAdapter> s_protocolAdapter = nullptr; /* 协议适配器 */
static std::shared_ptr<SessionManager> s_sessionManager = nullptr; /* 会话管理器 */
static std::shared_ptr<ConnectService> s_connectService = nullptr; /* 连接服务 */
static threading::ExecutorPtr s_bizExecutor = nullptr; /* 业务处理线程 */
static BizExecutorHook s_bizExecutorHook = nullptr; /* 业务处理线程钩子 */
static std::mutex s_mutexStateHandlerList;
static std::list<std::weak_ptr<StateHandler>> s_stateHandlerList; /* 状态处理器列表 */
static std::mutex s_mutexMsgHandlerMap;
static std::map<int32_t, std::list<std::weak_ptr<MsgHandler>>> s_msgHandlerMap; /* 消息处理器列表 */
static std::mutex s_mutexCfg;
static AccessConfig s_cfg; /* 接入配置 */
static std::mutex s_mutexRetryTimer;
static threading::SteadyTimerPtr s_retryTimer = nullptr; /* 重试(自动重连)定时器 */

void AccessCtrl::start(const std::shared_ptr<ProtocolAdapter>& adapter, const threading::ExecutorPtr& bizExecutor,
                       const BizExecutorHook& bizExecutorHook)
{
    if (!adapter)
    {
        throw std::logic_error("arg adapter must not be empty");
    }
    s_dataChannel = std::make_shared<DataChannel>();
    s_protocolAdapter = adapter;
    s_protocolAdapter->setDataChannel(s_dataChannel);
    s_sessionManager = std::make_shared<SessionManager>();
    s_sessionManager->setDataChannel(s_dataChannel);
    s_sessionManager->setProtocolAdapter(s_protocolAdapter);
    s_sessionManager->setMsgReceiver([&](int32_t bizCode, int64_t seqId, const std::string& data) { onReceiveMsg(bizCode, seqId, data); });
    s_connectService = std::make_shared<ConnectService>();
    s_connectService->setDataChannel(s_dataChannel);
    s_connectService->setSessionManager(s_sessionManager);
    s_connectService->setConnectCallback([&](const ConnectState& state) { onConnectStateChanged(state); });
    s_bizExecutor = bizExecutor;
    s_bizExecutorHook = bizExecutorHook;
}

void AccessCtrl::setPacketVersionMismatchCallback(const std::function<bool(int32_t localVersion, int32_t pktVersion)>& callback)
{
    if (!s_protocolAdapter)
    {
        return;
    }
    s_protocolAdapter->setPacketVersionMismatchCallback([&, callback](int32_t localVersion, int32_t pktVersion) {
        if (!s_bizExecutor)
        {
            return;
        }
        auto name = "nac.tcli.api.pkt.version_mismatch";
        s_bizExecutor->post(name, [&, name, localVersion, pktVersion, callback]() {
            if (callback)
            {
                bool ret = true;
                if (s_bizExecutorHook)
                {
                    s_bizExecutorHook(name, [localVersion, pktVersion, callback, &ret]() { ret = callback(localVersion, pktVersion); });
                }
                else
                {
                    ret = callback(localVersion, pktVersion);
                }
                if (!ret) /* 停止重连 */
                {
                    std::lock_guard<std::mutex> locker(s_mutexRetryTimer);
                    if (s_retryTimer)
                    {
                        s_retryTimer->stop();
                        s_retryTimer.reset();
                    }
                }
            }
        });
    });
}

void AccessCtrl::setPacketLengthAbnormalCallback(const std::function<bool(int32_t maxLength, int32_t pktLength)>& callback)
{
    if (!s_protocolAdapter)
    {
        return;
    }
    s_protocolAdapter->setPacketLengthAbnormalCallback([&, callback](int32_t maxLength, int32_t pktLength) {
        if (!s_bizExecutor)
        {
            return;
        }
        auto name = "nac.tcli.api.pkt.length_abnormal";
        s_bizExecutor->post(name, [&, name, maxLength, pktLength, callback]() {
            if (callback)
            {
                bool ret = true;
                if (s_bizExecutorHook)
                {
                    s_bizExecutorHook(name, [maxLength, pktLength, callback, &ret]() { ret = callback(maxLength, pktLength); });
                }
                else
                {
                    ret = callback(maxLength, pktLength);
                }
                if (!ret) /* 停止重连 */
                {
                    std::lock_guard<std::mutex> locker(s_mutexRetryTimer);
                    if (s_retryTimer)
                    {
                        s_retryTimer->stop();
                        s_retryTimer.reset();
                    }
                }
            }
        });
    });
}

void AccessCtrl::setAuthDataGenerator(const std::function<std::string()>& generator)
{
    if (!s_connectService)
    {
        return;
    }
    s_connectService->setAuthDataGenerator([generator]() {
        if (generator)
        {
            return generator();
        }
        return std::string();
    });
}

void AccessCtrl::setAuthResultCallback(const std::function<bool(const std::string& data)>& callback)
{
    if (!s_connectService)
    {
        return;
    }
    s_connectService->setAuthResultCallback([callback](const std::string& data) {
        if (callback)
        {
            return callback(data);
        }
        return true;
    });
}

void AccessCtrl::setHeartbeatDataGenerator(const std::function<std::string()>& generator)
{
    if (!s_connectService)
    {
        return;
    }
    s_connectService->setHeartbeatDataGenerator([generator]() {
        if (generator)
        {
            return generator();
        }
        return std::string();
    });
}

bool AccessCtrl::connect(const AccessConfig& cfg)
{
    if (!s_connectService)
    {
        return false;
    }
    {
        std::lock_guard<std::mutex> locker(s_mutexCfg);
        s_cfg = cfg;
    }
    /* 创建重试定时器 */
    if (cfg.retryInterval.size() > 0)
    {
        std::lock_guard<std::mutex> locker(s_mutexRetryTimer);
        if (s_retryTimer)
        {
            s_retryTimer->setDelay(std::chrono::seconds(cfg.retryInterval[0]));
        }
        else
        {
            s_retryTimer = threading::SteadyTimer::onceTimer(
                "nac.tcli.connect.retry", std::chrono::seconds(cfg.retryInterval[0]),
                [&](const std::chrono::steady_clock::time_point& tp) { onRetryTimer(); }, s_dataChannel->getPktExecutor().lock());
        }
    }
    /* 首次连接 */
    return s_connectService->connect(cfg.localPort, cfg.address, cfg.port, cfg.sslOn, cfg.sslWay, cfg.certFmt, cfg.certFile, cfg.pkFile,
                                     cfg.pkPwd, cfg.connectTimeout, (int32_t)cfg.authBizCode, cfg.authTimeout,
                                     (int32_t)cfg.heartbeatBizCode, cfg.heartbeatInterval, cfg.offlineTime);
}

void AccessCtrl::disconnect()
{
    if (!s_connectService)
    {
        return;
    }
    {
        std::lock_guard<std::mutex> locker(s_mutexRetryTimer);
        if (s_retryTimer)
        {
            s_retryTimer->stop();
            s_retryTimer.reset();
        }
    }
    s_connectService->disconnect();
}

int64_t AccessCtrl::sendMsg(int32_t bizCode, int64_t seqId, const std::string& data, const RespCallback& callback, unsigned int timeout)
{
    if (!s_sessionManager)
    {
        return -1;
    }
    AccessConfig cfg;
    {
        std::lock_guard<std::mutex> locker(s_mutexCfg);
        cfg = s_cfg;
    }
    if (bizCode == cfg.authBizCode || bizCode == cfg.heartbeatBizCode) /* 鉴权和心跳内部处理 */
    {
        return -1;
    }
    return s_sessionManager->sendMsg(
        bizCode, seqId, data, timeout, [&, callback](bool sendOk, int32_t bizCode, int64_t seqId, const std::string& data) {
            if (!s_bizExecutor)
            {
                return;
            }
            auto name = "nac.tcli.api.resp|" + std::to_string(sendOk) + "|" + std::to_string(bizCode) + "|" + std::to_string(seqId);
            s_bizExecutor->post(name, [&, name, sendOk, bizCode, seqId, data, callback]() {
                if (callback)
                {
                    if (s_bizExecutorHook)
                    {
                        s_bizExecutorHook(name, [sendOk, data, callback]() { callback(sendOk, data); });
                    }
                    else
                    {
                        callback(sendOk, data);
                    }
                }
            });
        });
}

boost::asio::ip::tcp::endpoint AccessCtrl::getLocalEndpoint()
{
    if (!s_dataChannel)
    {
        return boost::asio::ip::tcp::endpoint();
    }
    return s_dataChannel->getLocalEndpoint();
}

bool AccessCtrl::subscribeState(const std::shared_ptr<StateHandler>& handler)
{
    if (handler)
    {
        std::lock_guard<std::mutex> locker(s_mutexStateHandlerList);
        for (const auto& wpHandler : s_stateHandlerList)
        {
            if (wpHandler.lock() == handler)
            {
                return false;
            }
        }
        s_stateHandlerList.emplace_back(handler);
        return true;
    }
    return false;
}

void AccessCtrl::unsubscribeState(const std::shared_ptr<StateHandler>& handler)
{
    if (handler)
    {
        std::lock_guard<std::mutex> locker(s_mutexMsgHandlerMap);
        s_stateHandlerList.remove_if([handler](const std::weak_ptr<StateHandler>& wpHandler) { return (wpHandler.lock() == handler); });
    }
}

bool AccessCtrl::subscribeMsg(int32_t bizCode, const std::shared_ptr<MsgHandler>& handler)
{
    AccessConfig cfg;
    {
        std::lock_guard<std::mutex> locker(s_mutexCfg);
        cfg = s_cfg;
    }
    if (bizCode == cfg.authBizCode || bizCode == cfg.heartbeatBizCode) /* 鉴权和心跳内部处理 */
    {
        return false;
    }
    if (handler)
    {
        std::lock_guard<std::mutex> locker(s_mutexMsgHandlerMap);
        auto& handlerList = s_msgHandlerMap[bizCode];
        for (const auto& wpHandler : handlerList)
        {
            if (wpHandler.lock() == handler)
            {
                return false;
            }
        }
        s_msgHandlerMap[bizCode].emplace_back(handler);
        return true;
    }
    return false;
}

void AccessCtrl::unsubscribeMsg(const std::shared_ptr<MsgHandler>& handler)
{
    if (handler)
    {
        std::lock_guard<std::mutex> locker(s_mutexMsgHandlerMap);
        for (auto iter = s_msgHandlerMap.begin(); s_msgHandlerMap.end() != iter; ++iter)
        {
            iter->second.remove_if([handler](const std::weak_ptr<MsgHandler>& wpHandler) { return (wpHandler.lock() == handler); });
        }
    }
}

void AccessCtrl::onReceiveMsg(int32_t bizCode, int64_t seqId, const std::string& data)
{
    if (!s_bizExecutor)
    {
        return;
    }
    auto name = "nac.tcli.api.notify|" + std::to_string(bizCode) + "|" + std::to_string(seqId);
    s_bizExecutor->post(name, [&, name, bizCode, seqId, data]() {
        std::list<std::weak_ptr<MsgHandler>> handlerList;
        {
            std::lock_guard<std::mutex> locker(s_mutexMsgHandlerMap);
            const auto& iter = s_msgHandlerMap.find(bizCode);
            if (s_msgHandlerMap.end() == iter)
            {
                return;
            }
            handlerList = iter->second;
        }
        if (handlerList.empty())
        {
            return;
        }
        if (s_bizExecutorHook)
        {
            s_bizExecutorHook(name, [seqId, data, handlerList]() {
                for (const auto& wpHandler : handlerList)
                {
                    auto handler = wpHandler.lock();
                    if (handler)
                    {
                        (*handler)(seqId, data);
                    }
                }
            });
        }
        else
        {
            for (const auto& wpHandler : handlerList)
            {
                auto handler = wpHandler.lock();
                if (handler)
                {
                    (*handler)(seqId, data);
                }
            }
        }
    });
}

void AccessCtrl::onConnectStateChanged(const ConnectState& state)
{
    if (!s_bizExecutor)
    {
        return;
    }
    auto name = "nac.tcli.api.state|" + std::to_string((int)state);
    s_bizExecutor->post(name, [&, name, state]() {
        std::list<std::weak_ptr<StateHandler>> handlerList;
        {
            std::lock_guard<std::mutex> locker(s_mutexStateHandlerList);
            handlerList = s_stateHandlerList;
        }
        if (handlerList.empty())
        {
            return;
        }
        if (s_bizExecutorHook)
        {
            s_bizExecutorHook(name, [state, handlerList]() {
                for (const auto& wpHandler : handlerList)
                {
                    auto handler = wpHandler.lock();
                    if (handler)
                    {
                        (*handler)(state);
                    }
                }
            });
        }
        else
        {
            for (const auto& wpHandler : handlerList)
            {
                auto handler = wpHandler.lock();
                if (handler)
                {
                    (*handler)(state);
                }
            }
        }
    });
    if (ConnectState::disconnected == state) /* 断开连接, 重连 */
    {
        std::lock_guard<std::mutex> locker(s_mutexRetryTimer);
        if (s_retryTimer)
        {
            s_retryTimer->start();
        }
    }
}

void AccessCtrl::onRetryTimer()
{
    if (!s_connectService)
    {
        return;
    }
    AccessConfig cfg;
    {
        std::lock_guard<std::mutex> locker(s_mutexCfg);
        /* 设置下一次重试间隔 */
        if (s_cfg.retryInterval.size() > 1)
        {
            s_cfg.retryInterval.erase(s_cfg.retryInterval.begin());
        }
        cfg = s_cfg;
    }
    {
        std::lock_guard<std::mutex> locker(s_mutexRetryTimer);
        if (s_retryTimer)
        {
            s_retryTimer->setDelay(std::chrono::seconds(cfg.retryInterval[0]));
        }
    }
    /* 重试 */
    s_connectService->reconnect();
}
} // namespace tcli
} // namespace nac
