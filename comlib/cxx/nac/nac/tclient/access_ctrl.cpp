#include "access_ctrl.h"

#include <atomic>
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

void AccessCtrl::start(const std::shared_ptr<ProtocolAdapter>& adapter, const threading::ExecutorPtr& bizExecutor,
                       const BizExecutorHook& bizExecutorHook)
{
    if (!adapter)
    {
        throw std::logic_error("arg adapter must not be empty");
    }
    m_dataChannel = std::make_shared<DataChannel>();
    m_protocolAdapter = adapter;
    m_protocolAdapter->setDataChannel(m_dataChannel);
    m_sessionManager = std::make_shared<SessionManager>();
    m_sessionManager->setDataChannel(m_dataChannel);
    m_sessionManager->setProtocolAdapter(m_protocolAdapter);
    m_sessionManager->setMsgReceiver([&](int32_t bizCode, int64_t seqId, const std::string& data) { onReceiveMsg(bizCode, seqId, data); });
    m_connectService = std::make_shared<ConnectService>();
    m_connectService->setDataChannel(m_dataChannel);
    m_connectService->setSessionManager(m_sessionManager);
    m_connectService->setConnectCallback([&](const ConnectState& state) { onConnectStateChanged(state); });
    m_bizExecutor = bizExecutor;
    m_bizExecutorHook = bizExecutorHook;
}

void AccessCtrl::setPacketVersionMismatchCallback(const PACKET_VERSION_MISMATCH_CALLBACK& callback)
{
    if (!m_protocolAdapter)
    {
        return;
    }
    m_protocolAdapter->setPacketVersionMismatchCallback([&, callback](int32_t localVersion, int32_t pktVersion) {
        if (!m_bizExecutor)
        {
            return;
        }
        auto name = "nac.tcli.api.pkt.version_mismatch";
        m_bizExecutor->post(name, [&, name, localVersion, pktVersion, callback]() {
            if (callback)
            {
                if (m_bizExecutorHook)
                {
                    m_bizExecutorHook(name, [localVersion, pktVersion, callback]() { callback(localVersion, pktVersion); });
                }
                else
                {
                    callback(localVersion, pktVersion);
                }
            }
        });
    });
}

void AccessCtrl::setPacketLengthAbnormalCallback(const PACKET_LENGTH_ABNORMAL_CALLBACK& callback)
{
    if (!m_protocolAdapter)
    {
        return;
    }
    m_protocolAdapter->setPacketLengthAbnormalCallback([&, callback](int32_t maxLength, int32_t pktLength) {
        if (!m_bizExecutor)
        {
            return;
        }
        auto name = "nac.tcli.api.pkt.length_abnormal";
        m_bizExecutor->post(name, [&, name, maxLength, pktLength, callback]() {
            if (callback)
            {
                if (m_bizExecutorHook)
                {
                    m_bizExecutorHook(name, [maxLength, pktLength, callback]() { callback(maxLength, pktLength); });
                }
                else
                {
                    callback(maxLength, pktLength);
                }
            }
        });
    });
}

void AccessCtrl::setAuthDataGenerator(const std::function<std::string()>& generator)
{
    if (!m_connectService)
    {
        return;
    }
    m_connectService->setAuthDataGenerator([generator]() {
        if (generator)
        {
            return generator();
        }
        return std::string();
    });
}

void AccessCtrl::setAuthResultCallback(const std::function<bool(const std::string& data)>& callback)
{
    if (!m_connectService)
    {
        return;
    }
    m_connectService->setAuthResultCallback([callback](const std::string& data) {
        if (callback)
        {
            return callback(data);
        }
        return true;
    });
}

void AccessCtrl::setHeartbeatDataGenerator(const std::function<std::string()>& generator)
{
    if (!m_connectService)
    {
        return;
    }
    m_connectService->setHeartbeatDataGenerator([generator]() {
        if (generator)
        {
            return generator();
        }
        return std::string();
    });
}

