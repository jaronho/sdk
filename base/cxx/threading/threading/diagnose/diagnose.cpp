#include "diagnose.h"

#include <atomic>
#include <iostream>
#include <mutex>
#include <unordered_map>

#include "../task/executor.h"

namespace threading
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
    std::string executorName; /* 所在执行者(线程池)名称 */
    int64_t threadId = 0; /* 所在线程id */
    std::string threadName; /* 所在线程名称 */
    std::string exceptionMsg; /* 异常消息 */
};

/**
 * @brief 执行者(线程池)诊断信息
 */
struct DiagExecutor
{
    DiagExecutor(const Executor* executor) : executor(executor) {}
    const Executor* executor;
    std::unordered_map<const Task*, std::shared_ptr<DiagTask>> taskList; /* 任务列表 */
};

static std::atomic_bool s_enabled = {false}; /* 是否开启诊断功能(默认关闭) */
static std::mutex s_mutexTask;
static std::unordered_map<const Executor*, std::unique_ptr<DiagExecutor>> s_executorList; /* 执行者(线程池)列表 */
static TaskCreatedCallback s_taskCreatedCallback = nullptr; /* 任务创建回调 */
static TaskNormalStateCallback s_taskRunningStateCallback = nullptr; /* 任务运行状态回调 */
static TaskNormalStateCallback s_taskFinishedStateCallback = nullptr; /* 任务结束状态回调 */
static TaskExceptionStateCallback s_taskExceptionStateCallback = nullptr; /* 任务异常状态回调 */

static std::shared_ptr<DiagTask> getDiagTask(const Task* task)
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

void Diagnose::setTaskCreatedCallback(const TaskCreatedCallback& createdCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTask);
    s_taskCreatedCallback = createdCb;
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

void Diagnose::setEnable(bool enable)
{
    s_enabled = enable;
}

std::vector<ExecutorDiagnoseInfo> Diagnose::getTaskDiagnoseInfo()
{
    std::vector<ExecutorDiagnoseInfo> infoList;
    std::lock_guard<std::mutex> locker(s_mutexTask);
    for (auto iter = s_executorList.begin(); s_executorList.end() != iter; ++iter)
    {
        ExecutorDiagnoseInfo edi;
        edi.name = iter->second->executor->getName();
        for (auto taskIter = iter->second->taskList.begin(); iter->second->taskList.end() != taskIter; ++taskIter)
        {
            TaskDiagnoseInfo info;
            info.taskName = taskIter->second->task->getName();
            info.threadId = taskIter->second->threadId;
            info.threadName = taskIter->second->threadName;
            switch (taskIter->second->task->getState())
            {
            case Task::State::created:
                info.state = DiagnoseState::created;
                info.queue = std::chrono::steady_clock::duration::zero();
                info.run = std::chrono::steady_clock::duration::zero();
                break;
            case Task::State::queuing:
                info.state = DiagnoseState::queuing;
                info.queue = std::chrono::steady_clock::now() - taskIter->second->queuing;
                info.run = std::chrono::steady_clock::duration::zero();
                break;
            case Task::State::running:
                info.state = DiagnoseState::running;
                info.queue = taskIter->second->running - taskIter->second->queuing;
                info.run = std::chrono::steady_clock::now() - taskIter->second->running;
                break;
            case Task::State::finished:
                info.state = DiagnoseState::finished;
                std::chrono::steady_clock::time_point zero{};
                if (taskIter->second->finished > zero)
                {
                    info.run = taskIter->second->finished - taskIter->second->running;
                }
                else if (taskIter->second->abnormal > zero)
                {
                    info.run = taskIter->second->abnormal - taskIter->second->running;
                }
                else
                {
                    info.run = std::chrono::steady_clock::now() - taskIter->second->running;
                }
                break;
            }
            info.exceptionMsg = taskIter->second->exceptionMsg;
            edi.taskInfoList.emplace_back(info);
        }
        infoList.emplace_back(edi);
    }
    return infoList;
}

void Diagnose::onExecutorCreated(const Executor* executor)
{
    if (!executor)
    {
        return;
    }
    std::lock_guard<std::mutex> locker(s_mutexTask);
    if (s_executorList.end() == s_executorList.find(executor))
    {
        auto executorInfo = std::make_unique<DiagExecutor>(executor);
        s_executorList.insert(std::make_pair(executor, std::move(executorInfo)));
    }
}

void Diagnose::onExecutorDestroyed(const Executor* executor)
{
    if (!executor)
    {
        return;
    }
    std::lock_guard<std::mutex> locker(s_mutexTask);
    const auto iter = s_executorList.find(executor);
    if (s_executorList.end() != iter)
    {
        iter->second->taskList.clear();
        s_executorList.erase(iter);
    }
}

void Diagnose::onTaskCreated(const Executor* executor, const Task* task)
{
    if (!s_enabled)
    {
        return;
    }
    if (!executor || !task)
    {
        return;
    }
    TaskCreatedCallback createdCallback;
    std::string executorName;
    size_t maxCount, nowCount;
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
        executorName = executor->getName();
        maxCount = executor->getMaxCount();
        nowCount = iter->second->taskList.size();
        taskName = task->getName();
        auto diagTask = std::make_shared<DiagTask>(task);
        diagTask->executorName = executor->getName();
        diagTask->queuing = std::chrono::steady_clock::now();
        iter->second->taskList.insert(std::make_pair(task, diagTask));
        createdCallback = s_taskCreatedCallback;
    }
    if (createdCallback)
    {
        createdCallback(executorName, maxCount, nowCount, taskName);
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
        diagTask->threadId = threadId;
        diagTask->threadName = threadName;
        runningCallback = s_taskRunningStateCallback;
        executorName = diagTask->executorName;
        taskName = task->getName();
        prevElapsed = diagTask->running - diagTask->queuing;
    }
    if (runningCallback)
    {
        runningCallback(executorName, threadId, threadName, taskName, prevElapsed);
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
        diagTask->threadId = threadId;
        diagTask->threadName = threadName;
        finishedCallback = s_taskFinishedStateCallback;
        executorName = diagTask->executorName;
        taskName = task->getName();
        prevElapsed = diagTask->finished - diagTask->running;
        delDiagTask(task);
    }
    if (finishedCallback)
    {
        finishedCallback(executorName, threadId, threadName, taskName, prevElapsed);
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
    std::string taskName;
    {
        std::lock_guard<std::mutex> locker(s_mutexTask);
        auto diagTask = getDiagTask(task);
        if (!diagTask)
        {
            return;
        }
        diagTask->abnormal = std::chrono::steady_clock::now();
        diagTask->threadId = threadId;
        diagTask->threadName = threadName;
        diagTask->exceptionMsg = msg;
        exceptionCallback = s_taskExceptionStateCallback;
        executorName = diagTask->executorName;
        taskName = task->getName();
        delDiagTask(task);
    }
    if (exceptionCallback)
    {
        exceptionCallback(executorName, threadId, threadName, taskName, msg);
    }
}
} // namespace threading
