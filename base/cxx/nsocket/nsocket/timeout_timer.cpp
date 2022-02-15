#include "timeout_timer.h"

#include <thread>

namespace nsocket
{
static std::mutex s_mutex;
static std::unique_ptr<boost::asio::io_context> s_context = nullptr;
static std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> s_work = nullptr;
static std::unique_ptr<std::thread> s_thread = nullptr;

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

TimeoutTimer::TimeoutTimer(const std::chrono::steady_clock::duration& timeout, const std::function<void()>& func)
    : m_timeout(timeout), m_func(func), m_started(false)
{
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
                setThreadName("timeout_timer");
                s_context->run();
            });
        }
    }
    m_timer = std::make_shared<boost::asio::steady_timer>(s_context->get_executor());
}

TimeoutTimer::~TimeoutTimer()
{
    stop();
}

bool TimeoutTimer::isStarted()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_started;
}

void TimeoutTimer::start()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    bool preCancel = false; /* 预先取消状态 */
    if (m_started)
    {
        preCancel = true;
        m_timer->cancel();
    }
    const std::weak_ptr<TimeoutTimer> wpSelf = shared_from_this();
    m_timer->expires_from_now(m_timeout);
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
                    return;
                }
            }
            else
            {
                self->onTrigger();
            }
            self->onStop();
        }
    });
    m_started = true;
}

void TimeoutTimer::stop()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_started)
    {
        m_timer->cancel();
        m_started = false;
    }
}

void TimeoutTimer::onRecover()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_started)
    {
        const std::weak_ptr<TimeoutTimer> wpSelf = shared_from_this();
        m_timer->async_wait([wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (!code)
                {
                    self->onTrigger();
                }
                self->onStop();
            }
        });
    }
}

void TimeoutTimer::onTrigger()
{
    if (m_func)
    {
        m_func();
    }
}

void TimeoutTimer::onStop()
{
    /* 只做状态改变, 不调用实际取消接口 */
    std::lock_guard<std::mutex> locker(m_mutex);
    m_started = false;
}
} // namespace nsocket
