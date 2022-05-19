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
 * @brief 任务诊断信息
 */
struct DiagTask
{
    DiagTask(const Task* task) : task(task) {}
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
using DiagTaskPtr = std::shared_ptr<DiagTask>;

/**
 * @brief 执行者诊断信息
 */
struct DiagExecutor
{
    DiagExecutor(const Executor* executor) : executor(executor) {}
    const Executor* executor;
    std::unordered_map<const Task*, DiagTaskPtr> taskList; /* 任务列表 */
};
using DiagExecutorPtr = std::unique_ptr<DiagExecutor>;

/**
 * @brief 触发器状态
 */
enum class TriggerState
{
    created, /* 已创建 */
    queuing, /* 排队中 */
    running, /* 运行中 */
    finished /* 已完成 */
};

/**
 * @brief 触发器诊断信息
 */
struct DiagTrigger
{
    DiagTrigger(const Timer* timer) : timer(timer) {}
    const Timer* timer;
    TriggerState state = TriggerState::created; /* 触发状态 */
    std::chrono::steady_clock::time_point queuing{}; /* 开始排队时间点 */
    std::chrono::steady_clock::time_point running{}; /* 开始运行时间点 */
    std::chrono::steady_clock::time_point finished{}; /* 运行结束时间点 */
    std::chrono::steady_clock::time_point abnormal{}; /* 出现异常时间点 */
    std::string exceptionMsg; /* 异常消息 */
};
using DiagTriggerPtr = std::shared_ptr<DiagTrigger>;
} // namespace diagnose
} // namespace threading
