#pragma once

#include <list>
#include <mutex>

#include "threading/thread_proxy.hpp"

/**
 * @brief 异步任务(主要用作基类)
 */
class AsyncTask
{
public:
    std::string name; /* 任务名 */
    std::function<void()> func = nullptr; /* 执行函数(在worker线程调用) */
    std::function<void()> finishCb = nullptr; /* 结束回调(在主逻辑线程调用, 一般是在主线程) */
    threading::ExecutorPtr finishExecutor = nullptr; /* 指定结束回调的执行线程(选填), 若非空则其不受runOnce影响 */
};

using AsyncTaskPtr = std::shared_ptr<AsyncTask>;

/**
 * @brief 异步代理
 */
class AsyncProxy final
{
public:
    /**
     * @brief 初始化
     * @param threadCount 用于执行异步任务的线程个数
     */
    static void init(size_t threadCount);

    /**
     * @brief 运行单次(用于监听响应回调, 在主逻辑线程中循环调用, 一般是在主线程)
     */
    static void runOnce();

    /**
     * @brief 执行异步任务
     * @param task 异步任务
     * @param finishExecutor 指定结束回调的执行线程(选填), 若非空则其不受runOnce影响
     */
    static void execute(const AsyncTaskPtr& task, const threading::ExecutorPtr& finishExecutor = nullptr);

    /**
     * @brief 执行异步任务
     * @param func 异步任务执行函数(在worker线程调用)
     * @param finishCb 结束回调(在主逻辑线程调用)
     * @param name 任务名(选填)
     * @param finishExecutor 指定结束回调的执行线程(选填), 若非空则其不受runOnce影响
     */
    static void execute(const std::function<void()>& func, const std::function<void()>& finishCb = nullptr, const std::string& name = "",
                        const threading::ExecutorPtr& finishExecutor = nullptr);

private:
    static threading::ExecutorPtr s_workers; /* 工作线程池 */
    static std::mutex s_finishMutex; /* 互斥量 */
    static std::list<AsyncTaskPtr> s_finishList; /* 结束列表 */
};
