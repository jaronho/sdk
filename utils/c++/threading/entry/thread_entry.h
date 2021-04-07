#pragma once

#include <boost/fiber/all.hpp>
#include <chrono>
#include <exception>
#include <functional>
#include <future>
#include <type_traits>

#include "../task/executor.h"

namespace threading
{
class ThreadEntry final
{
    ThreadEntry() = delete;

public:
    static void Start();
    static void Stop();

    static ExecutorPtr GetLogicExecutor();
    static ExecutorPtr GetCallbackExecutor();
    static ExecutorPtr GetWorkerExecutor();

    /**
     * @brief 设置同步接口超时时间
     * @param timeout 同步接口超时时间(默认为5秒)
     */
    static void SetSyncTimeout(const std::chrono::steady_clock::duration& timeout);

    /**
     * @brief 休眠一段时间
     * @param duration 休眠时长
     * @note 用法参考std::this_thread::sleep_for, 在协程上使用会让出所有权
     */
    static void FiberSleepFor(const std::chrono::steady_clock::duration& duration);

    /**
     * @brief 休眠直到指定时间点
     * @param timePoint 休眠停止时间点
     * @note 用法参考std::this_thread::sleep_until, 在协程上使用会让出所有权
     */
    static void FiberSleepUntil(const std::chrono::system_clock::time_point& timepoint);

    /**
     * @brief 同步执行并阻塞等待结果
     * 用于API层在UI调用时切换到逻辑层线程执行
     */
    template<typename Fn,
             typename std::enable_if<!std::is_void<typename std::result_of<Fn()>::type>::value, int>::type = 0>
    static typename std::result_of<Fn()>::type Sync(Fn&& function, const std::string& taskName = std::string());

    template<typename Fn,
             typename std::enable_if<std::is_void<typename std::result_of<Fn()>::type>::value, int>::type = 0>
    static typename std::result_of<Fn()>::type Sync(Fn&& function, const std::string& taskName = std::string());

    /**
     * @brief 异步执行并通过回调返回结果
     * 用于API层在UI调用时切换到逻辑层线程执行, 并且在回调线程中执行回调
     */
    template<typename T>
    static void Async(const std::function<T()>& function,
                      const std::function<void(const std::string& message, T)>& callback,
                      const std::string& taskName = std::string());

    static void Async(const std::function<void()>& function,
                      const std::function<void(const std::string& message)>& callback,
                      const std::string& taskName = std::string());

    /**
     * @brief 在指定executor异步执行并通过纤程等待结果
     * 用于逻辑层内部纤程方式调度执行
     */
    template<typename Fn,
             typename std::enable_if<!std::is_void<typename std::result_of<Fn()>::type>::value, int>::type = 0>
    static typename std::result_of<Fn()>::type Await(Fn&& function, const ExecutorPtr& executor = GetLogicExecutor(),
                                                     const std::string& taskName = std::string());

    template<typename Fn,
             typename std::enable_if<std::is_void<typename std::result_of<Fn()>::type>::value, int>::type = 0>
    static typename std::result_of<Fn()>::type Await(Fn&& function, const ExecutorPtr& executor = GetLogicExecutor(),
                                                     const std::string& taskName = std::string());

    static void Async(const std::function<void()>& function, const ExecutorPtr& executor = GetLogicExecutor(),
                      const std::string& taskName = std::string());

private:
    static ExecutorPtr s_logicExecutor;
    static ExecutorPtr s_callbackExecutor;
    static ExecutorPtr s_workerExecutor;

    static std::chrono::steady_clock::duration s_syncTimeout;
};

template<typename Fn, typename std::enable_if<!std::is_void<typename std::result_of<Fn()>::type>::value, int>::type>
typename std::result_of<Fn()>::type ThreadEntry::Sync(Fn&& function, const std::string& taskName)
{
    auto result = std::make_shared<std::promise<typename std::result_of<Fn()>::type>>();
    auto future = result->get_future().share();

    GetLogicExecutor()->Post(
        [function, result, future] {
            try
            {
                result->set_value(function());
            }
            catch (const std::exception& e)
            {
                std::string str = "ThreadEntry::Sync task raise exception: " + std::string(e.what()) + "\n";
                std::cout << str;
                try
                {
                    result->set_exception(std::current_exception());
                }
                catch (...)
                {
                }
            }
            catch (...)
            {
                std::string str = "ThreadEntry::Sync task raise exception: unknown\n";
                std::cout << str;
                try
                {
                    result->set_exception(std::current_exception());
                }
                catch (...)
                {
                }
            }
        },
        taskName);

    const auto waitResult = future.wait_for(s_syncTimeout);
    if (std::future_status::timeout == waitResult)
    {
        std::string str = "Sync interface timeout["
                          + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(s_syncTimeout).count())
                          + " ms]\n";
        std::cout << str;
        throw std::exception(str.c_str());
    }
    return future.get();
}

