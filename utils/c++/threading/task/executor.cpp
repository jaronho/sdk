#include "executor.h"

#include "simple_task.h"

namespace threading
{
Executor::Executor(const std::string& name)
    : m_name(name)
{
}

Executor::~Executor()
{
}

std::string Executor::GetName() const
{
    return m_name;
}

TaskPtr Executor::Post(const std::function<void()>& task, const std::string& name)
{
    const auto taskPtr = std::make_shared<SimpleTask>(task, name);
    return Post(taskPtr);
}
} /* namespace threading */
