#include "timer.h"

#include <list>
#include <mutex>
#include <thread>

namespace threading
{
static std::mutex s_mutex;
static std::unique_ptr<boost::asio::io_context> s_context = nullptr;
static std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> s_work = nullptr;
static std::unique_ptr<std::thread> s_thread = nullptr;
static std::mutex s_mutexTrigger;
static std::list<std::function<void()>> s_triggerList;

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

void Timer::addToTriggerList(const std::function<void()>& func)
{
    std::lock_guard<std::mutex> locker(s_mutexTrigger);
    if (s_triggerList.size() >= 1024) /* runOnce未被调用或者程序阻塞, 会引起内存泄漏 */
    {
        throw std::exception(std::logic_error("var 's_triggerList' too large"));
    }
    s_triggerList.emplace_back(func);
}

void Timer::runOnce()
{
    std::function<void()> func = nullptr;
    {
        std::lock_guard<std::mutex> locker(s_mutexTrigger);
        if (s_triggerList.empty())
        {
            return;
        }
        func = *(s_triggerList.begin());
        s_triggerList.pop_front();
    }
    if (func)
    {
        func();
    }
}
} // namespace threading