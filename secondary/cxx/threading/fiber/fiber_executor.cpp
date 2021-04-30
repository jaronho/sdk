#include "fiber_executor.h"

#include "../diagnose/diagnose.h"
#include "../platform.h"

#include <exception>

namespace threading
{
FiberExecutor::FiberExecutor(const std::string& name, size_t maxFiberCount, size_t stackSize)
    : Executor(name), m_channel(std::make_unique<boost::fibers::buffered_channel<TaskPtr>>(maxFiberCount))
{
    Diagnose::onExecutorCreated(this);
    std::promise<void> result;
    m_thread = std::make_unique<std::thread>([this, name, stackSize, &result] {
        /* 设置线程名称 */
        auto threadId = Platform::getThreadId();
        Platform::setThreadName(name);
        /* 循环从任务队列中取出任务 */
        result.set_value();
        TaskPtr task;
        while (boost::fibers::channel_op_status::closed != m_channel->pop(task))
        {
            --m_count;
            /* 创建fiber运行 */
            boost::fibers::fiber{std::allocator_arg, boost::fibers::default_stack(stackSize),
                                 [threadId, name, task = std::move(task)]() {
                                     try
                                     {
                                         if (!task->isCancelled())
                                         {
                                             task->setState(Task::State::RUNNING);
                                             Diagnose::onTaskRunning(threadId, name, task.get());
                                             task->run();
                                         }
                                         task->setState(Task::State::FINISHED);
                                         Diagnose::onTaskFinished(threadId, name, task.get());
                                     }
                                     catch (const std::exception& e)
                                     {
                                         Diagnose::onTaskException(threadId, name, task.get(), e.what());
                                     }
                                     catch (...)
                                     {
                                         Diagnose::onTaskException(threadId, name, task.get(), "unknown exception");
                                     }
                                 }}
                .detach();
        }
        /* 当前线程会等待所有fiber结束才会退出 */
    });
    result.get_future().get();
}

FiberExecutor::~FiberExecutor()
{
    m_channel->close();
    join();
    Diagnose::onExecutorDestroyed(this);
}

void FiberExecutor::join()
{
    m_thread->join();
}

TaskPtr FiberExecutor::post(const TaskPtr& task)
{
    task->setState(Task::State::QUEUING);
    auto ret = m_channel->try_push(task);
    switch (ret)
    {
    case boost::fibers::channel_op_status::success: {
        ++m_count;
        Diagnose::bindTaskToExecutor(task.get(), this);
        break;
    }
    case boost::fibers::channel_op_status::full: {
        Diagnose::onTaskException(Platform::getThreadId(), getName(), task.get(), "fiber task full, count [" + std::to_string(m_count) + "]");
        break;
    }
    case boost::fibers::channel_op_status::closed: {
        Diagnose::onTaskException(Platform::getThreadId(), getName(), task.get(), "fiber closed, ignore");
        break;
    }
    default: {
        Diagnose::onTaskException(Platform::getThreadId(), getName(), task.get(), "fiber invalid, status [" + std::to_string((int)ret) + "]");
    }
    }
    return task;
}
} /* namespace threading */
