#include "diagnose.h"

#include <atomic>
#include <iostream>
#include <mutex>
#include <unordered_map>

#include "../async_proxy.h"
#include "../task/executor.h"
#include "../timer/timer.h"

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

/**
 * @brief 结束器诊断信息
 */
struct DiagFinisher
{
    DiagFinisher(const AsyncTask* task) : task(task) {}
    const AsyncTask* task;
    DiagnoseState state = DiagnoseState::created; /* 诊断状态 */
    std::chrono::steady_clock::time_point queuing{}; /* 开始排队时间点 */
    std::chrono::steady_clock::time_point running{}; /* 开始运行时间点 */
    std::chrono::steady_clock::time_point finished{}; /* 运行结束时间点 */
    std::chrono::steady_clock::time_point abnormal{}; /* 出现异常时间点 */
    std::string exceptionMsg; /* 异常消息 */
};

/**
 * @brief 触发器诊断信息
 */
struct DiagTrigger
{
    DiagTrigger(const Timer* timer) : timer(timer) {}
    const Timer* timer;
    DiagnoseState state = DiagnoseState::created; /* 诊断状态 */
    std::chrono::steady_clock::time_point queuing{}; /* 开始排队时间点 */
    std::chrono::steady_clock::time_point running{}; /* 开始运行时间点 */
    std::chrono::steady_clock::time_point finished{}; /* 运行结束时间点 */
    std::chrono::steady_clock::time_point abnormal{}; /* 出现异常时间点 */
    std::string exceptionMsg; /* 异常消息 */
};

static std::atomic_bool s_enabled = {false}; /* 是否开启诊断功能(默认关闭) */
static std::mutex s_mutexTask;
static std::unordered_map<const Executor*, std::unique_ptr<DiagExecutor>> s_executorList; /* 执行者(线程池)列表 */
static TaskCreatedCallback s_taskCreatedCallback = nullptr; /* 任务创建回调 */
static TaskNormalStateCallback s_taskRunningStateCallback = nullptr; /* 任务运行状态回调 */
static TaskNormalStateCallback s_taskFinishedStateCallback = nullptr; /* 任务结束状态回调 */
static TaskExceptionStateCallback s_taskExceptionStateCallback = nullptr; /* 任务异常状态回调 */
static std::mutex s_mutexFinisher;
static std::unordered_map<uint64_t, std::shared_ptr<DiagFinisher>> s_finisherList; /* 结束器列表 */
static FinisherCreateCallback s_finisherCreatedCallback = nullptr; /* 结束器创建回调 */
static FinisherNormalStateCallback s_finisherRunningStateCallback = nullptr; /* 结束器运行状态回调 */
static FinisherNormalStateCallback s_finisherFinishedStateCallback = nullptr; /* 结束器结束状态回调 */
static FinisherExceptionStateCallback s_finisherExceptionStateCallback = nullptr; /* 结束器异常状态回调 */
static std::mutex s_mutexTrigger;
static std::unordered_map<uint64_t, std::shared_ptr<DiagTrigger>> s_triggerList; /* 触发器列表 */
static TriggerCreateCallback s_triggerCreatedCallback = nullptr; /* 触发器创建回调 */
static TriggerNormalStateCallback s_triggerRunningStateCallback = nullptr; /* 触发器运行状态回调 */
static TriggerNormalStateCallback s_triggerFinishedStateCallback = nullptr; /* 触发器结束状态回调 */
static TriggerExceptionStateCallback s_triggerExceptionStateCallback = nullptr; /* 触发器异常状态回调 */

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

void Diagnose::setEnable()
{
    s_enabled = true;
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

void Diagnose::setFinisherCreatedCallback(const FinisherCreateCallback& createdCb)
{
    std::lock_guard<std::mutex> locker(s_mutexFinisher);
    s_finisherCreatedCallback = createdCb;
}

void Diagnose::setFinisherRunningStateCallback(const FinisherNormalStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexFinisher);
    s_finisherRunningStateCallback = stateCb;
}

void Diagnose::setFinisherFinishedStateCallback(const FinisherNormalStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexFinisher);
    s_finisherFinishedStateCallback = stateCb;
}

void Diagnose::setFinisherExceptionStateCallback(const FinisherExceptionStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexFinisher);
    s_finisherExceptionStateCallback = stateCb;
}

void Diagnose::setTriggerCreatedCallback(const TriggerCreateCallback& createdCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTrigger);
    s_triggerCreatedCallback = createdCb;
}

void Diagnose::setTriggerRunningStateCallback(const TriggerNormalStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTrigger);
    s_triggerRunningStateCallback = stateCb;
}

void Diagnose::setTriggerFinishedStateCallback(const TriggerNormalStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTrigger);
    s_triggerFinishedStateCallback = stateCb;
}

void Diagnose::setTriggerExceptionStateCallback(const TriggerExceptionStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_mutexTrigger);
    s_triggerExceptionStateCallback = stateCb;
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
            info.taskId = taskIter->second->task->getId();
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

