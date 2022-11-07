#include "timer.h"

#include <chrono>
#include <mutex>
#include <thread>

#include "../diagnose/diagnose.h"
#include "../safe_queue.h"

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
 * @brief 触发器
 */
struct Trigger
{
    int64_t id = 0; /* ID */
    std::weak_ptr<Timer> wpTimer; /* 定时器 */
};

static std::atomic<int64_t> s_timerTimestamp{0}; /* 注意: std::atomic_int64_t在某些平台下未定义 */
static std::atomic_int s_timerNum{0};
static std::mutex s_mutex;
static std::unique_ptr<boost::asio::io_context> s_context;
static std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> s_work;
static std::unique_ptr<std::thread> s_thread; /* 定时器线程 */
static SafeQueue<std::shared_ptr<Trigger>> s_triggerQueue; /* 触发器队列 */

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
            setThreadName("thd::timer");
            s_context->run();
        });
    }
    /* 计算定时器ID */
    auto nt = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    if (nt == s_timerTimestamp)
    {
        ++s_timerNum;
    }
    else
    {
        s_timerNum = 0;
        s_timerTimestamp = nt;
    }
    m_id = (s_timerTimestamp << 12) + (s_timerNum & 0xFFF);
}

Timer::~Timer()
{
    Diagnose::onTimerDestroy(this);
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

void Timer::tryOnce()
{
    handleTrigger(true);
}

void Timer::waitOnce()
{
    handleTrigger(false);
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
    /* 查询当前触发器数量和最早的触发器ID */
    int64_t oldestTriggerId = 0;
    std::shared_ptr<Trigger> trigger = nullptr;
    int triggerCount = s_triggerQueue.tryFront(trigger);
    if (triggerCount > 0 && trigger)
    {
        oldestTriggerId = trigger->id;
    }
    /* 计算触发器ID */
    static int64_t s_triggerTimestamp = 0;
    static int s_triggerNum = 0;
    auto nt = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    if (nt == s_triggerTimestamp)
    {
        ++s_triggerNum;
    }
    else
    {
        s_triggerNum = 0;
        s_triggerTimestamp = nt;
    }
    int64_t triggerId = (s_triggerTimestamp << 12) + (s_triggerNum & 0xFFF);
    /* 添加到触发器列表 */
    auto discardType = Diagnose::onTriggerCreated(triggerCount, oldestTriggerId, triggerId, timer.get());
    if (DiscardType::discard_newest == discardType) /* 丢弃最新 */
    {
        return;
    }
    trigger = std::make_shared<Trigger>();
    trigger->id = triggerId;
    trigger->wpTimer = timer;
    if (DiscardType::discard_oldest == discardType) /* 丢弃最早 */
    {
        s_triggerQueue.push(trigger, 1);
    }
    else if (DiscardType::discard_all == discardType) /* 丢弃所有 */
    {
        s_triggerQueue.push(trigger, 2);
    }
    else
    {
        s_triggerQueue.push(trigger, 0);
    }
}

void Timer::handleTrigger(bool tryFlag)
{
    std::shared_ptr<Trigger> trigger = nullptr;
    if (tryFlag)
    {
        s_triggerQueue.tryPop(trigger);
    }
    else
    {
        trigger = s_triggerQueue.waitPop();
    }
    if (trigger)
    {
        const auto timer = trigger->wpTimer.lock();
        if (timer && timer->m_func)
        {
            try
            {
                Diagnose::onTriggerRunning(trigger->id, timer.get());
                timer->m_func();
                Diagnose::onTriggerFinished(trigger->id, timer.get());
            }
            catch (const std::exception& e)
            {
                Diagnose::onTriggerException(trigger->id, timer.get(), e.what());
            }
            catch (...)
            {
                Diagnose::onTriggerException(trigger->id, timer.get(), "unknown exception");
            }
        }
    }
}
} // namespace threading
