#pragma once
#include "thread_proxy.hpp"

namespace threading
{
/**
 * @brief 异步任务结束回调执行器钩子
 * @param name 任务名称
 * @param finishCb 结束回调
 */
using AsyncTaskFinishExecutorHook = std::function<void(const std::string& name, const std::function<void()>& finishCb)>;

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
    std::function<void()> finishCb = nullptr; /* 结束回调(在结束回调的执行线程调用) */
    ExecutorPtr finishExecutor = nullptr; /* 指定结束回调的执行线程(选填), 当为空时将在默认执行器执行结束回调 */

private:
    /**
     * @brief 响应结束回调
     */
    void onFinishCb();
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
     * @param defaultFinishExecutor 默认结束回调执行器
     * @param defaultFinishExecutorHook 默认结束回调执行构子(选填), 为空时直接执行结束回调
     */
    static void start(size_t threadCount = 4, const ExecutorPtr& defaultFinishExecutor = nullptr,
                      const AsyncTaskFinishExecutorHook& defaultFinishExecutorHook = nullptr);

    /**
     * @brief 停止模块
     */
    static void stop();

    /**
     * @brief 扩展线程个数
     * @param threadCount 线程个数
     */
    static void extend(size_t threadCount);

    /**
     * @brief 执行异步任务
     * @param task 异步任务
     */
    static void execute(const std::shared_ptr<AsyncTask>& task);

    /**
     * @brief 执行异步任务
     * @param name 任务名(强烈建议设置唯一标识, 以方便后续诊断)
     * @param func 异步任务执行函数(在worker线程调用)
     * @param finishCb 结束回调(在主逻辑线程调用)
     * @param finishExecutor 指定结束回调的执行线程(选填), 当为空时将在默认执行器执行结束回调
     */
    static void execute(const std::string& name, const std::function<void()>& func, const std::function<void()>& finishCb = nullptr,
                        const ExecutorPtr& finishExecutor = nullptr);
};
} // namespace threading