bool AccessCtrl::connect(const AccessConfig& cfg)
{
    if (!m_connectService)
    {
        return false;
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexCfg);
        m_cfg = cfg;
    }
    /* 创建重试定时器 */
    {
        std::lock_guard<std::mutex> locker(m_mutexRetryTimer);
        if (m_retryTimer)
        {
            m_retryTimer->stop();
        }
    }
    if (cfg.retryInterval.size() > 0)
    {
        std::lock_guard<std::mutex> locker(m_mutexRetryTimer);
        if (m_retryTimer)
        {
            m_retryTimer->setDelay(std::chrono::seconds(cfg.retryInterval[0]));
        }
        else
        {
            m_retryTimer = threading::SteadyTimer::onceTimer(
                "nac.tcli.connect.retry", std::chrono::seconds(cfg.retryInterval[0]),
                [&](const std::chrono::steady_clock::time_point& tp) { onRetryTimer(); }, m_dataChannel->getPktExecutor().lock());
        }
    }
    /* 首次连接 */
    bool ret = m_connectService->connect(cfg.localPort, cfg.address, cfg.port, cfg.sslOn, cfg.sslWay, cfg.certFmt, cfg.certFile, cfg.pkFile,
                                         cfg.pkPwd, cfg.sendBufSize, cfg.recvBufSize, cfg.enableNagle, cfg.connectTimeout,
                                         (int32_t)cfg.authBizCode, cfg.authTimeout, (int32_t)cfg.heartbeatBizCode, cfg.heartbeatInterval,
                                         cfg.heartbeatFixedSend, cfg.offlineTime);
    if (!ret)
    {
        std::lock_guard<std::mutex> locker(m_mutexRetryTimer);
        if (m_retryTimer)
        {
            m_retryTimer->start();
        }
    }
    return ret;
}

void AccessCtrl::disconnect()
{
    if (!m_connectService)
    {
        return;
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexRetryTimer);
        if (m_retryTimer)
        {
            m_retryTimer->stop();
            m_retryTimer = nullptr;
        }
    }
    m_connectService->disconnect();
}

bool AccessCtrl::setParam(unsigned int heartbeatInterval, bool heartbeatFixedSend, unsigned int offlineTime)
{
    if (m_connectService)
    {
        return m_connectService->setParam(heartbeatInterval, heartbeatFixedSend, offlineTime);
    }
    return false;
}

int64_t AccessCtrl::sendMsg(int32_t bizCode, int64_t seqId, const std::string& data, const RespCallback& callback, unsigned int timeout)
{
    auto func = [&, callback](bool sendOk, int32_t bizCode, int64_t seqId, const std::string& data) {
        if (!m_bizExecutor)
        {
            if (callback)
            {
                callback(sendOk, data);
            }
            return;
        }
        auto name = "nac.tcli.api.resp|" + std::to_string(sendOk) + "|" + std::to_string(bizCode) + "|" + std::to_string(seqId);
        m_bizExecutor->post(name, [&, name, sendOk, bizCode, seqId, data, callback]() {
            if (callback)
            {
                if (m_bizExecutorHook)
                {
                    m_bizExecutorHook(name, [sendOk, data, callback]() { callback(sendOk, data); });
                }
                else
                {
                    callback(sendOk, data);
                }
            }
        });
    };
    if (!m_sessionManager)
    {
        func(false, bizCode, seqId, "");
        return -1;
    }
    if (ConnectState::connected != m_connectState)
    {
        func(false, bizCode, seqId, "");
        return -1;
    }
    AccessConfig cfg;
    {
        std::lock_guard<std::mutex> locker(m_mutexCfg);
        cfg = m_cfg;
    }
    if (bizCode == cfg.authBizCode || bizCode == cfg.heartbeatBizCode) /* 鉴权和心跳内部处理 */
    {
        func(false, bizCode, seqId, "");
        return -1;
    }
    return m_sessionManager->sendMsg(bizCode, seqId, data, timeout, func);
}

