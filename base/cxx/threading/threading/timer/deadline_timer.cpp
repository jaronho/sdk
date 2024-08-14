#include "deadline_timer.h"

namespace threading
{
DeadlineTimer::DeadlineTimer(const std::string& name, const std::chrono::system_clock::time_point& deadline, const TimerTriggerFunc& func,
                             const ExecutorPtr& executor)
    : Timer(name, func, executor), m_deadline(deadline)
{
    m_timer = std::make_shared<boost::asio::system_timer>(getContext());
}

DeadlineTimer::~DeadlineTimer()
{
    stop();
}

void DeadlineTimer::setDeadline(const std::chrono::system_clock::time_point& deadline)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_deadline = deadline;
}

void DeadlineTimer::start()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (!m_started)
    {
        m_started = true;
        const std::weak_ptr<DeadlineTimer> wpSelf = shared_from_this();
        m_timer->expires_at(m_deadline);
        m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (code) /* 说明: `cancel`后立即调用`async_wait`, `cancel`的回调可能会晚于`async_wait`执行 */
                {
                    self->onRecover();
                    return;
                }
                else
                {
                    self->onTrigger();
                }
                self->onStopped();
            }
        });
    }
}

void DeadlineTimer::stop()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_started)
    {
        m_started = false;
        m_timer->cancel();
    }
}

void DeadlineTimer::onRecover()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_started)
    {
        const std::weak_ptr<DeadlineTimer> wpSelf = shared_from_this();
        m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (!code)
                {
                    self->onTrigger();
                }
                self->onStopped();
            }
        });
    }
}

void DeadlineTimer::onTrigger()
{
    bool triggered = false;
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        triggered = m_started;
    }
    if (triggered)
    {
        onTriggerFunc(shared_from_this());
    }
}

void DeadlineTimer::onStopped()
{
    /* 只做状态改变, 不调用实际取消接口 */
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_started = false;
}
} // namespace threading
