#pragma once
#include <chrono>
#include <functional>
#include <string>

namespace threading
{
class Executor;
class Task;
class AsioExecutor;
class FiberExecutor;
class ThreadProxy;
class Timer;

/**
 * @brief 定时器触发丢弃类型
 */
enum class TimerTriggerDiscard
{
    none, /* 不丢弃 */
    discard_newest, /* 丢弃最新 */
    discard_oldest, /* 丢弃最早 */
    discard_all /* 丢弃所有 */
};

/**
 * @brief 任务绑定到执行者回调
 * @param executorName 执行者名称
 * @param taskCount 当前执行者中的任务数量(包含当前新绑定的)
 * @param taskId 任务ID
 * @param taskName 任务名称
 */
using TaskBindCallback = std::function<void(const std::string& executorName, int taskCount, int64_t taskId, const std::string& taskName)>;

/**
 * @brief 任务(正常)状态回调
 * @param executorName 执行者名称
 * @param threadId 线程ID
 * @param threadName 线程名称
 * @param taskId 任务ID
 * @param taskName 任务名称
 * @param prevElapsed 前一个状态耗时
 */
using TaskNormalStateCallback =
    std::function<void(const std::string& executorName, int threadId, const std::string& threadName, int64_t taskId,
                       const std::string& taskName, const std::chrono::steady_clock::duration& prevElapsed)>;

/**
 * @brief 任务(异常)状态回调
 * @param executorName 执行者名称
 * @param threadId 线程ID
 * @param threadName 线程名称
 * @param taskId 任务ID
 * @param taskName 任务名称
 * @param msg 异常消息
 */
using TaskExceptionStateCallback = std::function<void(const std::string& executorName, int threadId, const std::string& threadName,
                                                      int64_t taskId, const std::string& taskName, const std::string& msg)>;

/**
 * @brief 定时器触发回调
 * @param triggerCount 当前要触发的定时器数量(不包含当前新触发的)
 * @param timerId 定时器ID
 * @param timerName 定时器名称
 * @return 丢弃类型
 */
using TimerTriggerCallback = std::function<TimerTriggerDiscard(int triggerCount, int64_t timerId, const std::string& timerName)>;

/**
 * @brief 定时器触发(正常)状态回调
 * @param timerId 定时器ID
 * @param timerName 定时器名称
 * @param prevElapsed 前一个状态耗时
 */
using TimerTriggerNormalStateCallback =
    std::function<void(int64_t timerId, const std::string& timerName, const std::chrono::steady_clock::duration& prevElapsed)>;

/**
 * @brief 定时器触发(异常)状态回调
 * @param timerId 定时器ID
 * @param timerName 定时器名称
 * @param msg 异常消息
 */
using TimerTriggerExceptionStateCallback = std::function<void(int64_t timerId, const std::string& timerName, const std::string& msg)>;

/**
 * @brief 诊断信息收集模块
 */
class Diagnose final
{
    friend AsioExecutor;
    friend FiberExecutor;
    friend ThreadProxy;
    friend Timer;

public:
    /**
	 * @brief 设置开启诊断功能, 注意: 如果要开启则必须要先于其他所有线程组件调用
	 */
    static void setEnable();

    /**
	 * @brief 设置任务绑定回调
	 * @param taskBindCb 任务绑定回调
	 */
    static void setTaskBindCallback(const TaskBindCallback& taskBindCb);

    /**
	 * @brief 设置任务运行状态回调
	 * @param taskStateCb 任务状态回调
	 */
    static void setTaskRunningStateCallback(const TaskNormalStateCallback& taskStateCb);

    /**
	 * @brief 设置任务结束状态回调
	 * @param taskStateCb 任务状态回调
	 */
    static void setTaskFinishedStateCallback(const TaskNormalStateCallback& taskStateCb);

    /**
	 * @brief 设置任务异常状态回调
	 * @param taskStateCb 任务状态回调
	 */
    static void setTaskExceptionStateCallback(const TaskExceptionStateCallback& taskStateCb);

    /**
	 * @brief 设置定时器触发回调
	 * @param timerTriggerCb 定时器触发回调
	 */
    static void setTimerTriggerCallback(const TimerTriggerCallback& timerTriggerCb);

    /**
	 * @brief 设置定时器触发运行状态回调
	 * @param timerTriggerStateCb 定时器触发状态回调
	 */
    static void setTimerTriggerRunningStateCallback(const TimerTriggerNormalStateCallback& timerTriggerStateCb);

    /**
	 * @brief 设置定时器触发结束状态回调
	 * @param timerTriggerStateCb 定时器触发状态回调
	 */
    static void setTimerTriggerFinishedStateCallback(const TimerTriggerNormalStateCallback& timerTriggerStateCb);

