#include "diagnose.h"

#include <iostream>
#include <mutex>
#include <unordered_map>

namespace threading
{
static std::mutex s_mutexTask;
static std::unordered_map<const Executor*, diagnose::ExecutorInfoPtr> s_taskExecutors; /* 全局任务执行者 */
static TaskBindCallback s_taskBindCallback = nullptr; /* 任务绑定回调 */
static TaskNormalStateCallback s_taskRunningStateCallback = nullptr; /* 任务运行状态回调 */
static TaskNormalStateCallback s_taskFinishedStateCallback = nullptr; /* 任务结束状态回调 */
static TaskExceptionStateCallback s_taskExceptionStateCallback = nullptr; /* 任务异常状态回调 */

diagnose::TaskInfoPtr Diagnose::getTaskInfo(const Task* task)
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

void Diagnose::delTaskInfo(const Task* task)
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

void Diagnose::onExecutorCreated(const Executor* executor)
{
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

std::string Diagnose::taskStateToString(const Task::State& state)
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

std::string Diagnose::durationToString(const std::chrono::steady_clock::duration& duration)
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

std::string Diagnose::taskInfoToString(const diagnose::TaskInfoPtr& taskInfo, bool showRun)
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

std::string Diagnose::executorInfoToString(const diagnose::ExecutorInfoPtr& executorInfo)
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

std::string Diagnose::getDiagnoseInfo()
{
    std::string result;
    result += "{";
    result += "\"executor\":[";
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
    result += "}";
    return result;
}
} // namespace threading
