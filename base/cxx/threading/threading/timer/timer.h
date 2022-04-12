#pragma once
#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <mutex>

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
     * @brief 运行单次(用于监听触发回调, 在主逻辑线程中循环调用, 一般是在主线程), 调用频率建议不超过1秒
     *        注意: 如果定时器有指定任务触发回调的执行线程, 则其回调不受该接口接管
     */
    static void runOnce();

protected:
    /**
     * @brief 触发信息
     */
    struct TriggerInfo
    {
        std::string name;
        std::weak_ptr<Timer> wpTimer;
        std::function<void()> func = nullptr;
    };

protected:
    /**
     * @brief 获取I/O上下文(用于运行定时器)
     */
    boost::asio::io_context& getContext();

    /**
     * @brief 添加到触发列表
     * @param info 触发信息
     */
    void addToTriggerList(const TriggerInfo& info);

private:
    static std::mutex s_mutex;
    static std::unique_ptr<boost::asio::io_context> s_context;
    static std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> s_work;
    static std::unique_ptr<std::thread> s_thread;
    static std::mutex s_mutexTrigger;
    static std::list<TriggerInfo> s_triggerList; /* 定时器触发列表 */
};

using TimerPtr = std::shared_ptr<Timer>;
} // namespace threading
