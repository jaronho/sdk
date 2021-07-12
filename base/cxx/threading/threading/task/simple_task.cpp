#include "simple_task.h"

namespace threading
{
SimpleTask::SimpleTask(const std::string& name, const std::function<void()>& func) : Task(name), m_func(func) {}

void SimpleTask::run()
{
    if (m_func)
    {
        m_func();
    }
}
} // namespace threading
