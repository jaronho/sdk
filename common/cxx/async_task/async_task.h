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
    std::function<void()> func = nullptr; /* 执行函数(在worker线程调用) */
    std::function<void()> finishCb = nullptr; /* 结束回调(在主线程调用) */
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
     * @brief 运行单次(用于监听响应回调, 在主线程中循环调用)
     */
    static void runOnce();

    /**
     * @brief 执行异步任务
     * @param task 异步任务
     */
    static void execute(const AsyncTaskPtr& task);

    /**
     * @brief 执行异步任务
     * @param func 异步任务执行函数(在worker线程调用)
     * @param finishCb 结束回调(在主线程调用)
     */
    static void execute(const std::function<void()>& func, const std::function<void()>& finishCb = nullptr);

private:
    static threading::ExecutorPtr s_workers; /* 工作线程池 */
    static std::mutex s_finishMutex; /* 互斥量 */
    static std::list<AsyncTaskPtr> s_finishList; /* 结束列表 */
};
