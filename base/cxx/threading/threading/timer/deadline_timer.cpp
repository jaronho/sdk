#include "deadline_timer.h"

#include "../timer_proxy.h"

namespace threading
{
DeadlineTimer::DeadlineTimer(const std::chrono::system_clock::time_point& deadline, const std::string& name,
                             const std::function<void()>& func, const ExecutorPtr& executor)
    : m_deadline(deadline), m_name(name), m_func(func), m_executor(executor), m_started(false)
{
    m_timer = std::make_shared<boost::asio::system_timer>(*TimerProxy::getContext());
}

DeadlineTimer::~DeadlineTimer()
{
    stop();
}

void DeadlineTimer::setDeadline(const std::chrono::system_clock::time_point& deadline)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_deadline = deadline;
}

bool DeadlineTimer::isStarted()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_started;
}

void DeadlineTimer::start()
{
    std::lock_guard<std::mutex> locker(m_mutex);
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
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_started)
    {
        m_timer->cancel();
        m_started = false;
    }
}

void DeadlineTimer::onRecover()
{
    std::lock_guard<std::mutex> locker(m_mutex);
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
}

void DeadlineTimer::onStop()
{
    /* 只做状态改变, 不调用实际取消接口 */
    std::lock_guard<std::mutex> locker(m_mutex);
    m_started = false;
}
} // namespace threading