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
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_delay = delay;
}

void SteadyTimer::setInterval(const std::chrono::steady_clock::duration& interval)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_interval = interval;
}

bool SteadyTimer::isStarted()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_started;
}

void SteadyTimer::start()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    stop();
    const std::weak_ptr<SteadyTimer>& wpSelf = shared_from_this();
    m_timer->expires_from_now(m_delay);
    m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
        const auto self = wpSelf.lock();
        if (self && !code)
        {
            self->onTrigger();
        }
    });
    m_started = true;
}

void SteadyTimer::stop()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_started)
    {
        m_timer->cancel();
        m_started = false;
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
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_started && m_interval > std::chrono::steady_clock::duration::zero())
    {
        const std::weak_ptr<SteadyTimer>& wpSelf = shared_from_this();
        m_timer->expires_from_now(m_interval);
        m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
            /* 如果`stop`之后立即`start`, 前一次`stop`的最后一次回调可能晚于`start`执行, 会导致`m_started`状态错误 */
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
