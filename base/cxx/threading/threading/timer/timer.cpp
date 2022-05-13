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
std::function<int(int nowTotalCount)> Timer::s_triggerAddBeforeFunc = nullptr;

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
    if (s_triggerAddBeforeFunc)
    {
        switch (s_triggerAddBeforeFunc(s_triggerList.size()))
        {
        case 1: /* 丢弃最新 */
            return;
        case 2: /* 丢弃最早 */
            s_triggerList.pop_front();
            break;
        case 3: /* 丢弃所有 */
            s_triggerList.clear();
            break;
        default: /* 继续添加(可能会内存持续上涨) */
            break;
        }
    }
    s_triggerList.emplace_back(info);
}

void Timer::setTriggerAddBeforeFunc(const std::function<int(int nowTotalCount)>& func)
{
    std::lock_guard<std::mutex> locker(s_mutexTrigger);
    s_triggerAddBeforeFunc = func;
}

void Timer::runOnce()
{
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