boost::asio::ip::tcp::endpoint AccessCtrl::getLocalEndpoint()
{
    if (!m_dataChannel)
    {
        return boost::asio::ip::tcp::endpoint();
    }
    return m_dataChannel->getLocalEndpoint();
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

bool AccessCtrl::subscribeMsg(int32_t bizCode, const std::shared_ptr<MsgHandler>& handler)
{
    AccessConfig cfg;
    {
        std::lock_guard<std::mutex> locker(m_mutexCfg);
        cfg = m_cfg;
    }
    if (bizCode == cfg.authBizCode || bizCode == cfg.heartbeatBizCode) /* 鉴权和心跳内部处理 */
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

void AccessCtrl::onReceiveMsg(int32_t bizCode, int64_t seqId, const std::string& data)
{
    if (!m_bizExecutor)
    {
        return;
    }
    auto name = "nac.tcli.api.notify|" + std::to_string(bizCode) + "|" + std::to_string(seqId);
    m_bizExecutor->post(name, [&, name, bizCode, seqId, data]() {
        std::list<std::weak_ptr<MsgHandler>> handlerList;
        {
            std::lock_guard<std::mutex> locker(m_mutexMsgHandlerMap);
            const auto& iter = m_msgHandlerMap.find(bizCode);
            if (m_msgHandlerMap.end() == iter)
            {
                return;
            }
            handlerList = iter->second;
        }
        if (handlerList.empty())
        {
            return;
        }
        if (m_bizExecutorHook)
        {
            m_bizExecutorHook(name, [seqId, data, handlerList]() {
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
    m_connectState = state;
    if (!m_bizExecutor)
    {
        return;
    }
    auto name = "nac.tcli.api.state|" + std::to_string((int)state);
    m_bizExecutor->post(name, [&, name, state]() {
        std::list<std::weak_ptr<StateHandler>> handlerList;
        {
            std::lock_guard<std::mutex> locker(m_mutexStateHandlerList);
            handlerList = m_stateHandlerList;
        }
        if (handlerList.empty())
        {
            return;
        }
        if (m_bizExecutorHook)
        {
            m_bizExecutorHook(name, [state, handlerList]() {
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
        std::lock_guard<std::mutex> locker(m_mutexRetryTimer);
        if (m_retryTimer)
        {
            m_retryTimer->start();
        }
    }
}

void AccessCtrl::onRetryTimer()
{
    if (!m_connectService)
    {
        return;
    }
    AccessConfig cfg;
    {
        std::lock_guard<std::mutex> locker(m_mutexCfg);
        /* 设置下一次重试间隔 */
        if (m_cfg.retryInterval.size() > 1)
        {
            m_cfg.retryInterval.erase(m_cfg.retryInterval.begin());
        }
        cfg = m_cfg;
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexRetryTimer);
        if (m_retryTimer)
        {
            m_retryTimer->setDelay(std::chrono::seconds(cfg.retryInterval[0]));
        }
    }
    /* 重试 */
    m_connectService->reconnect();
}

AccessObserver::AccessObserver(const std::shared_ptr<AccessCtrl>& accessCtrl)
{
    if (!accessCtrl)
    {
        throw std::logic_error("access ctrl must not be empty");
    }
    m_wpAccessCtrl = accessCtrl;
}

AccessObserver::~AccessObserver()
{
    auto accessCtrl = m_wpAccessCtrl.lock();
    if (accessCtrl)
    {
        std::shared_ptr<StateHandler> stateHandler;
        std::vector<std::shared_ptr<MsgHandler>> msgHandlerList;
        {
            std::lock_guard<std::mutex> locker(m_mutexHandler);
            stateHandler = m_stateHandler;
            m_stateHandler = nullptr;
            msgHandlerList = m_msgHandlerList;
            m_msgHandlerList.clear();
        }
        if (stateHandler)
        {
            accessCtrl->unsubscribeState(stateHandler);
        }
        for (auto& handler : msgHandlerList)
        {
            accessCtrl->unsubscribeMsg(handler);
        }
    }
}

bool AccessObserver::subscribeAccessState(const std::function<void(const ConnectState& state)>& func)
{
    if (func)
    {
        auto accessCtrl = m_wpAccessCtrl.lock();
        if (accessCtrl)
        {
            auto handler = std::make_shared<StateHandler>(func);
            if (accessCtrl->subscribeState(handler))
            {
                std::lock_guard<std::mutex> locker(m_mutexHandler);
                m_stateHandler = handler;
                return true;
            }
        }
    }
    return false;
}

bool AccessObserver::subscribeAccessMsg(int32_t bizCode, const std::function<void(int64_t seqId, const std::string& data)>& func)
{
    if (func)
    {
        auto accessCtrl = m_wpAccessCtrl.lock();
        if (accessCtrl)
        {
            auto handler = std::make_shared<MsgHandler>(func);
            if (accessCtrl->subscribeMsg(bizCode, handler))
            {
                std::lock_guard<std::mutex> locker(m_mutexHandler);
                m_msgHandlerList.emplace_back(handler);
                return true;
            }
        }
    }
    return false;
}
} // namespace tcli
} // namespace nac
