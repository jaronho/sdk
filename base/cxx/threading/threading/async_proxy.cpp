#include "async_proxy.h"

#include <stdexcept>

namespace threading
{
ExecutorPtr AsyncProxy::s_workerThreads = nullptr;
std::mutex AsyncProxy::s_mutexFinish;
std::list<AsyncTaskPtr> AsyncProxy::s_finishList;
std::function<int(int nowTotalCount)> AsyncProxy::s_finishAddBeforeFunc;

void AsyncProxy::start(size_t threadCount, const std::function<int(int nowTotalCount)>& func)
{
    if (!s_workerThreads)
    {
        s_workerThreads = ThreadProxy::createAsioExecutor("worker", std::max<size_t>(1U, threadCount));
    }
    std::lock_guard<std::mutex> locker(s_mutexFinish);
    s_finishAddBeforeFunc = func;
}

void AsyncProxy::stop()
{
    if (s_workerThreads)
    {
        s_workerThreads.reset();
    }
    std::lock_guard<std::mutex> locker(s_mutexFinish);
    s_finishList.clear();
}

void AsyncProxy::runOnce()
{
    AsyncTaskPtr task = nullptr;
    {
        std::lock_guard<std::mutex> locker(s_mutexFinish);
        if (s_finishList.empty())
        {
            return;
        }
        task = *(s_finishList.begin());
        s_finishList.pop_front();
    }
    if (task && task->finishCb)
    {
        task->finishCb();
    }
}

void AsyncProxy::execute(const AsyncTaskPtr& task, const ExecutorPtr& finishExecutor)
{
    if (!s_workerThreads)
    {
        throw std::exception(std::logic_error("var 's_workerThreads' is null"));
    }
    if (task && task->func)
    {
        std::string tag;
        if (!task->name.empty())
        {
            tag.append("|").append(task->name);
        }
        ThreadProxy::async(
            "worker.async" + tag,
            [task, finishExecutor, tag]() {
                task->func();
                if (task->finishCb)
                {
                    if (finishExecutor)
                    {
                        ThreadProxy::async("worker.async.finish" + tag, task->finishCb, finishExecutor);
                    }
                    else /* 则把回调添加到结束列表 */
                    {
                        std::lock_guard<std::mutex> locker(s_mutexFinish);
                        if (s_finishAddBeforeFunc)
                        {
                            switch (s_finishAddBeforeFunc(s_finishList.size()))
                            {
                            case 1: /* 丢弃最新 */
                                return;
                            case 2: /* 丢弃最早 */
                                s_finishList.pop_front();
                                break;
                            case 3: /* 丢弃所有 */
                                s_finishList.clear();
                                break;
                            default: /* 继续添加(可能会内存持续上涨) */
                                break;
                            }
                        }
                        s_finishList.emplace_back(task);
                    }
                }
            },
            s_workerThreads);
    }
}

void AsyncProxy::execute(const std::function<void()>& func, const std::function<void()>& finishCb, const std::string& name,
                         const ExecutorPtr& finishExecutor)
{
    if (!s_workerThreads)
    {
        throw std::exception(std::logic_error("var 's_workerThreads' is null"));
    }
    if (func)
    {
        auto task = std::make_shared<AsyncTask>();
        task->name = name;
        task->func = func;
        task->finishCb = finishCb;
        execute(task, finishExecutor);
    }
}
} // namespace threading
