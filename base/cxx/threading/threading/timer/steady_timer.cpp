#include "steady_timer.h"

namespace threading
{
SteadyTimer::SteadyTimer(const std::string& name, const std::chrono::steady_clock::duration& delay,
                         const std::chrono::steady_clock::duration& interval, const TimerTriggerFunc& func, const ExecutorPtr& executor)
    : Timer(name, func, executor), m_delay(delay), m_interval(interval)
{
    m_timer = std::make_shared<boost::asio::steady_timer>(getContext());
}

SteadyTimer::~SteadyTimer()
{
    stop();
}

void SteadyTimer::setDelay(const std::chrono::steady_clock::duration& delay)
{
    m_delay = delay;
}

void SteadyTimer::setInterval(const std::chrono::steady_clock::duration& interval)
{
    m_interval = interval;
}

void SteadyTimer::start()
{
    if (!m_started)
    {
        m_started = true;
        if (std::chrono::steady_clock::duration::zero() == m_delay.load())
        {
            onTrigger();
        }
        else
        {
            const std::weak_ptr<SteadyTimer>& wpSelf = shared_from_this();
            m_timer->expires_from_now(m_delay);
            m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
                const auto self = wpSelf.lock();
                if (self)
                {
                    if (code) /* 说明: `cancel`后立即调用`async_wait`, `cancel`的回调可能会晚于`async_wait`执行 */
                    {
                        self->onRecover();
                    }
                    else
                    {
                        self->onTrigger();
                    }
                }
            });
        }
    }
}

void SteadyTimer::stop()
{
    if (m_started)
    {
        m_timer->cancel();
        m_started = false;
    }
}

std::shared_ptr<SteadyTimer> SteadyTimer::onceTimer(const std::string& name, const std::chrono::steady_clock::duration& interval,
                                                    const TimerTriggerFunc& func, const ExecutorPtr& executor)
{
    return std::make_shared<threading::SteadyTimer>(name, interval, std::chrono::steady_clock::duration::zero(), func, executor);
}

std::shared_ptr<SteadyTimer> SteadyTimer::loopTimer(const std::string& name, const std::chrono::steady_clock::duration& interval,
                                                    const TimerTriggerFunc& func, const ExecutorPtr& executor)
{
    return std::make_shared<threading::SteadyTimer>(name, std::chrono::steady_clock::duration::zero(), interval, func, executor);
}

void SteadyTimer::onRecover()
{
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
    if (m_started)
    {
        auto interval = m_interval.load();
        auto nextStep = (std::chrono::steady_clock::duration::zero() != interval);
        m_started = nextStep;
        onTriggerFunc(shared_from_this());
        if (nextStep) /* 继续 */
        {
            const std::weak_ptr<SteadyTimer>& wpSelf = shared_from_this();
            m_timer->expires_from_now(interval);
            m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
                const auto self = wpSelf.lock();
                if (self && !code)
                {
                    self->onTrigger();
                }
            });
        }
    }
}
} // namespace threading
