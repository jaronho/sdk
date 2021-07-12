#include "executor.h"

#include "simple_task.h"

namespace threading
{
Executor::Executor(const std::string& name) : m_name(name) {}

std::string Executor::getName() const
{
    return m_name;
}

TaskPtr Executor::post(const std::string& name, const std::function<void()>& func)
{
    auto taskPtr = std::make_shared<SimpleTask>(name, func);
    return post(taskPtr);
}
} // namespace threading
