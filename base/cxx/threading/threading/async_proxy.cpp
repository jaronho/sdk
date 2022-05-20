#include "async_proxy.h"

#include <stdexcept>

#include "diagnose/diagnose.h"

namespace threading
{
/**
 * @brief 结束器
 */
struct Finisher
{
    int64_t id = 0; /* ID */
    std::shared_ptr<AsyncTask> task; /* 异步任务 */
};

static ExecutorPtr s_workerThreads = nullptr; /* 工作线程池 */
static std::mutex s_mutexFinisherList;
static std::list<std::shared_ptr<Finisher>> s_finisherList; /* 结束器列表 */

AsyncTask::AsyncTask(const std::string& name) : Task(name) {}

void AsyncTask::run()
{
    if (func)
    {
        func();
    }
    if (finishCb)
    {
        if (finishExecutor) /* 有执行者则把回调抛到执行线程 */
        {
            ThreadProxy::async(getName(), finishCb, finishExecutor);
        }
        else /* 把回调添加到结束器列表 */
        {
            addToFinisherList(shared_from_this());
        }
    }
}

void AsyncTask::addToFinisherList(const std::shared_ptr<AsyncTask>& task)
{
    /* 查询当前结束器数量和最早的结束器ID */
    int finisherCount;
    int64_t oldestFinisherId = 0;
    std::function<DiscardType(int totalCount)> finisherDiscardFunc;
    {
        std::lock_guard<std::mutex> locker(s_mutexFinisherList);
        finisherCount = s_finisherList.size();
        if (finisherCount > 0)
        {
            oldestFinisherId = (*(s_finisherList.begin()))->id;
        }
    }
    /* 计算触发器ID */
    static int64_t s_finisherTimestamp = 0;
    static int s_finisherNum = 0;
    auto nt = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    if (nt == s_finisherTimestamp)
    {
        ++s_finisherNum;
    }
    else
    {
        s_finisherNum = 0;
        s_finisherTimestamp = nt;
    }
    int64_t finisherId = (s_finisherTimestamp << 12) + (s_finisherNum & 0xFFF);
    /* 添加到结束器列表 */
    auto discardType = Diagnose::onFinisherCreated(finisherCount, oldestFinisherId, finisherId, task.get());
    {
        std::lock_guard<std::mutex> locker(s_mutexFinisherList);
        switch (discardType)
        {
        case DiscardType::discard_newest: /* 丢弃最新 */
            return;
        case DiscardType::discard_oldest: /* 丢弃最早 */
            s_finisherList.pop_front();
            break;
        case DiscardType::discard_all: /* 丢弃所有 */
            s_finisherList.clear();
            break;
        default: /* 不丢弃(可能会内存持续上涨) */
            break;
        }
        auto finisher = std::make_shared<Finisher>();
        finisher->id = finisherId;
        finisher->task = task;
        s_finisherList.emplace_back(finisher);
    }
}

void AsyncProxy::start(size_t threadCount)
{
    if (!s_workerThreads)
    {
        s_workerThreads = ThreadProxy::createAsioExecutor("threading::worker", std::max<size_t>(1U, threadCount));
    }
}

void AsyncProxy::stop()
{
    if (s_workerThreads)
    {
        s_workerThreads.reset();
    }
    std::lock_guard<std::mutex> locker(s_mutexFinisherList);
    s_finisherList.clear();
}

void AsyncProxy::runOnce()
{
    /* 获取首个结束器 */
    std::shared_ptr<Finisher> finisher = nullptr;
    {
        std::lock_guard<std::mutex> locker(s_mutexFinisherList);
        if (s_finisherList.empty())
        {
            return;
        }
        finisher = *(s_finisherList.begin());
        s_finisherList.pop_front();
    }
    /* 执行结束器 */
    if (finisher)
    {
        if (finisher->task && finisher->task->finishCb)
        {
            try
            {
                Diagnose::onFinisherRunning(finisher->id, finisher->task.get());
                finisher->task->finishCb();
                Diagnose::onFinisherFinished(finisher->id, finisher->task.get());
            }
            catch (const std::exception& e)
            {
                Diagnose::onFinisherException(finisher->id, finisher->task.get(), e.what());
            }
            catch (...)
            {
                Diagnose::onFinisherException(finisher->id, finisher->task.get(), "unknown exception");
            }
        }
    }
}

void AsyncProxy::execute(const std::shared_ptr<AsyncTask>& task, const ExecutorPtr& finishExecutor)
{
    if (!s_workerThreads)
    {
        throw std::exception(std::logic_error("var 's_workerThreads' is null"));
    }
    ThreadProxy::async(task, s_workerThreads);
}

void AsyncProxy::execute(const std::string& taskName, const std::function<void()>& func, const std::function<void()>& finishCb,
                         const ExecutorPtr& finishExecutor)
{
    if (!s_workerThreads)
    {
        throw std::exception(std::logic_error("var 's_workerThreads' is null"));
    }
    if (func)
    {
        auto task = std::make_shared<AsyncTask>(taskName);
        task->func = func;
        task->finishCb = finishCb;
        execute(task, finishExecutor);
    }
}
} // namespace threading
