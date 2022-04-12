#pragma once
#include <chrono>
#include <list>
#include <mutex>

#include "thread_proxy.hpp"

namespace threading
{
/**
 * @brief 异步任务(主要用作基类)
 */
class AsyncTask
{
public:
    std::string name; /* 任务名 */
    std::function<void()> func = nullptr; /* 执行函数(在worker线程调用) */
    std::function<void()> finishCb = nullptr; /* 结束回调(在主逻辑线程调用, 一般是在主线程) */
    ExecutorPtr finishExecutor = nullptr; /* 指定结束回调的执行线程(选填), 若非空则其不受runOnce影响 */
};

using AsyncTaskPtr = std::shared_ptr<AsyncTask>;

/**
 * @brief 异步代理
 */
class AsyncProxy final
{
public:
    /**
     * @brief 启动模块
     * @param threadCount 用于执行异步任务的线程个数
     */
    static void start(size_t threadCount = 4);

    /**
     * @brief 停止模块
     */
    static void stop();

    /**
     * @brief 运行单次(用于监听结束回调, 在主逻辑线程中循环调用, 一般是在主线程), 调用频率建议不超过1秒
     *        注意: 如果调用execute时有指定任务结束回调的执行线程, 则其回调不受该接口接管
     */
    static void runOnce();

    /**
     * @brief 执行异步任务
     * @param task 异步任务
     * @param finishExecutor 指定结束回调的执行线程(选填), 若非空则其回调不受runOnce接管
     */
    static void execute(const AsyncTaskPtr& task, const ExecutorPtr& finishExecutor = nullptr);

    /**
     * @brief 执行异步任务
     * @param func 异步任务执行函数(在worker线程调用)
     * @param finishCb 结束回调(在主逻辑线程调用)
     * @param name 任务名(选填)
     * @param finishExecutor 指定结束回调的执行线程(选填), 若非空则其回调不受runOnce接管
     */
    static void execute(const std::function<void()>& func, const std::function<void()>& finishCb = nullptr, const std::string& name = "",
                        const ExecutorPtr& finishExecutor = nullptr);

private:
    static ExecutorPtr s_workerThreads; /* 工作线程池 */
    static std::mutex s_mutexFinish;
    static std::list<AsyncTaskPtr> s_finishList; /* 结束列表 */
};
} // namespace threading
