#pragma once

#include <list>
#include <mutex>

#include "thread_proxy.hpp"

namespace threading
{
/**
 * @brief 定时器代理
 */
class TimerProxy final
{
    friend class SteadyTimer;
    friend class DeadlineTimer;

public:
    /**
     * @brief 启动模块
     */
    static void start();

    /**
     * @brief 停止模块
     */
    static void stop();

    /**
     * @brief 运行单次(用于监听触发回调, 在主逻辑线程中循环调用, 一般是在主线程)
     *        注意: 如果定时器有指定任务触发回调的执行线程, 则其回调不受该接口接管
     */
    static void runOnce();

protected:
    /**
     * @brief 获取执行者I/O上下文
     * @return 执行者I/O上下文
     */
    static boost::asio::io_context* getContext();

    /**
     * @brief 添加到触发列表
     * @param func 触发函数
     */
    static void addToTriggerList(const std::function<void()>& func);

private:
    static std::shared_ptr<AsioExecutor> s_timerThread; /* 定时器线程 */
    static std::mutex s_triggerMutex; /* 互斥量 */
    static std::list<std::function<void()>> s_triggerList; /* 触发列表 */
};
} // namespace threading
