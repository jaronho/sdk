#include "simple_task.h"

namespace threading
{
SimpleTask::SimpleTask(const CallbackType& callback, const std::string& name)
    : Task(name)
    , m_callback(callback)
{
}

SimpleTask::~SimpleTask() = default;

void SimpleTask::Run()
{
    if (m_callback)
    {
        m_callback();
    }
}
} /* namespace threading */
