#include "timer.h"

#include <chrono>
#include <list>
#include <mutex>
#include <thread>

#include "../diagnose/diagnose.h"

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

/**
 * @brief 定时器触发
 */
struct TimerTrigger
{
    std::weak_ptr<Timer> wpTimer; /* 定时器 */
    int64_t id = 0; /* 触发ID */
};

static std::atomic<int64_t> s_timestamp{0}; /* 注意: std::atomic_int64_t在某些平台下未定义 */
static std::atomic_int s_count{0};
static std::mutex s_mutex;
static std::unique_ptr<boost::asio::io_context> s_context;
static std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> s_work;
static std::unique_ptr<std::thread> s_thread; /* 定时器线程 */
static std::mutex s_mutexTriggerList;
static std::list<std::shared_ptr<TimerTrigger>> s_triggerList; /* 定时器触发列表 */

Timer::Timer(const std::string& name, const std::function<void()>& func, const ExecutorPtr& executor)
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
            setThreadName("threading::timer");
            s_context->run();
        });
    }
    /* 计算定时器ID */
    auto nt = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    if (nt == s_timestamp)
    {
        ++s_count;
    }
    else
    {
        s_count = 0;
        s_timestamp = nt;
    }
    m_id = (s_timestamp << 12) + (s_count & 0xFFF);
}

Timer::~Timer()
{
    //Diagnose::onTimerDestroy(this);
}

int64_t Timer::getId() const
{
    return m_id;
}

std::string Timer::getName() const
{
    return m_name;
}

bool Timer::isStarted() const
{
    return m_started;
}

boost::asio::io_context& Timer::getContext()
{
    return (*s_context);
}

void Timer::addToTriggerList(const std::shared_ptr<Timer>& timer)
{
    if (!timer)
    {
        return;
    }
    /* 查询当前触发数量和最早的触发ID */
    int triggerCount;
    int64_t oldestTriggerId = 0;
    std::function<TimerTriggerDiscard(int totalCount)> triggerDiscardFunc;
    {
        std::lock_guard<std::mutex> locker(s_mutexTriggerList);
        triggerCount = s_triggerList.size();
        if (triggerCount > 0)
        {
            oldestTriggerId = (*(s_triggerList.begin()))->id;
        }
    }
    /* 计算触发ID */
    static int64_t s_timestampInner = 0;
    static int s_countInner = 0;
    auto nt = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    if (nt == s_timestampInner)
    {
        ++s_countInner;
    }
    else
    {
        s_countInner = 0;
        s_timestampInner = nt;
    }
    int64_t triggerId = (s_timestampInner << 12) + (s_countInner & 0xFFF);
    /* 添加到触发列表 */
    auto discardType = TimerTriggerDiscard::none; //Diagnose::onTimerTrigger(triggerCount, oldestTriggerId, triggerId, timer.get());
    {
        std::lock_guard<std::mutex> locker(s_mutexTriggerList);
        switch (discardType)
        {
        case TimerTriggerDiscard::discard_newest: /* 丢弃最新 */
            return;
        case TimerTriggerDiscard::discard_oldest: /* 丢弃最早 */
            s_triggerList.pop_front();
            break;
        case TimerTriggerDiscard::discard_all: /* 丢弃所有 */
            s_triggerList.clear();
            break;
        default: /* 不丢弃(可能会内存持续上涨) */
            break;
        }
        auto trigger = std::make_shared<TimerTrigger>();
        trigger->wpTimer = timer;
        trigger->id = triggerId;
        s_triggerList.emplace_back(trigger);
    }
}

void Timer::runOnce()
{
    /* 从触发列表获取首个 */
    std::shared_ptr<TimerTrigger> trigger = nullptr;
    {
        std::lock_guard<std::mutex> locker(s_mutexTriggerList);
        if (s_triggerList.empty())
        {
            return;
        }
        trigger = *(s_triggerList.begin());
        s_triggerList.pop_front();
    }
    /* 执行触函数 */
    if (trigger)
    {
        const auto timer = trigger->wpTimer.lock();
        if (timer && timer->m_func)
        {
            try
            {
                //Diagnose::onTimerTriggerRunning(trigger->id, timer.get());
                timer->m_func();
                //Diagnose::onTimerTriggerFinished(trigger->id, timer.get());
            }
            catch (const std::exception& e)
            {
                //Diagnose::onTimerTriggerException(trigger->id, timer.get(), e.what());
            }
            catch (...)
            {
                //Diagnose::onTimerTriggerException(trigger->id, timer.get(), "unknown exception");
            }
        }
    }
}
} // namespace threading
