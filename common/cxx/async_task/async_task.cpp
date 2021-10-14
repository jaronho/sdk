#include "async_task.h"

#include <stdexcept>

threading::ExecutorPtr AsyncProxy::s_workers = nullptr;
std::mutex AsyncProxy::s_finishMutex;
std::list<AsyncTaskPtr> AsyncProxy::s_finishList;

void AsyncProxy::init(size_t threadCount)
{
    if (!s_workers)
    {
        s_workers = threading::ThreadProxy::createAsioExecutor("worker", std::max<size_t>(1U, threadCount));
    }
}

void AsyncProxy::runOnce()
{
    std::unique_lock<std::mutex> locker(s_finishMutex);
    if (s_finishList.empty())
    {
        return;
    }
    auto task = *(s_finishList.begin());
    s_finishList.pop_front();
    locker.unlock();
    if (task->finishCb)
    {
        task->finishCb();
    }
}

void AsyncProxy::execute(const AsyncTaskPtr& task)
{
    if (!s_workers)
    {
        throw std::exception(std::logic_error("var 's_workers' is null"));
    }
    if (task && task->func)
    {
        std::string tag;
        if (!task->name.empty())
        {
            tag.append("|").append(task->name);
        }
        threading::ThreadProxy::async(
            "worker.async" + tag,
            [task]() {
                task->func();
                if (task->finishCb)
                {
                    std::unique_lock<std::mutex> locker(s_finishMutex);
                    s_finishList.emplace_back(task);
                }
            },
            s_workers);
    }
}

void AsyncProxy::execute(const std::function<void()>& func, const std::function<void()>& finishCb, const std::string& name)
{
    if (!s_workers)
    {
        throw std::exception(std::logic_error("var 's_workers' is null"));
    }
    if (func)
    {
        auto task = std::make_shared<AsyncTask>();
        task->name = name;
        task->func = func;
        task->finishCb = finishCb;
        execute(task);
    }
}
