#pragma once
#include <atomic>
#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "../task/executor.h"

namespace threading
{
/**
 * @brief 定时器触发函数
 * @param tp 触发时间点
 */
using TimerTriggerFunc = std::function<void(const std::chrono::steady_clock::time_point& tp)>;

/**
 * @brief 定时器执行器钩子
 * @param name 定时器名称
 * @param func 触发函数
 */
using TimerExecutorHook = std::function<void(const std::string& name, const std::function<void()>& func)>;

/**
 * @brief 定时器基类
 */
class Timer
{
public:
    /**
     * @brief 构造函数
     * @param name 名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param func 触发函数
     * @param executor 指定触发函数的执行器(选填), 当为空时将在默认执行器执行触发函数
     */
    Timer(const std::string& name, const TimerTriggerFunc& func, const ExecutorPtr& executor = nullptr);

    virtual ~Timer() = default;

    /**
	 * @brief 获取名称
	 * @return 定时器名称
	 */
    std::string getName() const;

    /**
     * @brief 是否已启动
     * @return true-已启动, false-未启动
     */
    bool isStarted() const;

    /**
     * @brief 启动
     */
    virtual void start() = 0;

    /**
     * @brief 停止
     */
    virtual void stop() = 0;

    /**
     * @brief 设置默认触发函数执行器
     * @param executor 执行器
     * @param hook 执行器构子(选填), 为空时直接执行触发函数
     */
    static void setDefaultExecutor(const ExecutorPtr& executor, const TimerExecutorHook& hook = nullptr);

protected:
    /**
     * @brief 获取I/O上下文(用于运行定时器)
     */
    boost::asio::io_context& getContext();

    /**
     * @brief 响应触发函数
     * @param timer 定时器
     */
    void onTriggerFunc(const std::shared_ptr<Timer>& timer);

protected:
    std::atomic_bool m_started = {false}; /* 是否已启动 */

private:
    const std::string m_name; /* 定时器名称 */
    TimerTriggerFunc m_func = nullptr; /* 触发函数 */
    ExecutorPtr m_executor = nullptr; /* 指定线程(执行者) */
};

using TimerPtr = std::shared_ptr<Timer>;
} // namespace threading
