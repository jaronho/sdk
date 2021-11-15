#include "timer_proxy.h"

#include <stdexcept>

namespace threading
{
std::shared_ptr<AsioExecutor> TimerProxy::s_timerThread = nullptr;
std::mutex TimerProxy::s_triggerMutex;
std::list<std::function<void()>> TimerProxy::s_triggerList;

void TimerProxy::start()
{
    if (!s_timerThread)
    {
        s_timerThread = std::dynamic_pointer_cast<AsioExecutor>(ThreadProxy::createAsioExecutor("timer", 1));
    }
}

void TimerProxy::stop()
{
    if (s_timerThread)
    {
        s_timerThread.reset();
    }
    std::unique_lock<std::mutex> locker(s_triggerMutex);
    s_triggerList.clear();
}

void TimerProxy::runOnce()
{
    std::unique_lock<std::mutex> locker(s_triggerMutex);
    if (s_triggerList.empty())
    {
        return;
    }
    auto func = *(s_triggerList.begin());
    s_triggerList.pop_front();
    locker.unlock();
    if (func)
    {
        func();
    }
}

boost::asio::io_context* TimerProxy::getContext()
{
    if (!s_timerThread)
    {
        throw std::exception(std::logic_error("var 's_timerThread' is null"));
    }
    return s_timerThread->getContext();
}

void TimerProxy::addToTriggerList(const std::function<void()>& func)
{
    if (s_timerThread)
    {
        std::unique_lock<std::mutex> locker(s_triggerMutex);
        s_triggerList.emplace_back(func);
    }
}
} // namespace threading
