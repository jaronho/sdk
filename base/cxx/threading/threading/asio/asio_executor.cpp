#include "asio_executor.h"

#include <exception>

#include "../diagnose/diagnose.h"
#include "../platform.h"

namespace threading
{
AsioExecutor::AsioExecutor(const std::string& name, size_t threadCount) : Executor(name), m_worker(boost::asio::make_work_guard(m_context))
{
    Diagnose::onExecutorCreated(this);
    m_threads.create_threads(
        [this, name] {
            /* 设置线程名称 */
            ++m_threadIndex;
            auto threadName = name + "-" + std::to_string(m_threadIndex);
            Platform::setThreadName(threadName);
            /* 关联线程id和名称 */
            auto threadId = Platform::getThreadId();
            {
                std::lock_guard<std::mutex> locker(m_mapMutex);
                if (m_threadIdNameMap.end() == m_threadIdNameMap.find(threadId))
                {
                    m_threadIdNameMap.insert(std::make_pair(threadId, threadName));
                }
            }
            m_context.run();
        },
        threadCount);
}

AsioExecutor::~AsioExecutor()
{
    join();
    Diagnose::onExecutorDestroyed(this);
}

void AsioExecutor::join()
{
    m_context.stop();
    m_worker.reset();
    m_threads.join();
}

TaskPtr AsioExecutor::post(const TaskPtr& task)
{
    task->setState(Task::State::QUEUING);
    Diagnose::bindTaskToExecutor(task.get(), this);
    std::unordered_map<int, std::string> threadIdNameMap;
    {
        std::lock_guard<std::mutex> locker(m_mapMutex);
        threadIdNameMap = m_threadIdNameMap;
    }
    boost::asio::post(m_context, [threadIdNameMap, task] {
        /* 获取线程名称 */
        auto threadId = Platform::getThreadId();
        std::string threadName = std::to_string(threadId);
        auto iter = threadIdNameMap.find(threadId);
        if (threadIdNameMap.end() != iter)
        {
            threadName = iter->second;
        }
        /* 执行任务 */
        try
        {
            if (!task->isCancelled())
            {
                task->setState(Task::State::RUNNING);
                Diagnose::onTaskRunning(threadId, threadName, task.get());
                task->run();
            }
            task->setState(Task::State::FINISHED);
            Diagnose::onTaskFinished(threadId, threadName, task.get());
        }
        catch (const std::exception& e)
        {
            Diagnose::onTaskException(threadId, threadName, task.get(), e.what());
        }
        catch (...)
        {
            Diagnose::onTaskException(threadId, threadName, task.get(), "unknown exception");
        }
    });
    return task;
}

boost::asio::io_context* AsioExecutor::getContext()
{
    return &m_context;
}
} // namespace threading
