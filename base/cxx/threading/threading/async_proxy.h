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
     * @param func 函数, 说明: runOnce未被调用或者程序阻塞时结束列表会一直增长, 会造成内存泄漏, 此函数
     *                        用于监听当前总的结束列表数量, 然后可以自行决定是否要抛异常还是其他处理方式
     *                   参数: nowTotalCount-当前总的结束数量
     *                   返回值: 1-丢弃最新, 2-丢弃最早, 3-丢弃所有, 0和其他值-继续添加(可能会内存持续上涨)           
     */
    static void start(size_t threadCount = 4, const std::function<int(int nowTotalCount)>& func = nullptr);

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
    static std::function<int(int nowTotalCount)> s_finishAddBeforeFunc; /* 添加添加前函数 */
};
} // namespace threading
