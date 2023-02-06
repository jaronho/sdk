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

static std::mutex s_mutex;
static std::unique_ptr<boost::asio::io_context> s_context;
static std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> s_work;
static std::unique_ptr<std::thread> s_thread; /* 定时器线程 */
static std::mutex s_mutexDefault;
static ExecutorPtr s_defaultExecutor = nullptr; /* 触发函数默认执行器 */
static TimerExecutorHook s_defaultExecutorHook = nullptr; /* 触发函数默认执行器钩子 */

Timer::Timer(const std::string& name, const TimerTriggerFunc& func, const ExecutorPtr& executor)
    : m_name(name), m_func(func), m_executor(executor), m_started(false)
{
    /* 创建定时器线程 */
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
            setThreadName("thd::timer");
            s_context->run();
        });
    }
}

std::string Timer::getName() const
{
    return m_name;
}

bool Timer::isStarted()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_started;
}

void Timer::setDefaultExecutor(const ExecutorPtr& executor, const TimerExecutorHook& hook)
{
    std::lock_guard<std::mutex> locker(s_mutexDefault);
    s_defaultExecutor = executor;
    s_defaultExecutorHook = hook;
}

boost::asio::io_context& Timer::getContext()
{
    return (*s_context);
}

void Timer::onTriggerFunc(const std::shared_ptr<Timer>& timer)
{
    if (!m_func)
    {
        return;
    }
    ExecutorPtr executor = m_executor;
    TimerExecutorHook hook = nullptr;
    if (!executor)
    {
        std::lock_guard<std::mutex> locker(s_mutexDefault);
        executor = s_defaultExecutor;
        hook = s_defaultExecutorHook;
    }
    if (!executor)
    {
        return;
    }
    auto ntp = std::chrono::steady_clock::now();
    const std::weak_ptr<Timer> wpTimer = timer;
    executor->post(getName(), [wpTimer, func = m_func, hook, ntp]() {
        const auto timer = wpTimer.lock();
        if (timer && func)
        {
            if (hook)
            {
                hook(timer->getName(), [wpTimer, func, ntp] {
                    const auto timer = wpTimer.lock();
                    if (timer && func)
                    {
                        func(ntp);
                    }
                });
            }
            else
            {
                func(ntp);
            }
        }
    });
}
} // namespace threading
