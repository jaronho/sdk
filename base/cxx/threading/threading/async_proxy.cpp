#include "async_proxy.h"

#include <atomic>
#include <stdexcept>

#include "diagnose/diagnose.h"
#include "safe_queue.h"

namespace threading
{
/**
 * @brief 结束器
 */
struct Finisher
{
    uint64_t id = 0; /* ID */
    std::shared_ptr<AsyncTask> task; /* 异步任务 */
};

static ExecutorPtr s_workerThreads = nullptr; /* 工作线程池 */
static SafeQueue<std::shared_ptr<Finisher>> s_finisherQueue; /* 结束器队列 */

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
    if (!task)
    {
        return;
    }
    /* 计算触发器ID */
    static std::atomic<uint64_t> s_finisherTimestamp{0};
    static std::atomic_int s_finisherNum{0};
    auto ntp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    if (ntp == s_finisherTimestamp)
    {
        ++s_finisherNum;
    }
    else
    {
        s_finisherNum = 0;
        s_finisherTimestamp = ntp;
    }
    uint64_t finisherId = (s_finisherTimestamp << 12) + (s_finisherNum & 0xFFF);
    /* 添加到结束器列表 */
    Diagnose::onFinisherCreated(s_finisherQueue.size(), finisherId, task.get());
    auto finisher = std::make_shared<Finisher>();
    finisher->id = finisherId;
    finisher->task = task;
    s_finisherQueue.push(finisher, 0);
}

ExecutorPtr AsyncProxy::getExecutor()
{
    return s_workerThreads;
}

void AsyncProxy::start(size_t threadCount)
{
    if (!s_workerThreads)
    {
        s_workerThreads = ThreadProxy::createAsioExecutor("thd::worker", std::max<size_t>(1U, threadCount));
    }
}

void AsyncProxy::stop()
{
    if (s_workerThreads)
    {
        s_workerThreads.reset();
    }
    s_finisherQueue.clear();
}

void AsyncProxy::tryOnce()
{
    handleFinisher(true);
}

void AsyncProxy::waitOnce()
{
    handleFinisher(false);
}

void AsyncProxy::execute(const std::shared_ptr<AsyncTask>& task)
{
    if (!s_workerThreads)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] var 's_workerThreads' is null");
    }
    ThreadProxy::async(task, s_workerThreads);
}

void AsyncProxy::execute(const std::string& taskName, const std::function<void()>& func, const std::function<void()>& finishCb,
                         const ExecutorPtr& finishExecutor)
{
    if (!s_workerThreads)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] var 's_workerThreads' is null");
    }
    if (func)
    {
        auto task = std::make_shared<AsyncTask>(taskName);
        task->func = func;
        task->finishCb = finishCb;
        task->finishExecutor = finishExecutor;
        execute(task);
    }
}

void AsyncProxy::handleFinisher(bool tryFlag)
{
    std::shared_ptr<Finisher> finisher = nullptr;
    if (tryFlag)
    {
        s_finisherQueue.tryPop(finisher);
    }
    else
    {
        finisher = s_finisherQueue.waitPop();
    }
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
} // namespace threading
