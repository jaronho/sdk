#include "diagnose_impl.h"

#include "base/log/log.h"

#define THREADING_DIAGNOSE_LOG(logger, f, ...) // DEBUG_LOG(logger, f, ##__VA_ARGS__)

namespace aqua
{
namespace base
{
namespace threading
{
DiagnoseImpl& DiagnoseImpl::GetInstance()
{
    static DiagnoseImpl instance;
    return instance;
}

void DiagnoseImpl::TaskCreated(const Task* task, const std::string& name)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Task [{}]({}) created", fmt::ptr(task), name);
    // task创建，先加入pending列表
    const auto taskInfo = std::make_shared<TaskInfo>(task, name);
    taskInfo->state = TaskState::Created;
    taskInfo->created = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_tasks[task] = taskInfo;
    m_pendingTasks[task] = taskInfo;
}

void DiagnoseImpl::TaskReleased(const Task* task)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    THREADING_DIAGNOSE_LOG(m_logger, "Task [{}] released, stat: {}", fmt::ptr(task),
                           GetTaskStat(task)); // 由于包含统计信息，因此需要放在锁的范围内
    // task释放，从每个列表中移除
    m_tasks.erase(task);
    m_pendingTasks.erase(task);
    for (const auto& it : m_executors)
    {
        for (const auto& timer : it.second->timers)
        {
            if (timer.second->task->task == task)
            {
                timer.second->task.reset();
            }
        }
        it.second->tasks.erase(task);
    }
}

void DiagnoseImpl::TaskStateChanged(const Task* task, TaskState state)
{
    const auto stateName = FormatTaskState(state);
    THREADING_DIAGNOSE_LOG(m_logger, "Task [{}] enter state: {}", fmt::ptr(task), stateName);
    // task状态变化，更新对应信息并设置时间
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto taskInfo = FindTask(task);
    if (!taskInfo)
    {
        WARNING_LOG(m_logger, "Task [{}] info relesed, cannot enter state: {}", fmt::ptr(task), stateName);
        return;
    }
    taskInfo->state = state;
    switch (state)
    {
    case TaskState::Created:
        taskInfo->created = std::chrono::steady_clock::now();
        break;
    case TaskState::Queuing:
        taskInfo->queued = std::chrono::steady_clock::now();
        break;
    case TaskState::Running:
        taskInfo->run = std::chrono::steady_clock::now();
        break;
    case TaskState::Finished:
        taskInfo->finished = std::chrono::steady_clock::now();
        break;
    default:
        break;
    }
}

void DiagnoseImpl::TimerCreated(const Timer* timer, const Task* task, const Executor* executor, const std::string& name)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Timer [{}]({}) created with task [{}] and executor [{}]", fmt::ptr(timer), name,
                           fmt::ptr(task), fmt::ptr(executor));
    // 创建timer
    auto timerInfo = std::make_unique<TimerInfo>(timer, name);
    std::lock_guard<std::mutex> lock(m_mutex);
    // 绑定task
    timerInfo->task = FindTask(task);
    m_pendingTasks.erase(task);
    // 绑定executor
    m_executors[executor]->timers[timer] = std::move(timerInfo);
}

void DiagnoseImpl::TimerReleased(const Timer* timer)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Timer [{}] released", fmt::ptr(timer));
    std::lock_guard<std::mutex> lock(m_mutex);
    // 遍历executor列表并移除
    for (const auto& executorIter : m_executors)
    {
        RemoveTimerFrom(timer, executorIter.second->timers);
    }
    // 从pending列表移除
    RemoveTimerFrom(timer, m_pendingTimers);
}

void DiagnoseImpl::TimerSetTask(const Timer* timer, const Task* task)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Timer [{}] bind task [{}]", fmt::ptr(timer), fmt::ptr(task));
    // timer更新绑定task
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& timerInfo = FindTimer(timer);
    if (!timerInfo)
    {
        WARNING_LOG(m_logger, "Cannot find timer [{}] to start", fmt::ptr(timer));
        return;
    }
    if (timerInfo->task)
    {
        m_pendingTasks[timerInfo->task->task] = timerInfo->task;
    }
    timerInfo->task = FindTask(task);
    m_pendingTasks.erase(task);
}

