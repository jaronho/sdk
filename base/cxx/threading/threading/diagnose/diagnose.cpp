#include "diagnose.h"

#include <atomic>
#include <iostream>
#include <mutex>
#include <unordered_map>

#include "diagnose_info.h"

namespace threading
{
static std::atomic_bool s_enabled = {false}; /* 是否开启诊断功能(默认关闭) */
static std::mutex s_mutexTask;
static std::unordered_map<const Executor*, diagnose::ExecutorInfoPtr> s_taskExecutors; /* 全局任务执行者 */
static TaskBindCallback s_taskBindCallback = nullptr; /* 任务绑定回调 */
static TaskNormalStateCallback s_taskRunningStateCallback = nullptr; /* 任务运行状态回调 */
static TaskNormalStateCallback s_taskFinishedStateCallback = nullptr; /* 任务结束状态回调 */
static TaskExceptionStateCallback s_taskExceptionStateCallback = nullptr; /* 任务异常状态回调 */
static std::mutex s_mutexTimer;
static std::unordered_map<int64_t, diagnose::TimerTriggerInfoPtr> s_timerTriggers; /* 定时器触发表 */
static TimerTriggerCallback s_timerTriggerCallback = nullptr; /* 定时器触发回调 */
static TimerTriggerNormalStateCallback s_timerTriggerRunningStateCallback = nullptr; /* 定时器触发运行状态回调 */
static TimerTriggerNormalStateCallback s_timerTriggerFinishedStateCallback = nullptr; /* 定时器触发结束状态回调 */
static TimerTriggerExceptionStateCallback s_timerTriggerExceptionStateCallback = nullptr; /* 定时器触发异常状态回调 */

static diagnose::TaskInfoPtr getTaskInfo(const Task* task)
{
    if (!task)
    {
        return {};
    }
    for (const auto& iter : s_taskExecutors)
    {
        auto taskIter = iter.second->tasks.find(task);
        if (iter.second->tasks.end() != taskIter)
        {
            return taskIter->second;
        }
    }
    return {};
}

static void delTaskInfo(const Task* task)
{
    if (!task)
    {
        return;
    }
    for (const auto& iter : s_taskExecutors)
    {
        auto taskIter = iter.second->tasks.find(task);
        if (iter.second->tasks.end() != taskIter)
        {
            iter.second->tasks.erase(taskIter);
            return;
        }
    }
}

static std::string taskStateToString(const Task::State& state)
{
    switch (state)
    {
    case Task::State::created:
        return "created";
    case Task::State::queuing:
        return "queuing";
    case Task::State::running:
        return "running";
    case Task::State::finished:
        return "finished";
    default:
        break;
    }
    return std::string();
}

static std::string timerTriggerStateToString(const diagnose::TimerTriggerState& state)
{
    switch (state)
    {
    case diagnose::TimerTriggerState::triggered:
        return "triggered";
    case diagnose::TimerTriggerState::queuing:
        return "queuing";
    case diagnose::TimerTriggerState::running:
        return "running";
    case diagnose::TimerTriggerState::finished:
        return "finished";
    default:
        break;
    }
    return std::string();
}

static std::string durationToString(const std::chrono::steady_clock::duration& duration)
{
    using namespace std::chrono_literals;
    if (duration < 1us)
    {
        return std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) + " ns";
    }
    if (duration < 1ms)
    {
        return std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(duration).count()) + " us";
    }
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) + " ms";
}

static std::string taskInfoToString(const diagnose::TaskInfoPtr& taskInfo, bool showRun)
{
    static const std::chrono::steady_clock::time_point zero{};
    std::string result;
    result += "{";
    result += "\"id\":" + std::to_string(taskInfo->task->getId());
    result += ",";
    result += "\"name\":\"" + taskInfo->task->getName() + "\"";
    result += ",";
    result += "\"thread_id\":" + std::to_string(taskInfo->attachThreadId);
    result += ",";
    result += "\"thread_name\":\"" + taskInfo->attachThreadName + "\"";
    result += ",";
    result += "\"state\":\"" + (taskInfo->exceptionMsg.empty() ? taskStateToString(taskInfo->task->getState()) : "abnormal") + "\"";
    if (!taskInfo->exceptionMsg.empty())
    {
        result += ",";
        result += "\"error\":\"" + taskInfo->exceptionMsg + "\"";
    }
    auto now = std::chrono::steady_clock::now();
    /* 排队耗时 */
    if (taskInfo->queuing > zero)
    {
        result += ",";
        result += "\"queue\":\"" + durationToString((taskInfo->running > zero ? taskInfo->running : now) - taskInfo->queuing) + "\"";
    }
    /* 运行耗时 */
    if (showRun && taskInfo->running > zero)
    {
        result += ",";
        if (taskInfo->finished > zero)
        {
            result += "\"run\":\"" + durationToString(taskInfo->finished - taskInfo->running) + "\"";
        }
        else if (taskInfo->abnormal > zero)
        {
            result += "\"run\":\"" + durationToString(taskInfo->abnormal - taskInfo->running) + "\"";
        }
        else
        {
            result += "\"run\":\"" + durationToString(now - taskInfo->running) + "\"";
        }
    }
    result += "}";
    return result;
}