std::vector<FinisherDiagnoseInfo> Diagnose::getFinisherDiagnoseInfo()
{
    std::vector<FinisherDiagnoseInfo> infoList;
    std::lock_guard<std::mutex> locker(s_mutexFinisher);
    for (auto iter = s_finisherList.begin(); s_finisherList.end() != iter; ++iter)
    {
        FinisherDiagnoseInfo info;
        info.taskId = iter->second->task->getId();
        info.taskName = iter->second->task->getName();
        info.state = iter->second->state;
        switch (iter->second->state)
        {
        case DiagnoseState::queuing:
            info.state = DiagnoseState::queuing;
            info.queue = std::chrono::steady_clock::now() - iter->second->queuing;
            info.run = std::chrono::steady_clock::duration::zero();
            break;
        case DiagnoseState::running:
            info.queue = iter->second->running - iter->second->queuing;
            info.run = std::chrono::steady_clock::now() - iter->second->running;
            break;
        case DiagnoseState::finished:
            std::chrono::steady_clock::time_point zero{};
            if (iter->second->finished > zero)
            {
                info.run = iter->second->finished - iter->second->running;
            }
            else if (iter->second->abnormal > zero)
            {
                info.run = iter->second->abnormal - iter->second->running;
            }
            else
            {
                info.run = std::chrono::steady_clock::now() - iter->second->running;
            }
            break;
        }
        infoList.emplace_back(info);
    }
    return infoList;
}

std::vector<TriggerDiagnoseInfo> Diagnose::getTriggerDiagnoseInfo()
{
    std::vector<TriggerDiagnoseInfo> infoList;
    std::lock_guard<std::mutex> locker(s_mutexTrigger);
    for (auto iter = s_triggerList.begin(); s_triggerList.end() != iter; ++iter)
    {
        TriggerDiagnoseInfo info;
        info.timerId = iter->second->timer->getId();
        info.timerName = iter->second->timer->getName();
        info.state = iter->second->state;
        switch (iter->second->state)
        {
        case DiagnoseState::queuing:
            info.state = DiagnoseState::queuing;
            info.queue = std::chrono::steady_clock::now() - iter->second->queuing;
            info.run = std::chrono::steady_clock::duration::zero();
            break;
        case DiagnoseState::running:
            info.queue = iter->second->running - iter->second->queuing;
            info.run = std::chrono::steady_clock::now() - iter->second->running;
            break;
        case DiagnoseState::finished:
            std::chrono::steady_clock::time_point zero{};
            if (iter->second->finished > zero)
            {
                info.run = iter->second->finished - iter->second->running;
            }
            else if (iter->second->abnormal > zero)
            {
                info.run = iter->second->abnormal - iter->second->running;
            }
            else
            {
                info.run = std::chrono::steady_clock::now() - iter->second->running;
            }
            break;
        }
        infoList.emplace_back(info);
    }
    return infoList;
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
    if (s_executorList.end() == s_executorList.find(executor))
    {
        auto executorInfo = std::make_unique<DiagExecutor>(executor);
        s_executorList.insert(std::make_pair(executor, std::move(executorInfo)));
    }
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
    int taskCount = 0;
    uint64_t taskId = 0;
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
        auto diagTask = std::make_shared<DiagTask>(task);
        diagTask->executorName = executor->getName();
        diagTask->queuing = std::chrono::steady_clock::now();
        iter->second->taskList.insert(std::make_pair(task, diagTask));
        createdCallback = s_taskCreatedCallback;
        executorName = executor->getName();
        taskCount = iter->second->taskList.size();
        taskId = task->getId();
        taskName = task->getName();
    }
    if (createdCallback)
    {
        createdCallback(executorName, taskCount, taskId, taskName);
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
    uint64_t taskId = 0;
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
        taskId = task->getId();
        taskName = task->getName();
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
    uint64_t taskId = 0;
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
        taskId = task->getId();
        taskName = task->getName();
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
    uint64_t taskId = 0;
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
        taskId = task->getId();
        taskName = task->getName();
        delDiagTask(task);
    }
    if (exceptionCallback)
    {
        exceptionCallback(executorName, threadId, threadName, taskId, taskName, msg);
    }
}

void Diagnose::onFinisherCreated(int finisherCount, uint64_t finisherId, const AsyncTask* task)
{
    if (!s_enabled)
    {
        return;
    }
    FinisherCreateCallback createdCallback;
    {
        std::lock_guard<std::mutex> locker(s_mutexFinisher);
        createdCallback = s_finisherCreatedCallback;
    }
    if (createdCallback)
    {
        createdCallback(finisherCount, task->getId(), task->getName());
    }
    {
        std::lock_guard<std::mutex> locker(s_mutexFinisher);
        auto diagFinisher = std::make_shared<DiagFinisher>(task);
        diagFinisher->state = DiagnoseState::queuing;
        diagFinisher->queuing = std::chrono::steady_clock::now();
        s_finisherList.insert(std::make_pair(finisherId, diagFinisher));
    }
}