template<typename Fn, typename std::enable_if<std::is_void<typename std::result_of<Fn()>::type>::value, int>::type>
typename std::result_of<Fn()>::type ThreadEntry::Sync(Fn&& function, const std::string& taskName)
{
    auto result = std::make_shared<std::promise<void>>();
    auto future = result->get_future().share();

    GetLogicExecutor()->Post(
        [function, result, future] {
            try
            {
                function();
                result->set_value();
            }
            catch (const std::exception& e)
            {
                std::string str = "ThreadEntry::Sync task raise exception: " + std::string(e.what()) + "\n";
                std::cout << str;
                try
                {
                    result->set_exception(std::current_exception());
                }
                catch (...)
                {
                }
            }
            catch (...)
            {
                std::string str = "ThreadEntry::Sync task raise exception: unknown\n";
                std::cout << str;
                try
                {
                    result->set_exception(std::current_exception());
                }
                catch (...)
                {
                }
            }
        },
        taskName);

    const auto waitResult = future.wait_for(s_syncTimeout);
    if (std::future_status::timeout == waitResult)
    {
        std::string str = "Sync interface timeout["
                          + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(s_syncTimeout).count())
                          + " ms]\n";
        std::cout << str;
        throw std::exception(str.c_str());
    }
    return future.get();
}

template<typename T>
void ThreadEntry::Async(const std::function<T()>& function,
                        const std::function<void(const std::string& message, T)>& callback, const std::string& taskName)
{
    std::function<void(const std::string& message, T)> localCallback = callback;

    GetLogicExecutor()->Post(
        [function, callback = std::move(localCallback), taskNameTmp = taskName]() mutable {
            std::string message;
            T result;
            try
            {
                result = function();
            }
            catch (const std::exception& e)
            {
                message = "ThreadEntry::Async task raise exception: " + std::string(e.what()) + "\n";
                std::cout << message;
            }
            catch (...)
            {
                message = "ThreadEntry::Async task raise exception: unknown\n";
                std::cout << message;
            }

            GetCallbackExecutor()->Post(
                [callback = std::move(callback), message, result] {
                    if (callback)
                    {
                        callback(message, result);
                    }
                },
                taskNameTmp);
        },
        taskName);
}

inline void ThreadEntry::Async(const std::function<void()>& function,
                               const std::function<void(const std::string& message)>& callback,
                               const std::string& taskName)
{
    std::function<void(const std::string& message)> localCallback = callback;

    GetLogicExecutor()->Post(
        [function, callback = std::move(localCallback), taskNameTmp = taskName]() mutable {
            std::string message;
            try
            {
                function();
            }
            catch (const std::exception& e)
            {
                message = "ThreadEntry::Async task raise exception: " + std::string(e.what()) + "\n";
                std::cout << message;
            }
            catch (...)
            {
                message = "ThreadEntry::Async task raise exception: unknown\n";
                std::cout << message;
            }

            GetCallbackExecutor()->Post(
                [callback = std::move(callback), message] {
                    if (callback)
                    {
                        callback(message);
                    }
                },
                taskNameTmp);
        },
        taskName);
}

template<typename Fn, typename std::enable_if<!std::is_void<typename std::result_of<Fn()>::type>::value, int>::type>
typename std::result_of<Fn()>::type ThreadEntry::Await(Fn&& function, const ExecutorPtr& executor,
                                                       const std::string& taskName)
{
    const auto promise = std::make_shared<boost::fibers::promise<typename std::result_of<Fn()>::type>>();
    executor->Post(
        [function, promise] {
            try
            {
                promise->set_value(function());
            }
            catch (const std::exception& e)
            {
                std::string str = "ThreadEntry::Await task raise exception: " + std::string(e.what()) + "\n";
                std::cout << str;
                promise->set_exception(std::current_exception());
            }
            catch (...)
            {
                std::string str = "ThreadEntry::Await task raise exception: unknown\n";
                std::cout << str;
                promise->set_exception(std::current_exception());
            }
        },
        taskName);
    return promise->get_future().get();
}

template<typename Fn, typename std::enable_if<std::is_void<typename std::result_of<Fn()>::type>::value, int>::type>
typename std::result_of<Fn()>::type ThreadEntry::Await(Fn&& function, const ExecutorPtr& executor,
                                                       const std::string& taskName)
{
    const auto promise = std::make_shared<boost::fibers::promise<void>>();
    executor->Post(
        [function, promise] {
            try
            {
                function();
                promise->set_value();
            }
            catch (const std::exception& e)
            {
                std::string str = "ThreadEntry::Await task raise exception: " + std::string(e.what()) + "\n";
                std::cout << str;
                promise->set_exception(std::current_exception());
            }
            catch (...)
            {
                std::string str = "ThreadEntry::Await task raise exception: unknown\n";
                std::cout << str;
                promise->set_exception(std::current_exception());
            }
        },
        taskName);
    promise->get_future().get();
}
} /* namespace threading */