static std::string executorInfoToString(const diagnose::ExecutorInfoPtr& executorInfo)
{
    std::string result;
    result += "{";
    result += "\"name\":\"" + executorInfo->executor->getName() + "\"";
    result += ",";
    result += "\"count\":" + std::to_string(executorInfo->tasks.size());
    result += ",";
    result += "\"task\":[";
    for (auto iter = executorInfo->tasks.begin(); executorInfo->tasks.end() != iter; ++iter)
    {
        if (executorInfo->tasks.begin() != iter)
        {
            result += ",";
        }
        result += taskInfoToString(iter->second, true);
    }
    result += "]";
    result += "}";
    return result;
}

static std::string timerTriggerInfoToString(const diagnose::TimerTriggerInfoPtr& timerTriggerInfo, bool showRun)
{
    static const std::chrono::steady_clock::time_point zero{};
    std::string result;
    result += "{";
    result += "\"id\":" + std::to_string(timerTriggerInfo->timer->getId());
    result += ",";
    result += "\"name\":\"" + timerTriggerInfo->timer->getName() + "\"";
    result += ",";
    result +=
        "\"state\":\"" + (timerTriggerInfo->exceptionMsg.empty() ? timerTriggerStateToString(timerTriggerInfo->state) : "abnormal") + "\"";
    if (!timerTriggerInfo->exceptionMsg.empty())
    {
        result += ",";
        result += "\"error\":\"" + timerTriggerInfo->exceptionMsg + "\"";
    }
    auto now = std::chrono::steady_clock::now();
    /* 排队耗时 */
    if (timerTriggerInfo->queuing > zero)
    {
        result += ",";
        result += "\"queue\":\""
                  + durationToString((timerTriggerInfo->running > zero ? timerTriggerInfo->running : now) - timerTriggerInfo->queuing)
                  + "\"";
    }
    /* 运行耗时 */
    if (showRun && timerTriggerInfo->running > zero)
    {
        result += ",";
        if (timerTriggerInfo->finished > zero)
        {
            result += "\"run\":\"" + durationToString(timerTriggerInfo->finished - timerTriggerInfo->running) + "\"";
        }
        else if (timerTriggerInfo->abnormal > zero)
        {
            result += "\"run\":\"" + durationToString(timerTriggerInfo->abnormal - timerTriggerInfo->running) + "\"";
        }
        else
        {
            result += "\"run\":\"" + durationToString(now - timerTriggerInfo->running) + "\"";
        }
    }
    result += "}";
    return result;
}

void Diagnose::onExecutorCreated(const Executor* executor)
{
    if (!s_enabled)
    {
        return;
    }
    if (!executor)
    {
        return;
    }
    std::lock_guard<std::mutex> locker(s_mutexTask);
    const auto iter = s_taskExecutors.find(executor);
    if (s_taskExecutors.end() != iter)
    {
        return;
    }
    auto executorInfo = std::make_unique<diagnose::ExecutorInfo>(executor);
    s_taskExecutors.insert(std::make_pair(executor, std::move(executorInfo)));
}

void Diagnose::onExecutorDestroyed(const Executor* executor)
{
    if (!s_enabled)
    {
        return;
    }
    if (!executor)
    {
        return;
    }
    std::lock_guard<std::mutex> locker(s_mutexTask);
    const auto iter = s_taskExecutors.find(executor);
    if (s_taskExecutors.end() == iter)
    {
        return;
    }
    iter->second->tasks.clear();
    s_taskExecutors.erase(iter);
}

void Diagnose::bindTaskToExecutor(const Task* task, const Executor* executor)
{
    if (!s_enabled)
    {
        return;
    }
    if (!task || !executor)
    {
        return;
    }
    TaskBindCallback bindCallback;
    std::string executorName;
    int taskCount = 0;
    int64_t taskId = 0;
    std::string taskName;
    {
        std::lock_guard<std::mutex> locker(s_mutexTask);
        const auto iter = s_taskExecutors.find(executor);
        if (s_taskExecutors.end() == iter)
        {
            return;
        }
        if (iter->second->tasks.end() != iter->second->tasks.find(task))
        {
            return;
        }
        auto taskInfo = std::make_shared<diagnose::TaskInfo>(task);
        taskInfo->attachExecutorName = executor->getName();
        taskInfo->queuing = std::chrono::steady_clock::now();
        iter->second->tasks.insert(std::make_pair(task, taskInfo));
        bindCallback = s_taskBindCallback;
        executorName = executor->getName();
        taskCount = iter->second->tasks.size();
        taskId = task->getId();
        taskName = task->getName();
    }
    if (bindCallback)
    {
        bindCallback(executorName, taskCount, taskId, taskName);
    }
}