void DiagnoseImpl::TimerSetExecutor(const Timer* timer, const Executor* executor)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Timer [{}] bind executor [{}]", fmt::ptr(timer), fmt::ptr(executor));
    std::lock_guard<std::mutex> lock(m_mutex);
    TimerInfoPtr timerInfo;
    // 找到timer，并和原先对应的executor解除绑定
    for (const auto& executorIter : m_executors)
    {
        // 查找对应timer
        const auto timerIter = executorIter.second->timers.find(timer);
        if (timerIter == executorIter.second->timers.end())
        {
            continue;
        }
        // 获取timer引用
        timerInfo = std::move(timerIter->second);
        // 从原先的executor移除
        executorIter.second->timers.erase(timerIter);
    }
    // 绑定新的executor
    m_executors[executor]->timers[timer] = std::move(timerInfo);
}

void DiagnoseImpl::TimerStarted(const Timer* timer)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Timer [{}] started", fmt::ptr(timer));
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& timerInfo = FindTimer(timer);
    if (!timerInfo)
    {
        WARNING_LOG(m_logger, "Cannot find timer [{}] to start", fmt::ptr(timer));
        return;
    }
    timerInfo->started = true;
}

void DiagnoseImpl::TimerStopped(const Timer* timer)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Timer [{}] stopped", fmt::ptr(timer));
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& timerInfo = FindTimer(timer);
    if (!timerInfo)
    {
        WARNING_LOG(m_logger, "Cannot find timer [{}] to stop", fmt::ptr(timer));
        return;
    }
    timerInfo->started = false;
}

void DiagnoseImpl::DeadlineTimerSetDeadline(const Timer* timer, const std::chrono::system_clock::time_point& deadline)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Timer [{}] set deadline [{}]", fmt::ptr(timer), FormatTimePoint(deadline));
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& timerInfo = FindTimer(timer);
    if (!timerInfo)
    {
        WARNING_LOG(m_logger, "Cannot find timer [{}] to stop", fmt::ptr(timer));
        return;
    }
    timerInfo->type = TimerType::DeadlineTimer;
    timerInfo->data.deadlineTimerData.deadline = deadline;
}

void DiagnoseImpl::SteadyTimerSetDelay(const Timer* timer, const std::chrono::steady_clock::duration& delay)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Timer [{}] set delay [{}]", fmt::ptr(timer), FormatDuration(delay));
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& timerInfo = FindTimer(timer);
    if (!timerInfo)
    {
        WARNING_LOG(m_logger, "Cannot find timer [{}] to stop", fmt::ptr(timer));
        return;
    }
    timerInfo->type = TimerType::SteadyTimer;
    timerInfo->data.steadyTimerData.delay = delay;
}

void DiagnoseImpl::SteadyTimerSetInterval(const Timer* timer, const std::chrono::steady_clock::duration& interval)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Timer [{}] set interval [{}]", fmt::ptr(timer), FormatDuration(interval));
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& timerInfo = FindTimer(timer);
    if (!timerInfo)
    {
        WARNING_LOG(m_logger, "Cannot find timer [{}] to stop", fmt::ptr(timer));
        return;
    }
    timerInfo->type = TimerType::SteadyTimer;
    timerInfo->data.steadyTimerData.interval = interval;
}

void DiagnoseImpl::ExecutorCreated(const Executor* executor, const std::string& name)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Executor [{}]({}) created", fmt::ptr(executor), name);
    // executor创建
    auto executorInfo = std::make_unique<ExecutorInfo>(executor, name);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_executors[executor] = std::move(executorInfo);
}

void DiagnoseImpl::ExecutorReleased(const Executor* executor)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Executor [{}] released", fmt::ptr(executor));
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto executorIter = m_executors.find(executor);
    if (executorIter == m_executors.end())
    {
        WARNING_LOG(m_logger, "Unknown executor [{}] to release", fmt::ptr(executor));
        return;
    }
    // 把timer移动到pending列表
    for (auto& timerIter : executorIter->second->timers)
    {
        m_pendingTimers[timerIter.first] = std::move(timerIter.second);
    }
    executorIter->second->timers.clear();
    // 把task移动到pending列表
    for (auto& taskIter : executorIter->second->tasks)
    {
        m_pendingTasks[taskIter.first] = std::move(taskIter.second);
    }
    executorIter->second->tasks.clear();
    // 从列表中删除executor
    m_executors.erase(executorIter);
}

