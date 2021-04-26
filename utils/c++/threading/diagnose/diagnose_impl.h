#pragma once
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "../../task/task_state.h"
#include "base/log/log_manager.h"
#include "base/log/logger.h"
#include "diagnose_info.h"

namespace aqua
{
namespace base
{
namespace threading
{
class Task;
class Timer;
class Executor;

class DiagnoseImpl final
{
public:
    static DiagnoseImpl& GetInstance();

    // Task相关事件
    void TaskCreated(const Task* task, const std::string& name);

    void TaskReleased(const Task* task);

    void TaskStateChanged(const Task* task, TaskState state);

    // Timer相关事件
    void TimerCreated(const Timer* timer, const Task* task, const Executor* executor, const std::string& name);

    void TimerReleased(const Timer* timer);

    void TimerSetTask(const Timer* timer, const Task* task);

    void TimerSetExecutor(const Timer* timer, const Executor* executor);

    void TimerStarted(const Timer* timer);

    void TimerStopped(const Timer* timer);

    void DeadlineTimerSetDeadline(const Timer* timer, const std::chrono::system_clock::time_point& deadline);

    void SteadyTimerSetDelay(const Timer* timer, const std::chrono::steady_clock::duration& delay);

    void SteadyTimerSetInterval(const Timer* timer, const std::chrono::steady_clock::duration& interval);

    // Executor相关事件
    void ExecutorCreated(const Executor* executor, const std::string& name);

    void ExecutorReleased(const Executor* executor);

    void BindTaskWithExecutor(const Task* task, const Executor* executor);

    std::string GetDiagnoseInfo();

private:
    TaskInfoPtr FindTask(const Task* task);

    void RemoveTimerFrom(const Timer* timer, TimerInfoMap& timers);

    TimerInfoPtr& FindTimer(const Timer* timer);

    std::string GetTaskStat(const Task* task);

    static std::string GetTaskStat(const TaskInfoPtr& taskInfo);

    static std::string FormatTaskState(TaskState state);

    static std::string FormatTimePoint(const std::chrono::system_clock::time_point& timePoint);

    static std::string FormatDuration(const std::chrono::steady_clock::duration& duration);

    static std::string FormatTaskInfo(const TaskInfoPtr& taskInfo, const std::string& prefix = {});

    static std::string FormatTimerInfo(const TimerInfoPtr& timerInfo, const std::string& prefix = {});

    static std::string FormatExecutorInfo(const ExecutorInfoPtr& executorInfo);

    log::Logger m_logger = log::LogManager::GetCommonLogger("aqua.base.threading.diag");

    std::mutex m_mutex;

    std::map<const Task*, std::weak_ptr<TaskInfo>> m_tasks;
    TaskInfoMap m_pendingTasks;
    TimerInfoMap m_pendingTimers;
    std::map<const Executor*, ExecutorInfoPtr> m_executors;
};
} // namespace threading
} // namespace base
} // namespace aqua