void Diagnose::onTaskRunning(int threadId, const std::string& threadName, const Task* task)
{
    if (!s_enabled)
    {
        return;
    }
    TaskNormalStateCallback runningCallback;
    std::string executorName;
    int64_t taskId = 0;
    std::string taskName;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTask);
        auto taskInfo = getTaskInfo(task);
        if (!taskInfo)
        {
            return;
        }
        taskInfo->running = std::chrono::steady_clock::now();
        taskInfo->attachThreadId = threadId;
        taskInfo->attachThreadName = threadName;
        runningCallback = s_taskRunningStateCallback;
        executorName = taskInfo->attachExecutorName;
        taskId = task->getId();
        prevElapsed = taskInfo->running - taskInfo->queuing;
    }
    if (runningCallback)
    {
        runningCallback(executorName, threadId, threadName, taskId, taskName, prevElapsed);
    }
}

void Diagnose::onTaskFinished(int threadId, const std::string& threadName, const Task* task)
{
    if (!s_enabled)
    {
        return;
    }
    TaskNormalStateCallback finishedCallback;
    std::string executorName;
    int64_t taskId = 0;
    std::string taskName;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTask);
        auto taskInfo = getTaskInfo(task);
        if (!taskInfo)
        {
            return;
        }
        taskInfo->finished = std::chrono::steady_clock::now();
        taskInfo->attachThreadId = threadId;
        taskInfo->attachThreadName = threadName;
        finishedCallback = s_taskFinishedStateCallback;
        executorName = taskInfo->attachExecutorName;
        taskId = task->getId();
        prevElapsed = taskInfo->finished - taskInfo->running;
        delTaskInfo(task);
    }
    if (finishedCallback)
    {
        finishedCallback(executorName, threadId, threadName, taskId, taskName, prevElapsed);
    }
}

void Diagnose::onTaskException(int threadId, const std::string& threadName, const Task* task, const std::string& msg)
{
    if (!s_enabled)
    {
        return;
    }
    TaskExceptionStateCallback exceptionCallback;
    std::string executorName;
    int64_t taskId = 0;
    std::string taskName;
    {
        std::lock_guard<std::mutex> locker(s_mutexTask);
        auto taskInfo = getTaskInfo(task);
        if (!taskInfo)
        {
            return;
        }
        taskInfo->abnormal = std::chrono::steady_clock::now();
        taskInfo->attachThreadId = threadId;
        taskInfo->attachThreadName = threadName;
        taskInfo->exceptionMsg = msg;
        exceptionCallback = s_taskExceptionStateCallback;
        executorName = taskInfo->attachExecutorName;
        taskId = task->getId();
        delTaskInfo(task);
    }
    if (exceptionCallback)
    {
        exceptionCallback(executorName, threadId, threadName, taskId, taskName, msg);
    }
}

TimerTriggerDiscard Diagnose::onTimerTrigger(int triggerCount, int64_t oldestTriggerId, int64_t triggerId, const Timer* timer)
{
    if (!s_enabled)
    {
        return TimerTriggerDiscard::none;
    }
    TimerTriggerCallback triggerCallback;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        triggerCallback = s_timerTriggerCallback;
    }
    auto discardType = triggerCallback ? triggerCallback(triggerCount, timer->getId(), timer->getName()) : TimerTriggerDiscard::none;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        switch (discardType)
        {
        case TimerTriggerDiscard::discard_newest: /* 丢弃最新 */
            return discardType;
        case TimerTriggerDiscard::discard_oldest: /* 丢弃最早 */
        {
            auto iter = s_timerTriggers.find(oldestTriggerId);
            if (s_timerTriggers.end() != iter)
            {
                s_timerTriggers.erase(iter);
            }
        }
        break;
        case TimerTriggerDiscard::discard_all: /* 丢弃所有 */
            s_timerTriggers.clear();
            break;
        default: /* 不丢弃(可能会内存持续上涨) */
            break;
        }
        auto timerTriggerInfo = std::make_shared<diagnose::TimerTriggerInfo>(timer);
        timerTriggerInfo->state = diagnose::TimerTriggerState::queuing;
        timerTriggerInfo->queuing = std::chrono::steady_clock::now();
        s_timerTriggers.insert(std::make_pair(triggerId, timerTriggerInfo));
    }
    return discardType;
}

