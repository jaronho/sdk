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
    m_deadline = deadline;
}

void DeadlineTimer::start()
{
    bool preCancel = false; /* 预先取消状态 */
    if (m_started)
    {
        preCancel = true;
        m_timer->cancel();
    }
    else
    {
        m_started = true;
    }
    const std::weak_ptr<DeadlineTimer> wpSelf = shared_from_this();
    m_timer->expires_at(m_deadline);
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
                    return;
                }
            }
            else
            {
                self->onTrigger();
            }
            self->onStopped();
        }
    });
}

void DeadlineTimer::stop()
{
    if (m_started)
    {
        m_timer->cancel();
        m_started = false;
    }
}

void DeadlineTimer::onRecover()
{
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
    if (m_started)
    {
        onTriggerFunc(shared_from_this());
    }
}

void DeadlineTimer::onStopped()
{
    /* 只做状态改变, 不调用实际取消接口 */
    m_started = false;
}
} // namespace threading
