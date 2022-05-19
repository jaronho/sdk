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
static std::unordered_map<const Executor*, diagnose::DiagExecutorPtr> s_executorList; /* 执行者(线程池)列表 */
static TaskBindCallback s_taskBindedCallback = nullptr; /* 任务绑定回调 */
static TaskNormalStateCallback s_taskRunningStateCallback = nullptr; /* 任务运行状态回调 */
static TaskNormalStateCallback s_taskFinishedStateCallback = nullptr; /* 任务结束状态回调 */
static TaskExceptionStateCallback s_taskExceptionStateCallback = nullptr; /* 任务异常状态回调 */
static std::mutex s_mutexTimer;
static std::unordered_map<int64_t, diagnose::DiagTriggerPtr> s_triggerList; /* 触发器列表 */
static TriggerCreateCallback s_triggerCreatedCallback = nullptr; /* 触发器创建回调 */
static TriggerNormalStateCallback s_triggerRunningStateCallback = nullptr; /* 触发器运行状态回调 */
static TriggerNormalStateCallback s_triggerFinishedStateCallback = nullptr; /* 触发器结束状态回调 */
static TriggerExceptionStateCallback s_triggerExceptionStateCallback = nullptr; /* 触发器异常状态回调 */

static diagnose::DiagTaskPtr getDiagTask(const Task* task)
{
    if (!task)
    {
        return {};
    }
    for (const auto& iter : s_executorList)
    {
        auto taskIter = iter.second->taskList.find(task);
        if (iter.second->taskList.end() != taskIter)
        {
            return taskIter->second;
        }
    }
    return {};
}

