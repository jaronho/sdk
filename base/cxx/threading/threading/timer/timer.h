#pragma once
#include <atomic>
#include <boost/asio.hpp>
#include <functional>
#include <memory>

#include "../task/executor.h"

namespace threading
{
/**
 * @brief 定时器基类
 */
class Timer
{
public:
    /**
     * @brief 构造函数
     * @param name 名称
     * @param func 回调
     * @param executor 指定回调的执行器(选填), 当为空时, 回调会被`timer_proxy`的`runOnce`接管
     */
    Timer(const std::string& name, const std::function<void()>& func, const ExecutorPtr& executor = nullptr);

    virtual ~Timer();

    /**
     * @brief 获取ID
     * @return 定时器ID
     */
    int64_t getId() const;

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
     * @brief 运行单次(用于监听触发回调, 在主逻辑线程中循环调用, 一般是在主线程), 调用频率建议不超过1秒
     *        注意: 如果定时器有指定任务触发回调的执行线程, 则其回调不受该接口接管
     */
    static void runOnce();

protected:
    /**
     * @brief 获取I/O上下文(用于运行定时器)
     */
    boost::asio::io_context& getContext();

    /**
     * @brief 添加到触发器列表
     * @param timer 定时器
     */
    void addToTriggerList(const std::shared_ptr<Timer>& timer);

protected:
    std::function<void()> m_func = nullptr; /* 触发执行函数 */
    ExecutorPtr m_executor; /* 指定线程(执行者) */
    std::atomic_bool m_started = {false}; /* 是否已启动 */

private:
    int64_t m_id = 0; /* ID */
    const std::string m_name; /* 定时器名称 */
};

using TimerPtr = std::shared_ptr<Timer>;
} // namespace threading
