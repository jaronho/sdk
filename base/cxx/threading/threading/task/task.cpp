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
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this] { return State::RUNNING != m_state.load(); });
}
} // namespace threading