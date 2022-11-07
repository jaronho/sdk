#pragma once
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

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
     * @param name 名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param delay 首次触发的延时, <=0表示马上触发
     * @param interval 触发间隔, <=0表示不重复触发
     * @param func 回调
     * @param executor 指定回调的执行器(选填), 当为空时, 回调会被`timer_proxy`的`tryOnce`或`waitOnce`接管
     */
    SteadyTimer(const std::string& name, const std::chrono::steady_clock::duration& delay,
                const std::chrono::steady_clock::duration& interval, const std::function<void()>& func,
                const ExecutorPtr& executor = nullptr);

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
     * @brief 启动
     */
    void start() override;

    /**
     * @brief 停止
     */
    void stop() override;

    /**
     * @brief 创建单次定时器
     * @param name 名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param interval 触发间隔(后续如果要改变间隔, 需要调用`setDelay`进行设置)
     * @param func 回调
     * @param executor 指定回调的执行器(选填), 当为空时, 回调会被`timer_proxy`的`tryOnce`或`waitOnce`接管
     * @return 单次执行定时器
     */
    static std::shared_ptr<SteadyTimer> onceTimer(const std::string& name, const std::chrono::steady_clock::duration& interval,
                                                  const std::function<void()>& func, const ExecutorPtr& executor = nullptr);

    /**
     * @brief 创建循环定时器
     * @param name 名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param interval 触发间隔(后续如果要改变间隔, 需要调用`setInterval`进行设置)
     * @param func 回调
     * @param executor 指定回调的执行器(选填), 当为空时, 回调会被`timer_proxy`的`tryOnce`或`waitOnce`接管
     * @return 单次执行定时器
     */
    static std::shared_ptr<SteadyTimer> loopTimer(const std::string& name, const std::chrono::steady_clock::duration& interval,
                                                  const std::function<void()>& func, const ExecutorPtr& executor = nullptr);

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
    std::shared_ptr<boost::asio::steady_timer> m_timer; /* boost稳定定时器 */
};

using SteadyTimerPtr = std::shared_ptr<SteadyTimer>;
} // namespace threading
