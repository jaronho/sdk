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
class AsyncTask : public Task, public std::enable_shared_from_this<AsyncTask>
{
public:
    AsyncTask(const std::string& name);
    virtual ~AsyncTask() = default;
    void run() override;

    std::function<void()> func = nullptr; /* 执行函数(在worker线程调用) */
    std::function<void()> finishCb = nullptr; /* 结束回调(在主逻辑线程调用, 一般是在主线程) */
    ExecutorPtr finishExecutor = nullptr; /* 指定结束回调的执行线程(选填), 若非空则其不受runOnce影响 */

private:
    /**
     * @brief 添加到结束器列表
     * @param task 任务
     */
    static void addToFinisherList(const std::shared_ptr<AsyncTask>& task);
};

using AsyncTaskPtr = std::shared_ptr<AsyncTask>;

/**
 * @brief 异步代理
 */
class AsyncProxy final
{
public:
    /**
     * @brief 获取线程池
     * @return 线程池
     */
    static ExecutorPtr getExecutor();

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
     */
    static void execute(const std::shared_ptr<AsyncTask>& task);

    /**
     * @brief 执行异步任务
     * @param taskName 任务名(强烈建议设置唯一标识, 以方便后续诊断)
     * @param func 异步任务执行函数(在worker线程调用)
     * @param finishCb 结束回调(在主逻辑线程调用)
     * @param finishExecutor 指定结束回调的执行线程(选填), 若非空则其回调不受runOnce接管
     */
    static void execute(const std::string& taskName, const std::function<void()>& func, const std::function<void()>& finishCb = nullptr,
                        const ExecutorPtr& finishExecutor = nullptr);
};
} // namespace threading