void Diagnose::onFinisherRunning(uint64_t finisherId, const AsyncTask* task)
{
    if (!s_enabled)
    {
        return;
    }
    FinisherNormalStateCallback runningCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexFinisher);
        auto iter = s_finisherList.find(finisherId);
        if (s_finisherList.end() == iter)
        {
            return;
        }
        iter->second->state = DiagnoseState::running;
        iter->second->running = std::chrono::steady_clock::now();
        runningCallback = s_finisherRunningStateCallback;
        prevElapsed = iter->second->running - iter->second->queuing;
    }
    if (runningCallback)
    {
        runningCallback(task->getId(), task->getName(), prevElapsed);
    }
}

void Diagnose::onFinisherFinished(uint64_t finisherId, const AsyncTask* task)
{
    if (!s_enabled)
    {
        return;
    }
    FinisherNormalStateCallback finishedCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexFinisher);
        auto iter = s_finisherList.find(finisherId);
        if (s_finisherList.end() == iter)
        {
            return;
        }
        iter->second->state = DiagnoseState::finished;
        iter->second->finished = std::chrono::steady_clock::now();
        finishedCallback = s_finisherFinishedStateCallback;
        prevElapsed = iter->second->finished - iter->second->running;
        s_finisherList.erase(iter);
    }
    if (finishedCallback)
    {
        finishedCallback(task->getId(), task->getName(), prevElapsed);
    }
}

void Diagnose::onFinisherException(uint64_t finisherId, const AsyncTask* task, const std::string& msg)
{
    if (!s_enabled)
    {
        return;
    }
    FinisherExceptionStateCallback exceptionCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexFinisher);
        auto iter = s_finisherList.find(finisherId);
        if (s_finisherList.end() == iter)
        {
            return;
        }
        iter->second->state = DiagnoseState::finished;
        iter->second->abnormal = std::chrono::steady_clock::now();
        iter->second->exceptionMsg = msg;
        exceptionCallback = s_finisherExceptionStateCallback;
        s_finisherList.erase(iter);
    }
    if (exceptionCallback)
    {
        exceptionCallback(task->getId(), task->getName(), msg);
    }
}

void Diagnose::onTriggerCreated(int triggerCount, uint64_t triggerId, const Timer* timer)
{
    if (!s_enabled)
    {
        return;
    }
    TriggerCreateCallback createdCallback;
    {
        std::lock_guard<std::mutex> locker(s_mutexTrigger);
        createdCallback = s_triggerCreatedCallback;
    }
    if (createdCallback)
    {
        createdCallback(triggerCount, timer->getId(), timer->getName());
    }
    {
        std::lock_guard<std::mutex> locker(s_mutexTrigger);
        auto diagTrigger = std::make_shared<DiagTrigger>(timer);
        diagTrigger->state = DiagnoseState::queuing;
        diagTrigger->queuing = std::chrono::steady_clock::now();
        s_triggerList.insert(std::make_pair(triggerId, diagTrigger));
    }
}

void Diagnose::onTriggerRunning(uint64_t triggerId, const Timer* timer)
{
    if (!s_enabled)
    {
        return;
    }
    TriggerNormalStateCallback runningCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTrigger);
        auto iter = s_triggerList.find(triggerId);
        if (s_triggerList.end() == iter)
        {
            return;
        }
        iter->second->state = DiagnoseState::running;
        iter->second->running = std::chrono::steady_clock::now();
        runningCallback = s_triggerRunningStateCallback;
        prevElapsed = iter->second->running - iter->second->queuing;
    }
    if (runningCallback)
    {
        runningCallback(timer->getId(), timer->getName(), prevElapsed);
    }
}

void Diagnose::onTriggerFinished(uint64_t triggerId, const Timer* timer)
{
    if (!s_enabled)
    {
        return;
    }
    TriggerNormalStateCallback finishedCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTrigger);
        auto iter = s_triggerList.find(triggerId);
        if (s_triggerList.end() == iter)
        {
            return;
        }
        iter->second->state = DiagnoseState::finished;
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

void Diagnose::onTriggerException(uint64_t triggerId, const Timer* timer, const std::string& msg)
{
    if (!s_enabled)
    {
        return;
    }
    TriggerExceptionStateCallback exceptionCallback;
    std::chrono::steady_clock::duration prevElapsed;
    {
        std::lock_guard<std::mutex> locker(s_mutexTrigger);
        auto iter = s_triggerList.find(triggerId);
        if (s_triggerList.end() == iter)
        {
            return;
        }
        iter->second->state = DiagnoseState::finished;
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
    std::lock_guard<std::mutex> locker(s_mutexTrigger);
    for (auto iter = s_triggerList.begin(); s_triggerList.end() != iter; ++iter)
    {
        if (timer == iter->second->timer)
        {
            s_triggerList.erase(iter);
            return;
        }
    }
}
} // namespace threading
