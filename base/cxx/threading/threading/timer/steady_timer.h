#pragma once
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

#include "../asio/asio_executor.h"
#include "timer.h"

namespace threading
{
/**
 * @brief 稳定定时器, 可以保证以固定时间间隔触发, 不受系统时间影响
 */
class SteadyTimer final : public Timer, public std::enable_shared_from_this<SteadyTimer>
{
public:
    /**
     * @brief 构造函数
     * @param delay 首次触发的延时, <=0表示马上触发
     * @param interval 触发间隔, <=0表示不重复触发
     * @param name 名称
     * @param func 回调
     * @param executor 指定回调的执行器(选填), 当为空时, 回调会被`timer_proxy`的`runOnce`接管
     */
    SteadyTimer(const std::chrono::steady_clock::duration& delay, const std::chrono::steady_clock::duration& interval,
                const std::string& name, const std::function<void()>& func, const ExecutorPtr& executor = nullptr);

    virtual ~SteadyTimer();

    /**
     * @brief 设置首次触发的延时, 定时器运行过程中设置则下次启动`start`定时器生效
     * @param delay 首次触发的延时, 小于或等于0表示马上触发
     */
    void setDelay(const std::chrono::steady_clock::duration& delay);

    /**
     * @brief 设置触发间隔, 定时器运行过程中设置, 则下次触发后生效
     * @param interval 触发间隔, 小于或等于0表示不重复触发
     */
    void setInterval(const std::chrono::steady_clock::duration& interval);

    /**
	 * @brief 获取名称
	 * @return 定时器名称
	 */
    std::string getName() const override;

    /**
     * @brief 是否已启动
     * @return true-已启动, false-未启动
     */
    bool isStarted() override;

    /**
     * @brief 启动
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

private:
    std::recursive_mutex m_mutex;
    std::chrono::steady_clock::duration m_delay; /* 首次触发延迟时间 */
    std::chrono::steady_clock::duration m_interval; /* 定时器间隔 */
    std::string m_name; /* 定时器名称 */
    std::function<void()> m_func; /* 触发执行函数 */
    ExecutorPtr m_executor; /* 指定线程(执行者) */
    bool m_started; /* 是否已启动 */
    std::shared_ptr<boost::asio::steady_timer> m_timer; /* boost稳定定时器 */
};

using SteadyTimerPtr = std::shared_ptr<SteadyTimer>;
} // namespace threading