    /**
	 * @brief 设置定时器触发异常状态回调
	 * @param timerTriggerStateCb 定时器触发状态回调
	 */
    static void setTimerTriggerExceptionStateCallback(const TimerTriggerExceptionStateCallback& timerTriggerStateCb);

    /**
     * @brief 获取任务诊断信息, 格式例如:
        [ // 执行者(线程池)列表
            {
                "name":"workers", // 执行者(线程池)名称
                "count":16, // 线程池中所有未结束的任务数量
                "task": // 任务列表: 任务ID/名称, 所在线程ID/名称, 状态(created,queuing,running,finished), 排队(queue)/执行(run)耗时(单位: ns,us,ms)
                [
                    // 任务信息: 任务ID/名称, 所在线程ID/名称, 状态(created,queuing,running,finished), 各状态耗时(单位: ns,us,ms)
                    {"id":6770523678363649,"name":"task_3","thread_id":19288,"thread_name":"workers-2","state":"running","queue":"71 us","run":"4004 ms"},
                    {"id":6770523674263553,"name":"task_1","thread_id":24100,"thread_name":"workers-1","state":"running","queue":"67 us","run":"5005 ms"},
                    {"id":6770523688615937,"name":"task_8","thread_id":0,"thread_name":"","state":"queuing","queue":"1502 ms"},
                    {"id":6770523686563841,"name":"task_7","thread_id":0,"thread_name":"","state":"queuing","queue":"2002 ms"}
                ]
            }
        ]
     * @return 诊断信息字符串(JSON)
     */
    static std::string getTaskDiagnoseInfo();

    /**
     * @brief 获取定时器诊断信息, 格式例如:
        [ // 定时器触发列表
            // 触发信息: 定时器ID/名称, 状态(triggered,queuing,running,finished), 排队(queue)/执行(run)耗时(单位: ns,us,ms)
            {"id":6770523674243072,"name":"timer_1","state":"running","queue":"516 ms","run":"5 ms"},
            {"id":6770523674245234,"name":"timer_2","state":"running","queue":"900 ms","run":"1002 ms"}
        ]
     * @return 诊断信息字符串(JSON)
     */
    static std::string getTimerDiagnoseInfo();

protected:
    /**
     * @brief 响应执行者被创建(模块内部接口)
     * @param executor 执行者
     */
    static void onExecutorCreated(const Executor* executor);

    /**
     * @brief 响应执行者被销毁(模块内部接口)
     * @param executor 执行者
     */
    static void onExecutorDestroyed(const Executor* executor);

    /**
     * @brief 把任务绑定到执行者(模块内部接口)
     * @param task 任务
     * @param executor 执行者
     */
    static void bindTaskToExecutor(const Task* task, const Executor* executor);

    /**
     * @brief 响应任务开始运行(模块内部接口)
     * @param threadId 线程id
     * @param threadName 线程名称
     * @param task 任务
     */
    static void onTaskRunning(int threadId, const std::string& threadName, const Task* task);

    /**
     * @brief 响应任务运行结束(模块内部接口)
     * @param threadId 线程id
     * @param threadName 线程名称
     * @param task 任务
     */
    static void onTaskFinished(int threadId, const std::string& threadName, const Task* task);

    /**
     * @brief 响应任务运行异常(模块内部接口)
     * @param threadId 线程id
     * @param threadName 线程名称
     * @param task 任务
     * @param msg 异常消息
     */
    static void onTaskException(int threadId, const std::string& threadName, const Task* task, const std::string& msg);

    /**
     * @brief 响应定时器触发(模块内部接口)
     * @param triggerCount 当前要触发的定时器数量(不包含当前新触发的)
     * @param oldestTriggerId 最早的触发ID
     * @param triggerId 触发ID
     * @param timer 定时器
     */
    static TimerTriggerDiscard onTimerTrigger(int triggerCount, int64_t oldestTriggerId, int64_t triggerId, const Timer* timer);

    /**
     * @brief 响应定时器触发开始运行(模块内部接口)
     * @param triggerId 触发ID
     * @param timer 定时器
     */
    static void onTimerTriggerRunning(int64_t triggerId, const Timer* timer);

    /**
     * @brief 响应定时器触发运行结束(模块内部接口)
     * @param triggerId 触发ID
     * @param timer 定时器
     */
    static void onTimerTriggerFinished(int64_t triggerId, const Timer* timer);

    /**
     * @brief 响应定时器触发运行异常(模块内部接口)
     * @param triggerId 触发ID
     * @param timer 定时器
     * @param msg 异常消息
     */
    static void onTimerTriggerException(int64_t triggerId, const Timer* timer, const std::string& msg);

    /**
     * @brief 响应定时器销毁(模块内部接口)
     * @param timer 定时器
     */
    static void onTimerDestroy(const Timer* timer);
};
} // namespace threading
