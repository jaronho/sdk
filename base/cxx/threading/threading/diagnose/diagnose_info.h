#pragma once
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>

#include "../task/executor.h"
#include "../timer/timer.h"

namespace threading
{
namespace diagnose
{
/**
 * @brief 任务信息
 */
struct TaskInfo
{
    TaskInfo(const Task* task) : task(task) {}
    const Task* task;
    std::chrono::steady_clock::time_point queuing{}; /* 开始排队时间点 */
    std::chrono::steady_clock::time_point running{}; /* 开始运行时间点 */
    std::chrono::steady_clock::time_point finished{}; /* 运行结束时间点 */
    std::chrono::steady_clock::time_point abnormal{}; /* 出现异常时间点 */
    std::string attachExecutorName; /* 挂靠的执行者名称 */
    int64_t attachThreadId = 0; /* 挂靠的线程id */
    std::string attachThreadName; /* 挂靠的线程名称 */
    std::string exceptionMsg; /* 异常消息 */
};
using TaskInfoPtr = std::shared_ptr<TaskInfo>;

/**
 * @brief 执行者信息
 */
struct ExecutorInfo
{
    ExecutorInfo(const Executor* executor) : executor(executor) {}
    const Executor* executor;
    std::unordered_map<const Task*, TaskInfoPtr> tasks; /* 任务列表 */
};
using ExecutorInfoPtr = std::unique_ptr<ExecutorInfo>;

/**
 * @brief 定时器触发信息
 */
struct TimerTriggerInfo
{
    TimerTriggerInfo(const Timer* timer) : timer(timer) {}
    const Timer* timer;
    std::chrono::steady_clock::time_point queuing{}; /* 触发排队时间点 */
    std::chrono::steady_clock::time_point running{}; /* 触发执行时间点 */
    std::chrono::steady_clock::time_point finished{}; /* 执行结束时间点 */
    std::chrono::steady_clock::time_point abnormal{}; /* 出现异常时间点 */
    std::string exceptionMsg; /* 异常消息 */
};
using TimerTriggerInfoPtr = std::shared_ptr<TimerTriggerInfo>;
} // namespace diagnose
} // namespace threading
