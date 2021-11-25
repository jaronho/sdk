#include "steady_timer.h"

#include "../timer_proxy.h"

namespace threading
{
SteadyTimer::SteadyTimer(const std::chrono::steady_clock::duration& delay, const std::chrono::steady_clock::duration& interval,
                         const std::string& name, const std::function<void()>& func, const ExecutorPtr& executor)
    : m_delay(delay), m_interval(interval), m_name(name), m_func(func), m_executor(executor), m_started(false)
{
    m_timer = std::make_shared<boost::asio::steady_timer>(*TimerProxy::getContext());
}

SteadyTimer::~SteadyTimer()
{
    stop();
}

void SteadyTimer::setDelay(const std::chrono::steady_clock::duration& delay)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_delay = delay;
}

void SteadyTimer::setInterval(const std::chrono::steady_clock::duration& interval)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_interval = interval;
}

bool SteadyTimer::isStarted()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_started;
}

void SteadyTimer::start()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    bool preCancel = false; /* 预先取消状态 */
    if (m_started)
    {
        preCancel = true;
        m_timer->cancel();
    }
    const std::weak_ptr<SteadyTimer>& wpSelf = shared_from_this();
    m_timer->expires_from_now(m_delay);
    m_timer->async_wait([wpSelf, preCancel](const boost::system::error_code& code) {
        const auto self = wpSelf.lock();
        if (self)
        {
            if (code)
            {
                /* 说明: `cancel`后立即调用`async_wait`, `cancel`的回调可能会晚于`async_wait`执行 */
                if (preCancel) /* 预先取消导致停止, 所以需要恢复 */
                {
                    self->onRecover();
                }
            }
            else
            {
                self->onTrigger();
            }
        }
    });
    m_started = true;
}

void SteadyTimer::stop()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_started)
    {
        m_timer->cancel();
        m_started = false;
    }
}

void SteadyTimer::onRecover()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_started)
    {
        const std::weak_ptr<SteadyTimer> wpSelf = shared_from_this();
        m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self && !code)
            {
                self->onTrigger();
            }
        });
    }
}

void SteadyTimer::onTrigger()
{
    /* 触发 */
    if (m_func)
    {
        if (m_executor)
        {
            m_executor->post(m_name, m_func);
        }
        else
        {
            TimerProxy::addToTriggerList(m_func);
        }
    }
    /* 继续 */
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_started && m_interval > std::chrono::steady_clock::duration::zero())
    {
        const std::weak_ptr<SteadyTimer>& wpSelf = shared_from_this();
        m_timer->expires_from_now(m_interval);
        m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self && !code)
            {
                self->onTrigger();
            }
        });
    }
    else
    {
        m_started = false;
    }
}
} // namespace threading
