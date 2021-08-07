#include "diagnose.h"

#include <iostream>
#include <mutex>
#include <unordered_map>

namespace threading
{
static std::mutex s_mutex; /* 互斥锁 */
static std::unordered_map<const Executor*, ExecutorInfoPtr> s_executors; /* 全局执行者 */
static TaskBindCallback s_taskBindCallback; /* 任务绑定回调 */
static TaskNormalStateCallback s_taskRunningStateCallback; /* 任务运行状态回调 */
static TaskNormalStateCallback s_taskFinishedStateCallback; /* 任务结束状态回调 */
static TaskExceptionStateCallback s_taskExceptionStateCallback; /* 任务异常状态回调 */

TaskInfoPtr Diagnose::getTaskInfo(const Task* task)
{
    if (!task)
    {
        return {};
    }
    for (const auto& iter : s_executors)
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
    for (const auto& iter : s_executors)
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
    std::lock_guard<std::mutex> lock(s_mutex);
    const auto iter = s_executors.find(executor);
    if (s_executors.end() != iter)
    {
        return;
    }
    auto executorInfo = std::make_unique<ExecutorInfo>(executor);
    s_executors.insert(std::make_pair(executor, std::move(executorInfo)));
}

void Diagnose::onExecutorDestroyed(const Executor* executor)
{
    if (!executor)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(s_mutex);
    const auto iter = s_executors.find(executor);
    if (s_executors.end() == iter)
    {
        return;
    }
    iter->second->tasks.clear();
    s_executors.erase(iter);
}

void Diagnose::bindTaskToExecutor(const Task* task, const Executor* executor)
{
    if (!task || !executor)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(s_mutex);
    const auto iter = s_executors.find(executor);
    if (s_executors.end() == iter)
    {
        return;
    }
    if (iter->second->tasks.end() != iter->second->tasks.find(task))
    {
        return;
    }
    auto taskInfo = std::make_shared<TaskInfo>(task);
    taskInfo->attachExecutorName = executor->getName();
    taskInfo->queuing = std::chrono::steady_clock::now();
    iter->second->tasks.insert(std::make_pair(task, taskInfo));
    if (s_taskBindCallback)
    {
        s_taskBindCallback(executor->getName(), task->getName());
    }
}

void Diagnose::onTaskRunning(int threadId, const std::string& threadName, const Task* task)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    auto taskInfo = getTaskInfo(task);
    if (!taskInfo)
    {
        return;
    }
    taskInfo->running = std::chrono::steady_clock::now();
    taskInfo->attachThreadId = threadId;
    taskInfo->attachThreadName = threadName;
    if (s_taskRunningStateCallback)
    {
        s_taskRunningStateCallback(taskInfo->attachExecutorName, taskInfo->attachThreadId, taskInfo->attachThreadName, task->getName(),
                                   taskInfo->running - taskInfo->queuing);
    }
}

void Diagnose::onTaskFinished(int threadId, const std::string& threadName, const Task* task)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    auto taskInfo = getTaskInfo(task);
    if (!taskInfo)
    {
        return;
    }
    taskInfo->finished = std::chrono::steady_clock::now();
    taskInfo->attachThreadId = threadId;
    taskInfo->attachThreadName = threadName;
    if (s_taskFinishedStateCallback)
    {
        s_taskFinishedStateCallback(taskInfo->attachExecutorName, taskInfo->attachThreadId, taskInfo->attachThreadName, task->getName(),
                                    taskInfo->finished - taskInfo->running);
    }
    delTaskInfo(task);
}

void Diagnose::onTaskException(int threadId, const std::string& threadName, const Task* task, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    auto taskInfo = getTaskInfo(task);
    if (!taskInfo)
    {
        return;
    }
    taskInfo->abnormal = std::chrono::steady_clock::now();
    taskInfo->attachThreadId = threadId;
    taskInfo->attachThreadName = threadName;
    taskInfo->exceptionMsg = msg;
    if (s_taskExceptionStateCallback)
    {
        s_taskExceptionStateCallback(taskInfo->attachExecutorName, taskInfo->attachThreadId, taskInfo->attachThreadName, task->getName(),
                                     msg);
    }
    delTaskInfo(task);
}

std::string Diagnose::taskStateToString(const Task::State& state)
{
    switch (state)
    {
    case Task::State::CREATED:
        return "Created";
    case Task::State::QUEUING:
        return "Queuing";
    case Task::State::RUNNING:
        return "Running";
    case Task::State::FINISHED:
        return "Finished";
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

std::string Diagnose::taskInfoToString(const TaskInfoPtr& taskInfo, bool showRun)
{
    static const std::chrono::steady_clock::time_point zero{};
    auto result = "Task [" + taskInfo->task->getName() + "]";
    if (taskInfo->attachThreadId > 0)
    {
        result += ", thread [" + std::to_string(taskInfo->attachThreadId) + ", " + taskInfo->attachThreadName + "]";
    }
    result += ", current state [" + (taskInfo->exceptionMsg.empty() ? taskStateToString(taskInfo->task->getState()) : "ARNORMAL") + "]";
    if (!taskInfo->exceptionMsg.empty())
    {
        result += "(" + taskInfo->exceptionMsg + ")";
    }
    auto now = std::chrono::steady_clock::now();
    /* 排队耗时 */
    if (taskInfo->queuing > zero)
    {
        result += ", queue for [" + durationToString((taskInfo->running > zero ? taskInfo->running : now) - taskInfo->queuing) + "]";
    }
    /* 运行耗时 */
    if (showRun && taskInfo->running > zero)
    {
        if (taskInfo->finished > zero)
        {
            result += ", run for [" + durationToString(taskInfo->finished - taskInfo->running) + "]";
        }
        else if (taskInfo->abnormal > zero)
        {
            result += ", run for [" + durationToString(taskInfo->abnormal - taskInfo->running) + "]";
        }
        else
        {
            result += ", run for [" + durationToString(now - taskInfo->running) + "]";
        }
    }
    return result;
}

std::string Diagnose::executorInfoToString(const ExecutorInfoPtr& executorInfo)
{
    auto result = "Executor [" + executorInfo->executor->getName() + "] tasks:\n";
    for (const auto& iter : executorInfo->tasks)
    {
        result += "    " + taskInfoToString(iter.second, true) + "\n";
    }
    return result;
}

void Diagnose::setTaskBindCallback(const TaskBindCallback& taskBindCb)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_taskBindCallback = taskBindCb;
}

void Diagnose::setTaskRunningStateCallback(const TaskNormalStateCallback& taskStateCb)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_taskRunningStateCallback = taskStateCb;
}

void Diagnose::setTaskFinishedStateCallback(const TaskNormalStateCallback& taskStateCb)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_taskFinishedStateCallback = taskStateCb;
}

void Diagnose::setTaskExceptionStateCallback(const TaskExceptionStateCallback& taskStateCb)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_taskExceptionStateCallback = taskStateCb;
}

std::string Diagnose::getDiagnoseInfo()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    std::string result;
    for (const auto& iter : s_executors)
    {
        result += executorInfoToString(iter.second);
    }
    return result;
}
} // namespace threading
