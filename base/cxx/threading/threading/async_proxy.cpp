#include "async_proxy.h"

#include <stdexcept>

namespace threading
{
static ExecutorPtr s_workerThreads = nullptr; /* 工作线程池 */
static ExecutorPtr s_defaultFinishExecutor = nullptr; /* 结束回调默认执行器 */
static AsyncTaskFinishExecutorHook s_defaultFinishExecutorHook = nullptr; /* 结束回调默认执行器钩子 */

AsyncTask::AsyncTask(const std::string& name) : Task(name) {}

void AsyncTask::run()
{
    if (func)
    {
        func();
    }
    onFinishCb();
}

void AsyncTask::onFinishCb()
{
    if (!finishCb)
    {
        return;
    }
    ExecutorPtr executor = finishExecutor;
    AsyncTaskFinishExecutorHook hook = nullptr;
    if (!finishExecutor)
    {
        executor = s_defaultFinishExecutor;
        hook = s_defaultFinishExecutorHook;
    }
    if (!executor)
    {
        return;
    }
    executor->post(getName(), [name = getName(), cb = finishCb, hook]() {
        if (cb)
        {
            if (hook)
            {
                hook(name, [cb] {
                    if (cb)
                    {
                        cb();
                    }
                });
            }
            else
            {
                cb();
            }
        }
    });
}

void AsyncProxy::start(size_t threadCount, const ExecutorPtr& defaultFinishExecutor,
                       const AsyncTaskFinishExecutorHook& defaultFinishExecutorHook)
{
    if (!s_workerThreads)
    {
        s_workerThreads = ThreadProxy::createAsioExecutor("thd::worker", std::max<size_t>(1U, threadCount));
    }
    s_defaultFinishExecutor = defaultFinishExecutor;
    s_defaultFinishExecutorHook = defaultFinishExecutorHook;
}

void AsyncProxy::stop()
{
    if (s_workerThreads)
    {
        s_workerThreads.reset();
    }
    if (s_defaultFinishExecutor)
    {
        s_defaultFinishExecutor.reset();
    }
    if (s_defaultFinishExecutorHook)
    {
        s_defaultFinishExecutorHook = nullptr;
    }
}

size_t AsyncProxy::getTotalCount()
{
    if (s_workerThreads)
    {
        return s_workerThreads->getMaxCount();
    }
    return 0;
}

size_t AsyncProxy::getFreeCount()
{
    if (s_workerThreads)
    {
        return (s_workerThreads->getMaxCount() - s_workerThreads->getBusyCount());
    }
    return 0;
}

size_t AsyncProxy::extend(size_t threadCount)
{
    if (s_workerThreads)
    {
        return s_workerThreads->extend(threadCount);
    }
    return 0;
}

void AsyncProxy::execute(const std::shared_ptr<AsyncTask>& task)
{
    if (!s_workerThreads)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] var 's_workerThreads' is null");
    }
    s_workerThreads->post(task);
}

void AsyncProxy::execute(const std::string& name, const std::function<void()>& func, const std::function<void()>& finishCb,
                         const ExecutorPtr& finishExecutor)
{
    if (!s_workerThreads)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] var 's_workerThreads' is null");
    }
    if (func)
    {
        auto task = std::make_shared<AsyncTask>(name);
        task->func = func;
        task->finishCb = finishCb;
        task->finishExecutor = finishExecutor;
        execute(task);
    }
}
} // namespace threading
