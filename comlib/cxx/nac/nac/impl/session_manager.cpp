#include "session_manager.h"

#include "algorithm/snowflake/snowflake.h"
#include "threading/timer/steady_timer.h"

namespace nac
{
class SessionManager::Session final : public std::enable_shared_from_this<SessionManager::Session>
{
public:
    Session(unsigned int bizCode, int64_t seqId, const std::shared_ptr<SessionManager>& sessionManager, ResponseCallback responseCb)
        : m_bizCode(bizCode), m_seqId(seqId), m_wpSessionManager(sessionManager), m_responseCb(std::move(responseCb))
    {
    }

    ~Session()
    {
        stopTimer();
    }

    bool startTimer(int seconds, const std::weak_ptr<threading::Executor>& wpPktExecutor)
    {
        if (seconds <= 0)
        {
            return false;
        }
        m_timeout = seconds;
        if (!m_timeoutTimer)
        {
            const std::weak_ptr<SessionManager::Session> wpSelf = shared_from_this();
            m_timeoutTimer = std::make_shared<threading::SteadyTimer>(
                "nac.session.timeout", std::chrono::seconds(seconds), std::chrono::steady_clock::duration::zero(),
                [wpSelf]() {
                    const auto self = wpSelf.lock();
                    if (self)
                    {
                        self->onTimeout();
                    }
                },
                wpPktExecutor.lock());
        }
        m_timeoutTimer->start();
        return true;
    }

    void stopTimer()
    {
        if (m_timeoutTimer)
        {
            m_timeoutTimer->stop();
            m_timeoutTimer.reset();
        }
    }

    void onTimeout()
    {
        if (m_timeoutTimer)
        {
            m_timeoutTimer.reset();
        }
        WARN_LOG(m_logger, "会话超时({}秒): 未收到业务响应数据, bizCode[{}], seqId[{}].", m_timeout, m_bizCode, m_seqId);
        const auto sessionManager = m_wpSessionManager.lock();
        if (sessionManager)
        {
            sessionManager->onResponseCallback(false, m_bizCode, m_seqId, true, m_responseCb);
        }
    }

    void onResponse(const std::string& data)
    {
        stopTimer();
        if (m_responseCb)
        {
            m_responseCb(true, m_bizCode, m_seqId, data);
        }
    }

    void onClear()
    {
        stopTimer();
        if (m_responseCb)
        {
            m_responseCb(false, m_bizCode, m_seqId, "");
        }
    }

private:
    unsigned int m_timeout = 0; /* 超时时间(秒) */
    threading::SteadyTimerPtr m_timeoutTimer = nullptr; /* 超时定时器 */
    unsigned int m_bizCode; /* 会话业务码 */
    int64_t m_seqId; /* 序列ID */
    std::weak_ptr<SessionManager> m_wpSessionManager; /* 会话管理器 */
    ResponseCallback m_responseCb; /* 发送响应回调函数 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};

void SessionManager::setDataChannel(const std::shared_ptr<DataChannel>& dataChannel)
{
    m_wpDataChannel = dataChannel;
}

void SessionManager::setProtocolAdapter(const std::shared_ptr<ProtocolAdapter>& adapter)
{
    m_connections.clear();
    if (adapter)
    {
        const std::weak_ptr<SessionManager> wpSelf = shared_from_this();
        m_connections.emplace_back(adapter->sigRecvPacket.connect([wpSelf](const std::shared_ptr<ProtocolAdapter::Packet>& pkt) -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onProcessPacket(pkt);
            }
        }));
    }
    m_wpProtocolAdapter = adapter;
}

void SessionManager::setMsgReceiver(const MsgReceiver& receiver)
{
    m_msgReceiver = receiver;
}

int64_t SessionManager::sendMsg(unsigned int bizCode, unsigned long long seqId, const std::string& data, int timeout,
                                const ResponseCallback& callback)
{
    auto pkt = std::make_shared<ProtocolAdapter::Packet>();
    pkt->bizCode = bizCode;
    pkt->seqId = seqId > 0 ? seqId : algorithm::Snowflake::easyGenerate();
    pkt->data = data;
    /* 添加路由 */
    auto session = std::make_shared<Session>(bizCode, pkt->seqId, shared_from_this(), callback);
    const auto dataChannel = m_wpDataChannel.lock();
    if (session->startTimer(timeout, dataChannel ? dataChannel->getPktExecutor().lock() : nullptr))
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        m_sessionMap.emplace(pkt->seqId, session);
    }
    /* 协议解析层适配器 */
    const auto adapter = m_wpProtocolAdapter.lock();
    if (!adapter)
    {
        ERROR_LOG(m_logger, "消息发送错误: 协议适配器为空.");
        return -1;
    }
    /* 发送数据包 */
    const std::weak_ptr<SessionManager> wpSelf = shared_from_this();
    bool ret =
        adapter->sendPacket(pkt, [wpSelf, timeout, bizCode, seqId = pkt->seqId, callback](bool ok, size_t dataLength, size_t length) {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (ok) /* 发送成功 */
                {
                    if (timeout > 0) /* 需要应答, 需等待服务器应答或超时再处理 */
                    {
                        /* 这里不处理 */
                    }
                    else /* 不需要应答, 直接通知成功 */
                    {
                        self->onResponseCallback(true, bizCode, seqId, false, callback);
                    }
                }
                else /* 发送失败 */
                {
                    WARN_LOG(self->m_logger, "数据发送失败, bizCode: {}, seqId: {}.", bizCode, seqId);
                    self->onResponseCallback(false, bizCode, seqId, timeout > 0, callback);
                }
            }
        });
    if (ret) /* 发送成功, 返回seqId */
    {
        return pkt->seqId;
    }
    /* 发送失败 */
    return -1;
}

void SessionManager::clearSessionMap()
{
    std::map<int64_t, std::shared_ptr<Session>> sessionMap;
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        sessionMap = m_sessionMap;
        m_sessionMap.clear();
    }
    if (sessionMap.size() > 0)
    {
        INFO_LOG(m_logger, "清空会话表.");
    }
    for (const auto& kv : sessionMap)
    {
        kv.second->onClear();
    }
    sessionMap.clear();
}

void SessionManager::onProcessPacket(const std::shared_ptr<ProtocolAdapter::Packet>& pkt)
{
    std::shared_ptr<Session> session;
    /* 路由判断 */
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        auto iter = m_sessionMap.find(pkt->seqId);
        if (m_sessionMap.end() != iter) /* 在路由表中找到, 属于响应客户端请求的消息 */
        {
            session = iter->second;
            m_sessionMap.erase(iter);
        }
    }
    /* 结果处理 */
    if (session) /* 请求响应 */
    {
        session->onResponse(pkt->data);
    }
    else /* 主动通知 */
    {
        if (m_msgReceiver)
        {
            m_msgReceiver(pkt->bizCode, pkt->seqId, pkt->data);
        }
    }
}

void SessionManager::onResponseCallback(bool sendOk, unsigned int bizCode, int64_t seqId, bool waitResp, const ResponseCallback& callback)
{
    bool found = false;
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        auto iter = m_sessionMap.find(seqId);
        if (m_sessionMap.end() != iter)
        {
            m_sessionMap.erase(iter);
            found = true;
        }
    }
    if (!waitResp || found)
    {
        if (callback)
        {
            callback(sendOk, bizCode, seqId, "");
        }
    }
}
} // namespace nac