static void delDiagTask(const Task* task)
{
    if (!task)
    {
        return;
    }
    for (const auto& iter : s_executorList)
    {
        auto taskIter = iter.second->taskList.find(task);
        if (iter.second->taskList.end() != taskIter)
        {
            iter.second->taskList.erase(taskIter);
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

static std::string triggerStateToString(const diagnose::TriggerState& state)
{
    switch (state)
    {
    case diagnose::TriggerState::created:
        return "created";
    case diagnose::TriggerState::queuing:
        return "queuing";
    case diagnose::TriggerState::running:
        return "running";
    case diagnose::TriggerState::finished:
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

static std::string diagTaskToString(const diagnose::DiagTaskPtr& diagTask, bool showRun)
{
    static const std::chrono::steady_clock::time_point zero{};
    std::string result;
    result += "{";
    result += "\"id\":" + std::to_string(diagTask->task->getId());
    result += ",";
    result += "\"name\":\"" + diagTask->task->getName() + "\"";
    result += ",";
    result += "\"thread_id\":" + std::to_string(diagTask->attachThreadId);
    result += ",";
    result += "\"thread_name\":\"" + diagTask->attachThreadName + "\"";
    result += ",";
    result += "\"state\":\"" + (diagTask->exceptionMsg.empty() ? taskStateToString(diagTask->task->getState()) : "abnormal") + "\"";
    if (!diagTask->exceptionMsg.empty())
    {
        result += ",";
        result += "\"error\":\"" + diagTask->exceptionMsg + "\"";
    }
    auto now = std::chrono::steady_clock::now();
    /* 排队耗时 */
    if (diagTask->queuing > zero)
    {
        result += ",";
        result += "\"queue\":\"" + durationToString((diagTask->running > zero ? diagTask->running : now) - diagTask->queuing) + "\"";
    }
    /* 运行耗时 */
    if (showRun && diagTask->running > zero)
    {
        result += ",";
        if (diagTask->finished > zero)
        {
            result += "\"run\":\"" + durationToString(diagTask->finished - diagTask->running) + "\"";
        }
        else if (diagTask->abnormal > zero)
        {
            result += "\"run\":\"" + durationToString(diagTask->abnormal - diagTask->running) + "\"";
        }
        else
        {
            result += "\"run\":\"" + durationToString(now - diagTask->running) + "\"";
        }
    }
    result += "}";
    return result;
}

static std::string diagExecutorToString(const diagnose::DiagExecutorPtr& executorInfo)
{
    std::string result;
    result += "{";
    result += "\"name\":\"" + executorInfo->executor->getName() + "\"";
    result += ",";
    result += "\"count\":" + std::to_string(executorInfo->taskList.size());
    result += ",";
    result += "\"task\":[";
    for (auto iter = executorInfo->taskList.begin(); executorInfo->taskList.end() != iter; ++iter)
    {
        if (executorInfo->taskList.begin() != iter)
        {
            result += ",";
        }
        result += diagTaskToString(iter->second, true);
    }
    result += "]";
    result += "}";
    return result;
}

static std::string diagTriggerToString(const diagnose::DiagTriggerPtr& timerTriggerInfo, bool showRun)
{
    static const std::chrono::steady_clock::time_point zero{};
    std::string result;
    result += "{";
    result += "\"id\":" + std::to_string(timerTriggerInfo->timer->getId());
    result += ",";
    result += "\"name\":\"" + timerTriggerInfo->timer->getName() + "\"";
    result += ",";
    result += "\"state\":\"" + (timerTriggerInfo->exceptionMsg.empty() ? triggerStateToString(timerTriggerInfo->state) : "abnormal") + "\"";
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
    const auto iter = s_executorList.find(executor);
    if (s_executorList.end() != iter)
    {
        return;
    }
    auto executorInfo = std::make_unique<diagnose::DiagExecutor>(executor);
    s_executorList.insert(std::make_pair(executor, std::move(executorInfo)));
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
    const auto iter = s_executorList.find(executor);
    if (s_executorList.end() == iter)
    {
        return;
    }
    iter->second->taskList.clear();
    s_executorList.erase(iter);
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
    TaskBindCallback bindedCallback;
    std::string executorName;
    int taskCount = 0;
    int64_t taskId = 0;
    std::string taskName;
    {
        std::lock_guard<std::mutex> locker(s_mutexTask);
        const auto iter = s_executorList.find(executor);
        if (s_executorList.end() == iter)
        {
            return;
        }
        if (iter->second->taskList.end() != iter->second->taskList.find(task))
        {
            return;
        }
        auto diagTask = std::make_shared<diagnose::DiagTask>(task);
        diagTask->attachExecutorName = executor->getName();
        diagTask->queuing = std::chrono::steady_clock::now();
        iter->second->taskList.insert(std::make_pair(task, diagTask));
        bindedCallback = s_taskBindedCallback;
        executorName = executor->getName();
        taskCount = iter->second->taskList.size();
        taskId = task->getId();
        taskName = task->getName();
    }
    if (bindedCallback)
    {
        bindedCallback(executorName, taskCount, taskId, taskName);
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
        auto diagTask = getDiagTask(task);
        if (!diagTask)
        {
            return;
        }
        diagTask->running = std::chrono::steady_clock::now();
        diagTask->attachThreadId = threadId;
        diagTask->attachThreadName = threadName;
        runningCallback = s_taskRunningStateCallback;
        executorName = diagTask->attachExecutorName;
        taskId = task->getId();
        prevElapsed = diagTask->running - diagTask->queuing;
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
        auto diagTask = getDiagTask(task);
        if (!diagTask)
        {
            return;
        }
        diagTask->finished = std::chrono::steady_clock::now();
        diagTask->attachThreadId = threadId;
        diagTask->attachThreadName = threadName;
        finishedCallback = s_taskFinishedStateCallback;
        executorName = diagTask->attachExecutorName;
        taskId = task->getId();
        prevElapsed = diagTask->finished - diagTask->running;
        delDiagTask(task);
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
        auto diagTask = getDiagTask(task);
        if (!diagTask)
        {
            return;
        }
        diagTask->abnormal = std::chrono::steady_clock::now();
        diagTask->attachThreadId = threadId;
        diagTask->attachThreadName = threadName;
        diagTask->exceptionMsg = msg;
        exceptionCallback = s_taskExceptionStateCallback;
        executorName = diagTask->attachExecutorName;
        taskId = task->getId();
        delDiagTask(task);
    }
    if (exceptionCallback)
    {
        exceptionCallback(executorName, threadId, threadName, taskId, taskName, msg);
    }
}

DiscardType Diagnose::onTriggerCreated(int triggerCount, int64_t oldestTriggerId, int64_t triggerId, const Timer* timer)
{
    if (!s_enabled)
    {
        return DiscardType::none;
    }
    TriggerCreateCallback createdCallback;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        createdCallback = s_triggerCreatedCallback;
    }
    auto discardType = createdCallback ? createdCallback(triggerCount, timer->getId(), timer->getName()) : DiscardType::none;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        switch (discardType)
        {
        case DiscardType::discard_newest: /* 丢弃最新 */
            return discardType;
        case DiscardType::discard_oldest: /* 丢弃最早 */
        {
            auto iter = s_triggerList.find(oldestTriggerId);
            if (s_triggerList.end() != iter)
            {
                s_triggerList.erase(iter);
            }
        }
        break;
        case DiscardType::discard_all: /* 丢弃所有 */
            s_triggerList.clear();
            break;
        default: /* 不丢弃(可能会内存持续上涨) */
            break;
        }
        auto timerTriggerInfo = std::make_shared<diagnose::DiagTrigger>(timer);
        timerTriggerInfo->state = diagnose::TriggerState::queuing;
        timerTriggerInfo->queuing = std::chrono::steady_clock::now();
        s_triggerList.insert(std::make_pair(triggerId, timerTriggerInfo));
    }
    return discardType;
}

void Diagnose::onTriggerRunning(int64_t triggerId, const Timer* timer)
{
    if (!s_enabled)
    {
        return;
    }
    TriggerNormalStateCallback runningCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        auto iter = s_triggerList.find(triggerId);
        if (s_triggerList.end() == iter)
        {
            return;
        }
        iter->second->state = diagnose::TriggerState::running;
        iter->second->running = std::chrono::steady_clock::now();
        runningCallback = s_triggerRunningStateCallback;
        prevElapsed = iter->second->running - iter->second->queuing;
    }
    if (runningCallback)
    {
        runningCallback(timer->getId(), timer->getName(), prevElapsed);
    }
}

void Diagnose::onTriggerFinished(int64_t triggerId, const Timer* timer)
{
    if (!s_enabled)
    {
        return;
    }
    TriggerNormalStateCallback finishedCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        auto iter = s_triggerList.find(triggerId);
        if (s_triggerList.end() == iter)
        {
            return;
        }
        iter->second->state = diagnose::TriggerState::finished;
        iter->second->finished = std::chrono::steady_clock::now();
        finishedCallback = s_triggerFinishedStateCallback;
        prevElapsed = iter->second->finished - iter->second->running;
        s_triggerList.erase(iter);
    }
    if (finishedCallback)
    {
        finishedCallback(timer->getId(), timer->getName(), prevElapsed);
    }
}

void Diagnose::onTriggerException(int64_t triggerId, const Timer* timer, const std::string& msg)
{
    if (!s_enabled)
    {
        return;
    }
    TriggerExceptionStateCallback exceptionCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        auto iter = s_triggerList.find(triggerId);
        if (s_triggerList.end() == iter)
        {
            return;
        }
        iter->second->state = diagnose::TriggerState::finished;
        iter->second->abnormal = std::chrono::steady_clock::now();
        iter->second->exceptionMsg = msg;
        exceptionCallback = s_triggerExceptionStateCallback;
        s_triggerList.erase(iter);
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
    for (auto iter = s_triggerList.begin(); s_triggerList.end() != iter;)
    {
        if (timer == iter->second->timer)
        {
            s_triggerList.erase(iter);
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

void Diagnose::setTaskBindedCallback(const TaskBindCallback& bindedCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTask);
    s_taskBindedCallback = bindedCb;
}

void Diagnose::setTaskRunningStateCallback(const TaskNormalStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTask);
    s_taskRunningStateCallback = stateCb;
}

void Diagnose::setTaskFinishedStateCallback(const TaskNormalStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTask);
    s_taskFinishedStateCallback = stateCb;
}

void Diagnose::setTaskExceptionStateCallback(const TaskExceptionStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTask);
    s_taskExceptionStateCallback = stateCb;
}

