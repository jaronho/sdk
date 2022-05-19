#include "task.h"

#include <atomic>
#include <chrono>

namespace threading
{
static std::atomic<int64_t> s_timestamp{0}; /* 注意: std::atomic_int64_t在某些平台下未定义 */
static std::atomic_int s_count{0};

Task::Task(const std::string& name) : m_name(name)
{
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

int64_t Task::getId() const
{
    return m_id;
}

std::string Task::getName() const
{
    return m_name;
}

void Task::setState(const State& state)
{
    m_state.store(state);
    m_cv.notify_all();
}

Task::State Task::getState() const
{
    return m_state.load();
}

bool Task::isCancelled() const
{
    return m_cancelled;
}

void Task::cancel()
{
    m_cancelled = true;
}

void Task::join()
{
    std::unique_lock<std::mutex> lock(m_mutex); /* 和条件变量一同使用需要使用unique_lock */
    m_cv.wait(lock, [this] { return State::running != m_state.load(); });
}
} // namespace threading