void Diagnose::onTimerTriggerRunning(int64_t triggerId, const Timer* timer)
{
    if (!s_enabled)
    {
        return;
    }
    TimerTriggerNormalStateCallback runningCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        auto iter = s_timerTriggers.find(triggerId);
        if (s_timerTriggers.end() == iter)
        {
            return;
        }
        iter->second->state = diagnose::TimerTriggerState::running;
        iter->second->running = std::chrono::steady_clock::now();
        runningCallback = s_timerTriggerRunningStateCallback;
        prevElapsed = iter->second->running - iter->second->queuing;
    }
    if (runningCallback)
    {
        runningCallback(timer->getId(), timer->getName(), prevElapsed);
    }
}

void Diagnose::onTimerTriggerFinished(int64_t triggerId, const Timer* timer)
{
    if (!s_enabled)
    {
        return;
    }
    TimerTriggerNormalStateCallback finishedCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        auto iter = s_timerTriggers.find(triggerId);
        if (s_timerTriggers.end() == iter)
        {
            return;
        }
        iter->second->state = diagnose::TimerTriggerState::finished;
        iter->second->finished = std::chrono::steady_clock::now();
        finishedCallback = s_timerTriggerFinishedStateCallback;
        prevElapsed = iter->second->finished - iter->second->running;
        s_timerTriggers.erase(iter);
    }
    if (finishedCallback)
    {
        finishedCallback(timer->getId(), timer->getName(), prevElapsed);
    }
}

void Diagnose::onTimerTriggerException(int64_t triggerId, const Timer* timer, const std::string& msg)
{
    if (!s_enabled)
    {
        return;
    }
    TimerTriggerExceptionStateCallback exceptionCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        auto iter = s_timerTriggers.find(triggerId);
        if (s_timerTriggers.end() == iter)
        {
            return;
        }
        iter->second->state = diagnose::TimerTriggerState::finished;
        iter->second->abnormal = std::chrono::steady_clock::now();
        iter->second->exceptionMsg = msg;
        exceptionCallback = s_timerTriggerExceptionStateCallback;
        s_timerTriggers.erase(iter);
    }
    if (exceptionCallback)
    {
        exceptionCallback(timer->getId(), timer->getName(), msg);
    }
}

void Diagnose::onTimerDestroy(const Timer* timer)
{
    if (!s_enabled)
    {
        return;
    }
    std::lock_guard<std::mutex> locker(s_mutexTimer);
    for (auto iter = s_timerTriggers.begin(); s_timerTriggers.end() != iter;)
    {
        if (timer == iter->second->timer)
        {
            s_timerTriggers.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void Diagnose::setEnable()
{
    s_enabled = true;
}

void Diagnose::setTaskBindCallback(const TaskBindCallback& taskBindCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTask);
    s_taskBindCallback = taskBindCb;
}

void Diagnose::setTaskRunningStateCallback(const TaskNormalStateCallback& taskStateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTask);
    s_taskRunningStateCallback = taskStateCb;
}

void Diagnose::setTaskFinishedStateCallback(const TaskNormalStateCallback& taskStateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTask);
    s_taskFinishedStateCallback = taskStateCb;
}

void Diagnose::setTaskExceptionStateCallback(const TaskExceptionStateCallback& taskStateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTask);
    s_taskExceptionStateCallback = taskStateCb;
}

void Diagnose::setTimerTriggerCallback(const TimerTriggerCallback& timerTriggerCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTimer);
    s_timerTriggerCallback = timerTriggerCb;
}

void Diagnose::setTimerTriggerRunningStateCallback(const TimerTriggerNormalStateCallback& timerTriggerStateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTimer);
    s_timerTriggerRunningStateCallback = timerTriggerStateCb;
}

void Diagnose::setTimerTriggerFinishedStateCallback(const TimerTriggerNormalStateCallback& timerTriggerStateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTimer);
    s_timerTriggerFinishedStateCallback = timerTriggerStateCb;
}

void Diagnose::setTimerTriggerExceptionStateCallback(const TimerTriggerExceptionStateCallback& timerTriggerStateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTimer);
    s_timerTriggerExceptionStateCallback = timerTriggerStateCb;
}

std::string Diagnose::getTaskDiagnoseInfo()
{
    std::string result;
    result += "[";
    {
        std::lock_guard<std::mutex> locker(s_mutexTask);
        for (auto iter = s_taskExecutors.begin(); s_taskExecutors.end() != iter; ++iter)
        {
            if (s_taskExecutors.begin() != iter)
            {
                result += ",";
            }
            result += executorInfoToString(iter->second);
        }
    }
    result += "]";
    return result;
}

std::string Diagnose::getTimerDiagnoseInfo()
{
    std::string result;
    result += "[";
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        for (auto iter = s_timerTriggers.begin(); s_timerTriggers.end() != iter; ++iter)
        {
            if (s_timerTriggers.begin() != iter)
            {
                result += ",";
            }
            result += timerTriggerInfoToString(iter->second, true);
        }
    }
    result += "]";
    return result;
}
} // namespace threading
