#include "access_ctrl.h"

#include "impl/protocol_adapter_custom.h"

namespace nac
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
    MsgHandler(const std::function<void(unsigned long long seqId, const nlohmann::json& data)>& func) : m_func(func) {}

    void operator()(unsigned long long seqId, const nlohmann::json& data)
    {
        if (m_func)
        {
            m_func(seqId, data);
        }
    }

private:
    std::function<void(unsigned long long seqId, const nlohmann::json& data)> m_func = nullptr; /* 消息处理函数 */
};

AccessObserver::~AccessObserver()
{
    if (m_stateHandler)
    {
        AccessCtrl::getInstance().unsubscribeState(m_stateHandler);
        m_stateHandler.reset();
    }
    for (auto& handler : m_msgHandlerList)
    {
        AccessCtrl::getInstance().unsubscribeMsg(handler);
        handler.reset();
    }
    m_msgHandlerList.clear();
}

bool AccessObserver::subscribeAccessState(const std::function<void(const ConnectState& state)>& func)
{
    if (func)
    {
        auto handler = std::make_shared<StateHandler>(func);
        if (AccessCtrl::getInstance().subscribeState(handler))
        {
            m_stateHandler = handler;
            return true;
        }
    }
    return false;
}

bool AccessObserver::subscribeAccessMsg(const BizCode& bizCode,
                                        const std::function<void(unsigned long long seqId, const nlohmann::json& data)>& func)
{
    if (func)
    {
        auto handler = std::make_shared<MsgHandler>(func);
        if (AccessCtrl::getInstance().subscribeMsg(bizCode, handler))
        {
            m_msgHandlerList.emplace_back(handler);
            return true;
        }
    }
    return false;
}

AccessCtrl::AccessCtrl()
{
    m_dataChannel = std::make_shared<DataChannel>();
    m_protocolAdapter = std::make_shared<ProtocolAdapterCustom>();
    m_protocolAdapter->setDataChannel(m_dataChannel);
    m_sessionManager = std::make_shared<SessionManager>();
    m_sessionManager->setProtocolAdapter(m_protocolAdapter);
    m_sessionManager->setMsgReceiver(
        [&](unsigned int bizCode, int64_t seqId, const std::string& data) { onReceiveMsg(bizCode, seqId, data); });
    m_connectService = std::make_shared<ConnectService>();
    m_connectService->setDataChannel(m_dataChannel);
    m_connectService->setSessionManager(m_sessionManager);
    m_connectService->setConnectCallback([&](const ConnectState& state) { onConnectStateChanged(state); });
}

AccessCtrl& AccessCtrl::getInstance()
{
    static AccessCtrl s_instance;
    return s_instance;
}

void AccessCtrl::AccessCtrl::setAuthDataGenerator(const std::function<nlohmann::json()>& generator)
{
    m_connectService->setAuthDataGenerator([generator]() {
        if (generator)
        {
            return nlohmann::dump(generator());
        }
        return std::string();
    });
}

void AccessCtrl::setAuthResultCallback(const std::function<bool(const nlohmann::json& data)>& callback)
{
    m_connectService->setAuthResultCallback([callback](const std::string& data) {
        if (callback)
        {
            return callback(nlohmann::parse(data));
        }
        return true;
    });
}

void AccessCtrl::setHeartbeatDataGenerator(const std::function<nlohmann::json()>& generator)
{
    m_connectService->setHeartbeatDataGenerator([generator]() {
        if (generator)
        {
            return nlohmann::dump(generator());
        }
        return std::string();
    });
}

bool AccessCtrl::start(const AccessConfig& cfg)
{
    m_cfg = cfg;
    /* 创建重试定时器 */
    if (cfg.retryInterval.size() > 0)
    {
        if (m_retryTimer)
        {
            m_retryTimer->setDelay(std::chrono::seconds(cfg.retryInterval[0]));
        }
        else
        {
            m_retryTimer = std::make_shared<threading::SteadyTimer>("nac.connect.retry", std::chrono::seconds(cfg.retryInterval[0]),
                                                                    std::chrono::steady_clock::duration::zero(), [&]() { onRetryTimer(); });
        }
    }
    /* 首次连接 */
    return m_connectService->connect(cfg.address, cfg.port, cfg.certFile, cfg.privateKeyFile, cfg.privateKeyFilePwd, cfg.connectTimeout,
                                     (unsigned int)cfg.authBizCode, cfg.authTimeout, (unsigned int)cfg.heartbeatBizCode,
                                     cfg.heartbeatInterval, cfg.heartbeatFixedInterval);
}

