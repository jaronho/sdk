#pragma once
#include <boost/asio.hpp>
#include <functional>
#include <memory>

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
     */
    Timer();

    virtual ~Timer() = default;

    /**
     * @brief 是否已启动
     * @return true-已启动, false-未启动
     */
    virtual bool isStarted() = 0;

    /**
     * @brief 启动
     */
    virtual void start() = 0;

    /**
     * @brief 停止
     */
    virtual void stop() = 0;

    /**
     * @brief 运行单次(用于监听触发回调, 在主逻辑线程中循环调用, 一般是在主线程)
     *        注意: 如果定时器有指定任务触发回调的执行线程, 则其回调不受该接口接管
     */
    static void runOnce();

protected:
    /**
     * @brief 获取I/O上下文(用于运行定时器)
     */
    boost::asio::io_context& getContext();

    /**
     * @brief 添加到触发列表
     * @param func 触发函数
     */
    void addToTriggerList(const std::function<void()>& func);
};

using TimerPtr = std::shared_ptr<Timer>;
} // namespace threading