void Diagnose::setTriggerCreatedCallback(const TriggerCreateCallback& createdCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTimer);
    s_triggerCreatedCallback = createdCb;
}

void Diagnose::setTriggerRunningStateCallback(const TriggerNormalStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTimer);
    s_triggerRunningStateCallback = stateCb;
}

void Diagnose::setTriggerFinishedStateCallback(const TriggerNormalStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTimer);
    s_triggerFinishedStateCallback = stateCb;
}

void Diagnose::setTriggerExceptionStateCallback(const TriggerExceptionStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTimer);
    s_triggerExceptionStateCallback = stateCb;
}

std::string Diagnose::getTaskDiagnoseInfo()
{
    std::string result;
    result += "[";
    {
        std::lock_guard<std::mutex> locker(s_mutexTask);
        for (auto iter = s_executorList.begin(); s_executorList.end() != iter; ++iter)
        {
            if (s_executorList.begin() != iter)
            {
                result += ",";
            }
            result += diagExecutorToString(iter->second);
        }
    }
    result += "]";
    return result;
}

std::string Diagnose::getTriggerDiagnoseInfo()
{
    std::string result;
    result += "[";
    {
        std::lock_guard<std::mutex> locker(s_mutexTimer);
        for (auto iter = s_triggerList.begin(); s_triggerList.end() != iter; ++iter)
        {
            if (s_triggerList.begin() != iter)
            {
                result += ",";
            }
            result += diagTriggerToString(iter->second, true);
        }
    }
    result += "]";
    return result;
}
} // namespace threading
