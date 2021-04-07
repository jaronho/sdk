#include "task.h"

namespace threading
{
Task::Task(const std::string& name)
    : m_name(name)
    , m_state(State::CREATED)
    , m_cancelled(false)
{
}

Task::~Task() = default;

std::string Task::GetName() const
{
    return m_name;
}

void Task::Cancel()
{
    m_cancelled = true;
}

bool Task::IsCancelled() const
{
    return m_cancelled;
}

Task::State Task::GetState() const
{
    return m_state;
}

void Task::Join()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this] { return m_state != State::RUNNING; });
}

void Task::SetState(State state)
{
    m_state = state;
    m_cv.notify_all();
}
} /* namespace threading */