void DiagnoseImpl::BindTaskWithExecutor(const Task* task, const Executor* executor)
{
    THREADING_DIAGNOSE_LOG(m_logger, "Bind task [{}] with executor [{}]", fmt::ptr(task), fmt::ptr(executor));
    // 把task和executor绑定起来
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto taskInfo = FindTask(task);
    if (!taskInfo)
    {
        WARNING_LOG(m_logger, "Task [{}] info relesed, cannot bind executor [{}]", fmt::ptr(task), fmt::ptr(executor));
        return;
    }
    const auto executorIter = m_executors.find(executor);
    if (executorIter == m_executors.end())
    {
        WARNING_LOG(m_logger, "Unknown executor [{}] cannot bind task [{}]", fmt::ptr(executor), fmt::ptr(task));
        return;
    }
    auto& executorInfo = executorIter->second;
    executorInfo->tasks[task] = taskInfo;
    // 从pending列表中移除task
    m_pendingTasks.erase(task);
}

std::string DiagnoseImpl::GetDiagnoseInfo()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string result;
    // 未加入队列的task
    if (!m_pendingTasks.empty())
    {
        result += "Pending tasks:\n";
        for (const auto& taskIter : m_pendingTasks)
        {
            result += FormatTaskInfo(taskIter.second, "  ");
        }
    }
    // 未加入队列的timer
    if (!m_pendingTimers.empty())
    {
        result += "Pending timers:\n";
        for (const auto& timerIter : m_pendingTimers)
        {
            result += FormatTimerInfo(timerIter.second, "  ");
        }
    }
    // 遍历executor
    for (const auto& executorIter : m_executors)
    {
        result += FormatExecutorInfo(executorIter.second);
    }
    return result;
}

TaskInfoPtr DiagnoseImpl::FindTask(const Task* task)
{
    const auto taskIter = m_tasks.find(task);
    if (taskIter == m_tasks.end())
    {
        return {};
    }
    return taskIter->second.lock();
}

void DiagnoseImpl::RemoveTimerFrom(const Timer* timer, TimerInfoMap& timers)
{
    // 查找timer
    const auto timerIter = timers.find(timer);
    if (timerIter == timers.end())
    {
        return;
    }
    // 取出timer对应的task
    const auto taskInfo = timerIter->second->task;
    // 移除
    timers.erase(timerIter);
    // 如果task仅剩当前一个引用，则放入pending列表
    if (taskInfo && taskInfo.use_count() == 1)
    {
        m_pendingTasks[taskInfo->task] = taskInfo;
    }
}

TimerInfoPtr& DiagnoseImpl::FindTimer(const Timer* timer)
{
    static TimerInfoPtr nullTimer;
    for (const auto& executorIter : m_executors)
    {
        const auto timerIter = executorIter.second->timers.find(timer);
        if (timerIter != executorIter.second->timers.end())
        {
            return timerIter->second;
        }
    }
    return nullTimer;
}

std::string DiagnoseImpl::GetTaskStat(const Task* task)
{
    const auto taskInfo = FindTask(task);
    if (!taskInfo)
    {
        WARNING_LOG(m_logger, "Task [{}] info relesed, cannot get stat", fmt::ptr(task));
        return {};
    }
    return GetTaskStat(taskInfo);
}

