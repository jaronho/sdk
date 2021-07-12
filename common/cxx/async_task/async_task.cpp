#include "async_task.h"

#include <assert.h>

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
    assert(s_workers);
    if (task && task->func)
    {
        threading::ThreadProxy::async(
            "worker.async",
            [task]() {
                task->func();
                if (task->finishCb)
                {
                    std::unique_lock<std::mutex> locker(s_finishMutex);
                    s_finishList.push_back(task);
                }
            },
            s_workers);
    }
}

void AsyncProxy::execute(const std::function<void()>& func, const std::function<void()>& finishCb)
{
    assert(s_workers);
    if (func)
    {
        auto task = std::make_shared<AsyncTask>();
        task->func = func;
        task->finishCb = finishCb;
        execute(task);
    }
}
