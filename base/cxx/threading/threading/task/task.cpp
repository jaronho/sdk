#include "task.h"

namespace threading
{
Task::Task(const std::string& name) : m_name(name) {}

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
    std::unique_lock<std::mutex> locker(m_mutex); /* 和条件变量一同使用需要使用unique_lock */
    m_cv.wait(locker, [&] { return State::running != m_state.load(); });
}
} // namespace threading
