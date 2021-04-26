#include "asio_executor.h"

#include "../diagnose/diagnose.h"
#include "../platform.h"

#include <exception>

namespace threading
{
AsioExecutor::AsioExecutor(const std::string& name, size_t threadCount) : Executor(name), m_worker(boost::asio::make_work_guard(m_context))
{
    Diagnose::onExecutorCreated(this);
    m_threads.create_threads(
        [this, name] {
            /* �����߳����� */
            ++m_threadIndex;
            auto threadName = name + "-" + std::to_string(m_threadIndex);
            Platform::setThreadName(threadName);
            /* �����߳�id������ */
            auto threadId = Platform::getThreadId();
            if (m_threadIdNameMap.end() == m_threadIdNameMap.find(threadId))
            {
                m_threadIdNameMap[threadId] = threadName;
            }
            m_context.run();
        },
        threadCount);
}

AsioExecutor::~AsioExecutor()
{
    stop();
    join();
    Diagnose::onExecutorDestroyed(this);
}

void AsioExecutor::join()
{
    m_worker.reset();
    m_threads.join();
}

TaskPtr AsioExecutor::post(const TaskPtr& task)
{
    task->setState(Task::State::QUEUING);
    Diagnose::bindTaskToExecutor(task.get(), this);
    boost::asio::post(m_context, [this, task] {
        /* ��ȡ�߳����� */
        auto threadId = Platform::getThreadId();
        std::string threadName = std::to_string(threadId);
        if (m_threadIdNameMap.end() != m_threadIdNameMap.find(threadId))
        {
            threadName = m_threadIdNameMap[threadId];
        }
        /* ִ������ */
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

void AsioExecutor::stop()
{
    m_context.stop();
}
} /* namespace threading */
