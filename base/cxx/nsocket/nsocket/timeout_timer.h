#pragma once
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <memory>
#include <mutex>

namespace nsocket
{
/**
 * @brief 超时定时器, 可以保证以固定时间间隔触发, 不受系统时间影响
 */
class TimeoutTimer final : public std::enable_shared_from_this<TimeoutTimer>
{
public:
    /**
     * @brief 构造函数
     * @param timeout 超时时间
     * @param func 超时回调
     */
    TimeoutTimer(const std::chrono::steady_clock::duration& timeout, const std::function<void()>& func);

    virtual ~TimeoutTimer();

    /**
     * @brief 是否已启动
     * @return true-已启动, false-未启动
     */
    bool isStarted();

    /**
     * @brief 启动, 说明: 尽量不要在`stop`后立即调用
     */
    void start();

    /**
     * @brief 停止
     */
    void stop();

private:
    /**
     * @brief 触发恢复
     */
    void onRecover();

    /**
     * @brief 定时器触发
     */
    void onTrigger();

    /**
     * @brief 触发停止
     */
    void onStop();

private:
    std::mutex m_mutex;
    std::chrono::steady_clock::duration m_timeout; /* 超时时间 */
    std::function<void()> m_func; /* 超时回调 */
    bool m_started; /* 是否已启动 */
    std::shared_ptr<boost::asio::steady_timer> m_timer; /* boost截至时间定时器 */
};
} // namespace nsocket
