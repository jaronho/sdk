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
    if (m_started)
    {
        m_timer->cancel();
        m_started = false;
    }
    const std::weak_ptr<DeadlineTimer> wpSelf = shared_from_this();
    m_timer->expires_at(m_deadline);
    m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
        const auto self = wpSelf.lock();
        if (self)
        {
            if (!code)
            {
                self->onTrigger();
            }
            self->stop();
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

void DeadlineTimer::onTrigger()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_started)
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
}
} // namespace threading
