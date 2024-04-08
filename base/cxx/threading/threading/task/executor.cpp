#include "executor.h"

#include "simple_task.h"

namespace threading
{
Executor::Executor(const std::string& name, size_t maxCount) : m_name(name), m_maxCount(maxCount) {}

std::string Executor::getName() const
{
    return m_name;
}

size_t Executor::getMaxCount() const
{
    return m_maxCount;
}

TaskPtr Executor::post(const std::string& name, const std::function<void()>& func)
{
    auto taskPtr = std::make_shared<SimpleTask>(name, func);
    return post(taskPtr);
}

size_t Executor::extend(size_t count)
{
    m_maxCount += count;
    return m_maxCount;
}
} // namespace threading
