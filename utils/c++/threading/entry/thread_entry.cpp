#include "thread_entry.h"

#include "../fiber/fiber_executor.h"
#include "asio_executor.h"

namespace threading
{
ExecutorPtr ThreadEntry::s_logicExecutor;
ExecutorPtr ThreadEntry::s_callbackExecutor;
ExecutorPtr ThreadEntry::s_workerExecutor;
std::chrono::steady_clock::duration ThreadEntry::s_syncTimeout = std::chrono::milliseconds(5000);

void ThreadEntry::Start()
{
    s_logicExecutor = std::make_shared<FiberExecutor>("logic");
    s_callbackExecutor = std::make_shared<AsioExecutor>("callback", 1);
    s_workerExecutor = std::make_shared<AsioExecutor>("worker", std::thread::hardware_concurrency() * 2);
}

void ThreadEntry::Stop()
{
    s_logicExecutor.reset();
    s_callbackExecutor.reset();
    s_workerExecutor.reset();
}

ExecutorPtr ThreadEntry::GetLogicExecutor()
{
    return s_logicExecutor;
}

ExecutorPtr ThreadEntry::GetCallbackExecutor()
{
    return s_callbackExecutor;
}

ExecutorPtr ThreadEntry::GetWorkerExecutor()
{
    return s_workerExecutor;
}

void ThreadEntry::SetSyncTimeout(const std::chrono::steady_clock::duration& timeout)
{
    s_syncTimeout = timeout;
}

void ThreadEntry::FiberSleepFor(const std::chrono::steady_clock::duration& duration)
{
    boost::this_fiber::sleep_for(duration);
}

void ThreadEntry::FiberSleepUntil(const std::chrono::system_clock::time_point& timepoint)
{
    boost::this_fiber::sleep_until(timepoint);
}

void ThreadEntry::Async(const std::function<void()>& function, const ExecutorPtr& executor, const std::string& taskName)
{
    executor->Post(
        [function] {
            try
            {
                function();
            }
            catch (const std::exception& e)
            {
                std::string str = "fiber async raise std::exception. what=[" + std::string(e.what()) + "]\n";
                std::cout << str;
            }
            catch (...)
            {
                std::string str = "fiber async raise unknown exception.\n";
                std::cout << str;
            }
        },
        taskName);
}
} /* namespace threading */
