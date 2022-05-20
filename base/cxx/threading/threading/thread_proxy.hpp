#pragma once
#include "asio/asio_executor.h"
#if 1 == ENABLE_THREADING_FIBER
#include "fiber/fiber_executor.h"
#endif
#include <chrono>
#include <exception>
#include <functional>
#include <future>
#include <type_traits>

#include "diagnose/diagnose.h"

namespace threading
{
/**
 * @brief 线程代理(实现线程对外接口的封装)
 */
class ThreadProxy final
{
public:
    /**
     * @brief 创建asio线程, 注意: 如果线程内执行死循环, 当要杀死线程时需要先退出循环, 否则会阻塞调用线程
     * @param name 任务名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param threadCount 线程个数
     * @param 线程执行者
     */
    static ExecutorPtr createAsioExecutor(const std::string& name, size_t threadCount = 1)
    {
        return std::make_shared<AsioExecutor>(name, threadCount);
    }

    /**
     * @brief 同步接口
     * @param taskName 任务名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param function 任务逻辑函数
     * @param executor 指定要执行的线程
     * @param timeout 超时时间
     */
    template<typename Func, typename std::enable_if<std::is_void<typename std::result_of<Func()>::type>::value, int>::type>
    static typename std::result_of<Func()>::type
    sync(const std::string& taskName, const Func& function, const ExecutorPtr& executor,
         const std::chrono::steady_clock::duration& timeout = std::chrono::steady_clock::duration::zero())
    {
        auto result = std::make_shared<std::promise<void>>();
        auto future = result->get_future().share();
        auto task = executor->post(taskName, [function = std::move(function), result] {
            function();
            result->set_value();
        });
        if (timeout > std::chrono::steady_clock::duration::zero())
        {
            auto waitResult = future.wait_for(timeout);
            if (std::future_status::timeout == waitResult) /* 超时判断 */
            {
                int64_t millisecond = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
                Diagnose::onTaskException(0, std::string(), task.get(), "timeout [" + std::to_string(millisecond) + "]");
            }
        }
        future.get();
    }

    /**
     * @brief 同步接口
     * @param taskName 任务名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param function 任务逻辑函数
     * @param executor 指定要执行的线程
     * @param timeout 超时时间
     * @return 任意类型
     */
    template<typename Func, typename std::enable_if<!std::is_void<typename std::result_of<Func()>::type>::value, int>::type>
    static typename std::result_of<Func()>::type
    sync(const std::string& taskName, const Func& function, const ExecutorPtr& executor,
         const std::chrono::steady_clock::duration& timeout = std::chrono::steady_clock::duration::zero())
    {
        auto result = std::make_shared<std::promise<typename std::result_of<Func()>::type>>();
        auto future = result->get_future().share();
        auto task = executor->post(taskName, [function = std::move(function), result] { result->set_value(function()); });
        if (timeout > std::chrono::steady_clock::duration::zero())
        {
            auto waitResult = future.wait_for(timeout);
            if (std::future_status::timeout == waitResult) /* 超时判断 */
            {
                int64_t millisecond = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
                Diagnose::onTaskException(0, std::string(), task.get(), "timeout [" + std::to_string(millisecond) + "]");
            }
        }
        return future.get();
    }

    /**
     * @brief 异步接口
     * @param task 任务
     * @param executor 指定要执行的线程
     */
    static void async(const TaskPtr& task, const ExecutorPtr& executor)
    {
        executor->post(task);
    }

    /**
     * @brief 异步接口
     * @param taskName 任务名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param function 任务逻辑函数
     * @param executor 指定要执行的线程
     */
    static void async(const std::string& taskName, const std::function<void()>& function, const ExecutorPtr& executor)
    {
        executor->post(taskName, function);
    }

#if 1 == ENABLE_THREADING_FIBER
    /**
     * @brief 创建asio线程
     * @param name 线程名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param maxFiberCount 队列中允许的fiber最大个数, 这里默认最多1024个fiber
     * @param stackSize 每个fiber的栈空间大小(字节), 这里默认统一为512Kb
     * @param 线程执行者
     */
    static ExecutorPtr createFiberExecutor(const std::string& name, size_t maxFiberCount = 1024, size_t stackSize = 512 * 1024)
    {
        return std::make_shared<FiberExecutor>(name, maxFiberCount, stackSize);
    }

    /**
     * @brief 休眠一段时间(注意: 只用于fiber)
     * @param duration 休眠时长
     * @note 用法参考std::this_thread::sleep_for, 在fiber上使用会让出所有权
     */
    static void sleepFor(const std::chrono::steady_clock::duration& duration)
    {
        boost::this_fiber::sleep_for(duration);
    }

    /**
     * @brief 休眠直到指定时间点(注意: 只用于fiber)
     * @param timePoint 休眠停止时间点
     * @note 用法参考std::this_thread::sleep_until, 在fiber上使用会让出所有权
     */
    static void sleepUntil(const std::chrono::system_clock::time_point& timePoint)
    {
        boost::this_fiber::sleep_until(timePoint);
    }

    /**
     * @brief 同步接口(注意: 只用于从fiber线程切到asio线程)
     * @param taskName 任务名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param function 任务逻辑函数
     * @param executor 指定要执行的线程
     */
    template<typename Func, typename std::enable_if<std::is_void<typename std::result_of<Func()>::type>::value, int>::type>
    static typename std::result_of<Func()>::type await(const std::string& taskName, const Func& function, const ExecutorPtr& executor)
    {
        auto promise = std::make_shared<boost::fibers::promise<void>>();
        executor->post(taskName, [function = std::move(function), promise] {
            function();
            promise->set_value();
        });
        promise->get_future().get();
    }

    /**
     * @brief 同步接口(注意: 只用于从fiber线程切到asio线程)
     * @param taskName 任务名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param function 任务逻辑函数
     * @param executor 指定要执行的线程
     * @return 任意类型
     */
    template<typename Func, typename std::enable_if<!std::is_void<typename std::result_of<Func()>::type>::value, int>::type>
    static typename std::result_of<Func()>::type await(const std::string& taskName, const Func& function, const ExecutorPtr& executor)
    {
        auto promise = std::make_shared<boost::fibers::promise<typename std::result_of<Func()>::type>>();
        executor->post(taskName, [function = std::move(function), promise] { promise->set_value(function()); });
        return promise->get_future().get();
    }
#endif
};
} // namespace threading
