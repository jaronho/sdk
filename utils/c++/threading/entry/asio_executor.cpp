#include "asio_executor.h"

#include <iostream>
#include <memory>
#include <thread>

namespace threading
{
AsioExecutor::AsioExecutor(const std::string& name, size_t threadCount)
    : Executor(name)
    , m_threadIndex(0)
    , m_work(boost::asio::make_work_guard(m_context))
{
    m_threads.create_threads(
        [this, name] {
            ++m_threadIndex;
            m_context.run();
        },
        threadCount);
}

AsioExecutor::~AsioExecutor()
{
    Stop();
    m_work.reset();
    m_threads.join();
}

void AsioExecutor::Join()
{
    m_work.reset();
    m_threads.join();
}

TaskPtr AsioExecutor::Post(const TaskPtr& task)
{
    task->SetState(Task::State::QUEUING);
    m_context.post([this, task] {
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
            std::string str = "asio[" + GetName() + "] task[" + task->GetName()
                              + "] exception[what: " + std::string(e.what()) + "]\n";
            std::cout << str;
        }
        catch (...)
        {
            std::string str = "asio[" + GetName() + "] task[" + task->GetName() + "] unknown exception\n";
            std::cout << str;
        }
    });
    return task;
}

void AsioExecutor::Stop()
{
    m_context.stop();
}
} /* namespace threading */
