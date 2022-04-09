#include "timer.h"

#include <thread>

namespace threading
{
#ifdef _WIN32
namespace
{
/// See <http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx>
/// and <http://blogs.msdn.com/b/stevejs/archive/2005/12/19/505815.aspx> for
/// more information on the code below.
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void pthread_setname_np(DWORD dwThreadID, const char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}
} /* namespace */
#endif

static void setThreadName(const std::string& name)
{
#ifdef _WIN32
    pthread_setname_np(-1, name.c_str());
#else
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
#endif
}

std::mutex Timer::s_mutex;
std::unique_ptr<boost::asio::io_context> Timer::s_context = nullptr;
std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> Timer::s_work = nullptr;
std::unique_ptr<std::thread> Timer::s_thread = nullptr;
std::mutex Timer::s_mutexTrigger;
std::list<Timer::TriggerInfo> Timer::s_triggerList;
std::mutex Timer::s_mutexRunOnceCalled;
std::chrono::steady_clock::duration Timer::s_runOnceCalledMaxInterval = std::chrono::steady_clock::duration::zero();
std::chrono::steady_clock::time_point Timer::s_runOnceCalledTimePoint = std::chrono::steady_clock::now();

Timer::Timer()
{
    std::lock_guard<std::mutex> locker(s_mutex);
    if (!s_context)
    {
        s_context = std::make_unique<boost::asio::io_context>();
    }
    if (!s_work)
    {
        s_work = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
            boost::asio::make_work_guard(*s_context));
    }
    if (!s_thread)
    {
        s_thread = std::make_unique<std::thread>([] {
            setThreadName("timer");
            s_context->run();
        });
    }
}

boost::asio::io_context& Timer::getContext()
{
    return (*s_context);
}

void Timer::addToTriggerList(const Timer::TriggerInfo& info)
{
    std::lock_guard<std::mutex> locker(s_mutexTrigger);
    if (s_triggerList.size() >= 1024) /* runOnce未被调用或者程序阻塞, 会引起内存泄漏 */
    {
        throw std::exception(std::logic_error("var 's_triggerList' too large"));
    }
    s_triggerList.emplace_back(info);
}

bool Timer::isTriggerListWillConsumed()
{
    std::lock_guard<std::mutex> locker(s_mutexRunOnceCalled);
    if (std::chrono::steady_clock::duration::zero() == s_runOnceCalledMaxInterval)
    {
        return true;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - s_runOnceCalledTimePoint);
    return elapsed <= s_runOnceCalledMaxInterval;
}

void Timer::runOnce(const std::chrono::steady_clock::duration& maxInterval)
{
    {
        std::lock_guard<std::mutex> locker(s_mutexRunOnceCalled);
        s_runOnceCalledMaxInterval = maxInterval;
        s_runOnceCalledTimePoint = std::chrono::steady_clock::now();
    }
    TriggerInfo info;
    {
        std::lock_guard<std::mutex> locker(s_mutexTrigger);
        if (s_triggerList.empty())
        {
            return;
        }
        info = *(s_triggerList.begin());
        s_triggerList.pop_front();
    }
    const auto timer = info.wpTimer.lock();
    if (timer && info.func)
    {
        info.func();
    }
}
} // namespace threading
