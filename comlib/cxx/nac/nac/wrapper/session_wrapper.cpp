#include "session_wrapper.h"

#include "algorithm/snowflake/snowflake.h"
#include "threading/timer/steady_timer.h"

namespace nac
{
/**
 * @brief 会话
 */
class SessionWrapper::Session final : public std::enable_shared_from_this<SessionWrapper::Session>
{
public:
    Session(int32_t bizCode, int64_t seqId, const std::shared_ptr<SessionWrapper>& wrapper, const RespCallback& respCb)
        : m_bizCode(bizCode), m_seqId(seqId), m_wpWrapper(wrapper), m_respCb(std::move(respCb))
    {
    }

    ~Session()
    {
        stopTimer();
    }

    bool startTimer(int seconds, const threading::ExecutorPtr& timerExecutor)
    {
        if (seconds <= 0)
        {
            return false;
        }
        m_timeout = seconds;
        {
            std::lock_guard<std::mutex> locker(m_mutexTimeoutTimer);
            if (!m_timeoutTimer)
            {
                const std::weak_ptr<SessionWrapper::Session> wpSelf = shared_from_this();
                m_timeoutTimer = threading::SteadyTimer::onceTimer(
                    "session.timeout", std::chrono::seconds(seconds),
                    [wpSelf](const std::chrono::steady_clock::time_point& tp) {
                        const auto self = wpSelf.lock();
                        if (self)
                        {
                            self->onTimeout();
                        }
                    },
                    timerExecutor);
            }
            m_timeoutTimer->start();
        }
        return true;
    }

    void stopTimer()
    {
        std::lock_guard<std::mutex> locker(m_mutexTimeoutTimer);
        if (m_timeoutTimer)
        {
            m_timeoutTimer->stop();
            m_timeoutTimer.reset();
        }
    }

    void onTimeout()
    {
        {
            std::lock_guard<std::mutex> locker(m_mutexTimeoutTimer);
            if (m_timeoutTimer)
            {
                m_timeoutTimer.reset();
            }
        }
        const auto wrapper = m_wpWrapper.lock();
        if (wrapper)
        {
            WARN_LOG(wrapper->myLogger(), "会话超时({}秒): 未收到响应数据, bizCode[{}], seqId[{}].", m_timeout, m_bizCode, m_seqId);
            wrapper->onRespCallback(false, m_bizCode, m_seqId, true, m_respCb);
        }
    }

    void onResponse(const std::string& data)
    {
        stopTimer();
        if (m_respCb)
        {
            m_respCb(true, m_bizCode, m_seqId, data);
        }
    }

    void onClear()
    {
        stopTimer();
        if (m_respCb)
        {
            m_respCb(false, m_bizCode, m_seqId, "");
        }
    }

private:
    unsigned int m_timeout = 0; /* 超时时间(秒) */
    std::mutex m_mutexTimeoutTimer;
    threading::SteadyTimerPtr m_timeoutTimer = nullptr; /* 超时定时器 */
    int32_t m_bizCode = 0; /* 会话业务码 */
    int64_t m_seqId = 0; /* 序列ID */
    std::weak_ptr<SessionWrapper> m_wpWrapper; /* 会话管理器 */
    RespCallback m_respCb = nullptr; /* 发送响应回调函数 */
};

int64_t SessionWrapper::sendMsg(int32_t bizCode, int64_t seqId, const std::string& data, int timeout, const RespCallback& callback)
{
    seqId = seqId > 0 ? seqId : algorithm::Snowflake::easyGenerate();
    /* 添加路由 */
    if (timeout > 0)
    {
        auto session = std::make_shared<Session>(bizCode, seqId, shared_from_this(), callback);
        if (session->startTimer(timeout, myTimerExecutor()))
        {
            std::lock_guard<std::mutex> locker(m_mutexSessionMap);
            m_sessionMap.emplace(seqId, session);
        }
    }
    /* 发送数据包 */
    const std::weak_ptr<SessionWrapper> wpSelf = shared_from_this();
    return sendImpl(bizCode, seqId, data, timeout,
                    [wpSelf, bizCode, seqId, timeout, callback](const boost::system::error_code& code, size_t sentLength) {
                        const auto self = wpSelf.lock();
                        if (self)
                        {
                            if (code) /* 发送失败 */
                            {
                                self->onRespCallback(false, bizCode, seqId, timeout > 0, callback);
                            }
                            else /* 发送成功 */
                            {
                                if (timeout > 0) /* 需要应答, 需等待应答或超时再处理 */
                                {
                                    /* 这里不处理 */
                                }
                                else /* 不需要应答, 直接通知成功 */
                                {
                                    self->onRespCallback(true, bizCode, seqId, false, callback);
                                }
                            }
                        }
                    });
}

void SessionWrapper::clearSessionMap()
{
    std::unordered_map<int64_t, std::shared_ptr<Session>> sessionMap;
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        sessionMap = m_sessionMap;
        m_sessionMap.clear();
    }
    if (sessionMap.size() > 0)
    {
        INFO_LOG(myLogger(), "清空会话表.");
    }
    for (const auto& kv : sessionMap)
    {
        kv.second->onClear();
    }
    sessionMap.clear();
}

bool SessionWrapper::onResp(int32_t bizCode, int64_t seqId, size_t length, const std::string& data)
{
    std::shared_ptr<Session> session = nullptr;
    /* 路由判断 */
    {
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        auto iter = m_sessionMap.find(seqId);
        if (m_sessionMap.end() != iter) /* 在路由表中找到, 属于响应消息 */
        {
            session = iter->second;
            m_sessionMap.erase(iter);
        }
    }
    /* 结果处理 */
    if (session) /* 请求响应 */
    {
        TRACE_LOG(myLogger(), "收到响应数据, bizCode[{}], seqId[{}], length[{}]", bizCode, seqId, length);
        auto beg = std::chrono::steady_clock::now();
        session->onResponse(data);
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg);
        if (elapsed.count() > 0)
        {
            WARN_LOG(myLogger(), "响应数据处理完毕, bizCode[{}], seqId[{}], length[{}], 耗时: {} 毫秒.", bizCode, seqId, length,
                     elapsed.count());
        }
        return true;
    }
    return false;
}

logger::Logger SessionWrapper::myLogger()
{
    static logger::Logger s_logger;
    return s_logger;
}

std::shared_ptr<threading::Executor> SessionWrapper::myTimerExecutor()
{
    return nullptr;
}

void SessionWrapper::onRespCallback(bool sendOk, int32_t bizCode, int64_t seqId, bool waitResp, const RespCallback& callback)
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
        TRACE_LOG(myLogger(), "发送结果{}, bizCode[{}], seqId[{}]", sendOk ? "[成功]" : "[失败]", bizCode, seqId);
        auto beg = std::chrono::steady_clock::now();
        if (callback)
        {
            callback(sendOk, bizCode, seqId, "");
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg);
        if (elapsed.count() > 0)
        {
            WARN_LOG(myLogger(), "发送结果{}处理完毕, bizCode[{}], seqId[{}], 耗时: {} 毫秒.", sendOk ? "[成功]" : "[失败]", bizCode, seqId,
                     elapsed.count());
        }
    }
}
} // namespace nac
