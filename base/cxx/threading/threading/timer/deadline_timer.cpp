#include "deadline_timer.h"

namespace threading
{
DeadlineTimer::DeadlineTimer(const std::chrono::system_clock::time_point& deadline, const std::string& name,
                             const std::function<void()>& func, const ExecutorPtr& executor)
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
    bool preCancel = false; /* 预先取消状态 */
    if (m_started)
    {
        preCancel = true;
        m_timer->cancel();
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
            self->onStop();
        }
    });
    m_started = true;
}

void DeadlineTimer::stop()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_started)
    {
        m_timer->cancel();
        m_started = false;
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
                self->onStop();
            }
        });
    }
}

void DeadlineTimer::onTrigger()
{
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (!m_started)
        {
            return;
        }
    }
    if (m_func)
    {
        if (m_executor) /* 有执行者则把回调抛到执行线程 */
        {
            const std::weak_ptr<Timer> wpTimer = shared_from_this();
            m_executor->post(getName(), [wpTimer, func = m_func]() {
                const auto timer = wpTimer.lock();
                if (timer && func)
                {
                    func();
                }
            });
        }
        else /* 则把回调添加到触发列表 */
        {
            addToTriggerList(shared_from_this());
        }
    }
}

void DeadlineTimer::onStop()
{
    /* 只做状态改变, 不调用实际取消接口 */
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_started = false;
}
} // namespace threading
