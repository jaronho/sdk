#include "fiber_executor.h"

namespace threading
{
FiberExecutor::FiberExecutor(const std::string& name)
    : Executor(name)
    , m_channel(std::make_unique<boost::fibers::buffered_channel<TaskPtr>>(1024))
    , m_count(0)
{
    std::promise<void> result;
    m_thread = std::make_unique<std::thread>([this, name, &result] {
        result.set_value();
        TaskPtr task;
        /* 循环从任务队列中取出任务 */
        while (boost::fibers::channel_op_status::closed != m_channel->pop(task))
        {
            --m_count;
            /* 创建fiber运行 */
            boost::fibers::fiber{std::allocator_arg, boost::fibers::default_stack(512 * 1024),
                                 [this, task]() {
                                     try
                                     {
                                         if (!task->IsCancelled())
                                         {
                                             task->SetState(Task::State::RUNNING);
                                             task->Run();
                                         }
                                         task->SetState(Task::State::FINISHED);
                                     }
                                     catch (const std::exception& e)
                                     {
                                         std::string str = "fiber[" + GetName() + "] task[" + task->GetName()
                                                           + "] exception[what: " + std::string(e.what()) + "]\n";
                                         std::cout << str;
                                     }
                                     catch (...)
                                     {
                                         std::string str = "fiber[" + GetName() + "] task[" + task->GetName()
                                                           + "] unknown exception\n";
                                         std::cout << str;
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
    m_thread->join();
}

void FiberExecutor::Join()
{
    m_thread->join();
}

TaskPtr FiberExecutor::Post(const TaskPtr& task)
{
    task->SetState(Task::State::QUEUING);
    const auto ret = m_channel->try_push(task);
    switch (ret)
    {
    case boost::fibers::channel_op_status::success: {
        ++m_count;
        break;
    }
    case boost::fibers::channel_op_status::full: {
        std::string str = "fiber[" + GetName() + "] task full, count: " + std::to_string(m_count) + "\n";
        std::cout << str;
        throw std::exception(str.c_str());
        break;
    }
    case boost::fibers::channel_op_status::closed: {
        std::string str = "fiber[" + GetName() + "] closed, ignore\n";
        std::cout << str;
        break;
    }
    default: {
        std::string str = "fiber[" + GetName() + "] invalid status: " + std::to_string((int)ret) + "\n";
        std::cout << str;
    }
    }
    return task;
}
} /* namespace threading */