void AccessCtrl::stop()
{
    if (m_retryTimer)
    {
        m_retryTimer->stop();
    }
    m_connectService->disconnect();
}

int64_t AccessCtrl::sendMsg(const BizCode& bizCode, unsigned long long seqId, const nlohmann::json& data, const RespCallback& callback,
                            unsigned int timeout)
{
    if (bizCode == m_cfg.authBizCode || bizCode == m_cfg.heartbeatBizCode) /* 鉴权和心跳内部处理 */
    {
        return -1;
    }
    return m_sessionManager->sendMsg((unsigned int)bizCode, seqId, nlohmann::dump(data), timeout,
                                     [&, callback](bool sendOk, unsigned int bizCode, int64_t seqId, const std::string& data) {
                                         if (callback)
                                         {
                                             callback(sendOk, nlohmann::parse(data));
                                         }
                                     });
}

bool AccessCtrl::subscribeState(const std::shared_ptr<StateHandler>& handler)
{
    if (handler)
    {
        std::lock_guard<std::mutex> locker(m_mutexStateHandlerList);
        for (const auto& wpHandler : m_stateHandlerList)
        {
            if (wpHandler.lock() == handler)
            {
                return false;
            }
        }
        m_stateHandlerList.emplace_back(handler);
        return true;
    }
    return false;
}

void AccessCtrl::unsubscribeState(const std::shared_ptr<StateHandler>& handler)
{
    if (handler)
    {
        std::lock_guard<std::mutex> locker(m_mutexMsgHandlerMap);
        m_stateHandlerList.remove_if([handler](const std::weak_ptr<StateHandler>& wpHandler) { return (wpHandler.lock() == handler); });
    }
}

bool AccessCtrl::subscribeMsg(const BizCode& bizCode, const std::shared_ptr<MsgHandler>& handler)
{
    if (bizCode == m_cfg.authBizCode || bizCode == m_cfg.heartbeatBizCode) /* 鉴权和心跳内部处理 */
    {
        return false;
    }
    if (handler)
    {
        std::lock_guard<std::mutex> locker(m_mutexMsgHandlerMap);
        auto& handlerList = m_msgHandlerMap[bizCode];
        for (const auto& wpHandler : handlerList)
        {
            if (wpHandler.lock() == handler)
            {
                return false;
            }
        }
        m_msgHandlerMap[bizCode].emplace_back(handler);
        return true;
    }
    return false;
}

void AccessCtrl::unsubscribeMsg(const std::shared_ptr<MsgHandler>& handler)
{
    if (handler)
    {
        std::lock_guard<std::mutex> locker(m_mutexMsgHandlerMap);
        for (auto iter = m_msgHandlerMap.begin(); m_msgHandlerMap.end() != iter; ++iter)
        {
            iter->second.remove_if([handler](const std::weak_ptr<MsgHandler>& wpHandler) { return (wpHandler.lock() == handler); });
        }
    }
}

void AccessCtrl::onReceiveMsg(unsigned int bizCode, int64_t seqId, const std::string& data)
{
    std::list<std::weak_ptr<MsgHandler>> handlerList;
    {
        std::lock_guard<std::mutex> locker(m_mutexMsgHandlerMap);
        const auto& iter = m_msgHandlerMap.find((BizCode)bizCode);
        if (m_msgHandlerMap.end() == iter)
        {
            return;
        }
        handlerList = iter->second;
    }
    auto obj = nlohmann::parse(data);
    for (const auto& wpHandler : handlerList)
    {
        auto handler = wpHandler.lock();
        if (handler)
        {
            (*handler)(seqId, obj);
        }
    }
}

void AccessCtrl::onConnectStateChanged(const ConnectState& state)
{
    std::list<std::weak_ptr<StateHandler>> handlerList;
    {
        std::lock_guard<std::mutex> locker(m_mutexStateHandlerList);
        handlerList = m_stateHandlerList;
    }
    for (const auto& wpHandler : handlerList)
    {
        auto handler = wpHandler.lock();
        if (handler)
        {
            (*handler)(state);
        }
    }
    if (ConnectState::disconnected == state) /* 断开连接, 重连 */
    {
        if (m_retryTimer)
        {
            m_retryTimer->start();
        }
    }
}

void AccessCtrl::onRetryTimer()
{
    /* 设置下一次重试间隔 */
    if (m_cfg.retryInterval.size() > 1)
    {
        m_cfg.retryInterval.erase(m_cfg.retryInterval.begin());
    }
    m_retryTimer->setDelay(std::chrono::seconds(m_cfg.retryInterval[0]));
    /* 重试 */
    m_connectService->reconnect();
}
} // namespace nac
