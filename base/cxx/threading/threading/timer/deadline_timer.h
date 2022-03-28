#pragma once
#include <boost/asio/system_timer.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

#include "../asio/asio_executor.h"
#include "timer.h"

namespace threading
{
/**
 * @brief 截止时间定时器, 指定在特定系统时间触发, 修改系统时间可能受到影响
 */
class DeadlineTimer final : public Timer, public std::enable_shared_from_this<DeadlineTimer>
{
public:
    /**
     * @brief 构造函数
     * @param deadline 触发时间
     * @param name 名称
     * @param func 回调
     * @param executor 指定回调的执行器(选填), 当为空时, 回调会被`timer_proxy`的`runOnce`接管
     */
    DeadlineTimer(const std::chrono::system_clock::time_point& deadline, const std::string& name, const std::function<void()>& func,
                  const ExecutorPtr& executor = nullptr);

    virtual ~DeadlineTimer();

    /**
     * @brief 设置触发时间, 定时器运行过程中设置则下次启动`start`定时器生效
     * @param deadline 触发时间
     */
    void setDeadline(const std::chrono::system_clock::time_point& deadline);

    /**
     * @brief 是否已启动
     * @return true-已启动, false-未启动
     */
    bool isStarted() override;

    /**
     * @brief 启动, 说明: 尽量不要在`stop`后立即调用
     */
    void start() override;

    /**
     * @brief 停止
     */
    void stop() override;

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
    std::recursive_mutex m_mutex;
    std::atomic<std::chrono::system_clock::time_point> m_deadline; /* 触发时间点 */
    std::string m_name; /* 定时器名称 */
    std::function<void()> m_func; /* 触发执行函数 */
    ExecutorPtr m_executor; /* 指定线程(执行者) */
    bool m_started; /* 是否已启动 */
    std::shared_ptr<boost::asio::system_timer> m_timer; /* boost截至时间定时器 */
};

using DeadlineTimerPtr = std::shared_ptr<DeadlineTimer>;
} // namespace threading
