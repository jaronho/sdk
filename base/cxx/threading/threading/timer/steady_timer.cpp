#include "steady_timer.h"

namespace threading
{
SteadyTimer::SteadyTimer(const std::string& name, const std::chrono::steady_clock::duration& delay,
                         const std::chrono::steady_clock::duration& interval, const std::function<void()>& func,
                         const ExecutorPtr& executor)
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
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_delay = delay;
}

void SteadyTimer::setInterval(const std::chrono::steady_clock::duration& interval)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_interval = interval;
}

void SteadyTimer::start()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    bool preCancel = false; /* 预先取消状态 */
    if (m_started)
    {
        preCancel = true;
        m_timer->cancel();
    }
    m_started = true;
    if (std::chrono::steady_clock::duration::zero() == m_delay)
    {
        if (preCancel)
        {
            onRecover();
        }
        else
        {
            onTrigger();
        }
    }
    else
    {
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
    }
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

void SteadyTimer::onRecover()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
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
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (!m_started)
        {
            return;
        }
    }
    /* 触发 */
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
        else /* 把回调添加到触发器列表 */
        {
            addToTriggerList(shared_from_this());
        }
    }
    /* 继续 */
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
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