std::string DiagnoseImpl::GetTaskStat(const TaskInfoPtr& taskInfo)
{
    std::string stat = fmt::format(FMT_STRING("current state [{}]"), FormatTaskState(taskInfo->state));
    const auto now = std::chrono::steady_clock::now();
    static const std::chrono::steady_clock::time_point zero{};
    // 启动耗时
    stat += fmt::format(FMT_STRING(", created for [{}]"),
                        FormatDuration((taskInfo->queued > zero ? taskInfo->queued : now) - taskInfo->created));
    // 排队耗时
    if (taskInfo->queued > zero)
    {
        stat += fmt::format(FMT_STRING(", queued for [{}]"),
                            FormatDuration((taskInfo->run > zero ? taskInfo->run : now) - taskInfo->queued));
    }
    // 运行耗时
    if (taskInfo->run > zero)
    {
        stat += fmt::format(FMT_STRING(", run for [{}]"),
                            FormatDuration((taskInfo->finished > zero ? taskInfo->finished : now) - taskInfo->run));
    }
    // 结束耗时
    if (taskInfo->finished > zero)
    {
        stat += fmt::format(FMT_STRING(", finish for [{}]"), FormatDuration(now - taskInfo->finished));
    }
    return stat;
}

std::string DiagnoseImpl::FormatTaskState(TaskState state)
{
    switch (state)
    {
    case TaskState::Created:
        return "Created";
    case TaskState::Queuing:
        return "Queuing";
    case TaskState::Running:
        return "Running";
    case TaskState::Finished:
        return "Finished";
    default:
        break;
    }
    return {};
}

std::string DiagnoseImpl::FormatTimePoint(const std::chrono::system_clock::time_point& timePoint)
{
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
    std::string str(32, '\0');
    std::strftime(&str[0], str.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    return str;
}

std::string DiagnoseImpl::FormatDuration(const std::chrono::steady_clock::duration& duration)
{
    using namespace std::chrono_literals;
    if (duration < 1us)
    {
        return fmt::format(FMT_STRING("{}ns"), std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    }
    if (duration < 1ms)
    {
        return fmt::format(FMT_STRING("{}us"), std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
    }
    if (duration < 1s)
    {
        return fmt::format(FMT_STRING("{}ms"), std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    }
    return fmt::format(FMT_STRING("{}s"), std::chrono::duration_cast<std::chrono::seconds>(duration).count());
}

std::string DiagnoseImpl::FormatTaskInfo(const TaskInfoPtr& taskInfo, const std::string& prefix)
{
    return fmt::format(FMT_STRING("{}Task [{}]({}) {}\n"), prefix, fmt::ptr(taskInfo->task), taskInfo->name,
                       GetTaskStat(taskInfo));
}

std::string DiagnoseImpl::FormatTimerInfo(const TimerInfoPtr& timerInfo, const std::string& prefix)
{
    // timer状态
    auto result = fmt::format(FMT_STRING("{}Timer [{}]({}) {}\n"), prefix, fmt::ptr(timerInfo->timer), timerInfo->name,
                              (timerInfo->started ? "started" : "stopped"));
    // timer参数
    switch (timerInfo->type)
    {
    case TimerType::DeadlineTimer:
        result += fmt::format(FMT_STRING("{}  DeadlineTimer: deadline [{}]\n"), prefix,
                              FormatTimePoint(timerInfo->data.deadlineTimerData.deadline));
        break;
    case TimerType::SteadyTimer:
        result += fmt::format(FMT_STRING("{}  SteadyTimer: delay [{}] interval [{}]\n"), prefix,
                              FormatDuration(timerInfo->data.steadyTimerData.delay),
                              FormatDuration(timerInfo->data.steadyTimerData.interval));
        break;
    case TimerType::Invalid:
    default:
        result += fmt::format(FMT_STRING("{}  Invalid type\n"), prefix);
        break;
    }
    // timer对应task
    if (timerInfo->task)
    {
        result += FormatTaskInfo(timerInfo->task, prefix + "  ");
    }
    return result;
}

std::string DiagnoseImpl::FormatExecutorInfo(const ExecutorInfoPtr& executorInfo)
{
    auto result =
        fmt::format(FMT_STRING("Executor [{}]({}) tasks:\n"), fmt::ptr(executorInfo->executor), executorInfo->name);
    for (const auto& timerIter : executorInfo->timers)
    {
        result += FormatTimerInfo(timerIter.second, "  ");
    }
    for (const auto& taskIter : executorInfo->tasks)
    {
        result += FormatTaskInfo(taskIter.second, "  ");
    }
    return result;
}
} // namespace threading
} // namespace base
} // namespace aqua
