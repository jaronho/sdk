#pragma once

#include "diagnose/diagnose.h"
#include "task/executor.h"

#include <chrono>
#include <exception>
#include <functional>
#include <future>
#include <type_traits>

namespace threading
{
std::chrono::steady_clock::duration ThreadProxy::s_syncTimeout = std::chrono::seconds(30); /* 默认超时时间 */

class ThreadProxy final
{
public:
    /**
     * @brief 设置同步接口超时时间
     * @param timeout 同步接口超时时间,
     */
    static void setSyncTimeout(const std::chrono::steady_clock::duration& timeout)
    {
        s_syncTimeout = timeout;
    }

    /**
     * @brief 同步接口
     * @param taskName 任务名称
     * @param function 任务逻辑函数
     * @param executor 指定要执行的线程
     * @param timeout 超时时间
     */
    template<typename Func, typename std::enable_if<std::is_void<typename std::result_of<Func()>::type>::value, int>::type>
    static typename std::result_of<Func()>::type sync(const std::string& taskName, const Func& function, const ExecutorPtr& executor,
                                                      const std::chrono::steady_clock::duration& timeout = std::chrono::steady_clock::duration::zero)
    {
        auto result = std::make_shared<std::promise<void>>();
        auto future = result->get_future().share();
        TaskPtr task = executor->post(taskName, [function = std::move(function), result] {
            function();
            result->set_value();
        });
        if (std::chrono::steady_clock::duration::zero == timeout)
        {
            timeout = s_syncTimeout;
        }
        auto waitResult = future.wait_for(timeout);
        if (std::future_status::timeout == waitResult) /* 超时判断 */
        {
            int64_t millisecond = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
            Diagnose::onTaskException(0, std::string(), task, "timeout [" + std::to_string(millisecond) + "]");
        }
        future.get();
    }

    /**
     * @brief 同步接口
     * @param taskName 任务名称
     * @param function 任务逻辑函数
     * @param executor 指定要执行的线程
     * @param timeout 超时时间
     * @return 任意类型
     */
    template<typename Func, typename std::enable_if<!std::is_void<typename std::result_of<Func()>::type>::value, int>::type>
    static typename std::result_of<Func()>::type sync(const std::string& taskName, const Func& function, const ExecutorPtr& executor,
                                                      const std::chrono::steady_clock::duration& timeout = std::chrono::steady_clock::duration::zero)
    {
        auto result = std::make_shared<std::promise<typename std::result_of<Func()>::type>>();
        auto future = result->get_future().share();
        executor->post(taskName, [function = std::move(function), result] { result->set_value(function()); });
        if (std::chrono::steady_clock::duration::zero == timeout)
        {
            timeout = s_syncTimeout;
        }
        auto waitResult = future.wait_for(timeout);
        if (std::future_status::timeout == waitResult) /* 超时判断 */
        {
            int64_t millisecond = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
            Diagnose::onTaskException(0, std::string(), task, "timeout [" + std::to_string(millisecond) + "]");
        }
        return future.get();
    }

    /**
     * @brief 异步接口
     * @param taskName 任务名称
     * @param function 任务逻辑函数
     * @param executor 指定要执行的线程
     */
    static void async(const std::string& taskName, const std::function<void()>& function, const ExecutorPtr& executor)
    {
        executor->post(taskName, function);
    }

#if 1 == ENABLE_THREADING_FIBER
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
     * @param taskName 任务名称
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
     * @param taskName 任务名称
     * @param function 任务逻辑函数
     * @param executor 指定要执行的线程
     * @return 任意类型
     */
    template<typename Func, typename std::enable_if<!std::is_void<typename std::result_of<Func()>::type>::value, int>::type>
    static typename std::result_of<Func()>::type await(const std::string& taskName, const Func& function, const ExecutorPtr& executor)
    {
        auto promise = std::make_shared<boost::fibers::promise<typename std::result_of<Func()>::type>>();
        executor->Post(taskName, [function = std::move(function), promise] { promise->set_value(function()); });
        return promise->get_future().get();
    }
#endif

private:
    static std::chrono::steady_clock::duration s_syncTimeout; /* 调用同步接口时的超时时间 */
};
} /* namespace threading */
