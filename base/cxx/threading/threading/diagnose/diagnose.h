#pragma once
#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace threading
{
class AsioExecutor;
class AsyncProxy;
class AsyncTask;
class Executor;
class FiberExecutor;
class Task;
class ThreadProxy;
class Timer;

/**
 * @brief 丢弃类型
 */
enum class DiscardType
{
    none, /* 不丢弃 */
    discard_newest, /* 丢弃最新 */
    discard_oldest, /* 丢弃最早 */
    discard_all /* 丢弃所有 */
};

/**
 * @brief 诊断状态
 */
enum class DiagnoseState
{
    created, /* 已创建 */
    queuing, /* 排队中 */
    running, /* 运行中 */
    finished /* 已完成 */
};

/**
 * @brief 任务诊断信息
 */
struct TaskDiagnoseInfo
{
    int64_t taskId = 0; /* 任务ID */
    std::string taskName; /* 任务名称 */
    int64_t threadId = 0; /* 任务所在线程ID */
    std::string threadName; /* 任务所在线程名称 */
    DiagnoseState state = DiagnoseState::created; /* 状态 */
    std::chrono::steady_clock::duration queue; /* 排队耗时 */
    std::chrono::steady_clock::duration run; /* 运行耗时 */
    std::string exceptionMsg; /* 异常信息 */
};

/**
 * @brief 执行者(线程池)诊断信息
 */
struct ExecutorDiagnoseInfo
{
    std::string name; /* 执行者(线程池)名称 */
    std::vector<TaskDiagnoseInfo> taskInfoList; /* 任务信息列表 */
};

/**
 * @brief 结束器诊断信息
 */
struct FinisherDiagnoseInfo
{
    int64_t taskId = 0; /* 任务ID */
    std::string taskName; /* 任务名称 */
    DiagnoseState state = DiagnoseState::created; /* 状态 */
    std::chrono::steady_clock::duration queue; /* 排队耗时 */
    std::chrono::steady_clock::duration run; /* 运行耗时 */
    std::string exceptionMsg; /* 异常信息 */
};

/**
 * @brief 触发器诊断信息
 */
struct TriggerDiagnoseInfo
{
    int64_t timerId = 0; /* 定时器ID */
    std::string timerName; /* 定时器名称 */
    DiagnoseState state = DiagnoseState::created; /* 状态 */
    std::chrono::steady_clock::duration queue; /* 排队耗时 */
    std::chrono::steady_clock::duration run; /* 运行耗时 */
    std::string exceptionMsg; /* 异常信息 */
};

/**
 * @brief 任务创建回调
 * @param executorName 执行者名称
 * @param taskCount 当前执行者中的任务数量(包含当前新绑定的)
 * @param taskId 任务ID
 * @param taskName 任务名称
 */
using TaskCreatedCallback =
    std::function<void(const std::string& executorName, int taskCount, int64_t taskId, const std::string& taskName)>;

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
 * @brief 结束器创建回调
 * @param finisherCount 当前触发器数量(不包含新创建的)
 * @param taskId 任务ID
 * @param taskName 任务名称
 * @return 丢弃类型
 */
using FinisherCreateCallback = std::function<DiscardType(int finisherCount, int64_t taskId, const std::string& taskName)>;

/**
 * @brief 结束器(正常)状态回调
 * @param taskId 任务ID
 * @param taskName 任务名称
 * @param prevElapsed 前一个状态耗时
 */
using FinisherNormalStateCallback =
    std::function<void(int64_t taskId, const std::string& taskName, const std::chrono::steady_clock::duration& prevElapsed)>;

/**
 * @brief 结束器(异常)状态回调
 * @param taskId 任务ID
 * @param taskName 任务名称
 * @param msg 异常消息
 */
using FinisherExceptionStateCallback = std::function<void(int64_t taskId, const std::string& taskName, const std::string& msg)>;

/**
 * @brief 触发器创建回调
 * @param triggerCount 当前触发器数量(不包含新创建的)
 * @param timerId 定时器ID
 * @param timerName 定时器名称
 * @return 丢弃类型
 */
using TriggerCreateCallback = std::function<DiscardType(int triggerCount, int64_t timerId, const std::string& timerName)>;

/**
 * @brief 触发器(正常)状态回调
 * @param timerId 定时器ID
 * @param timerName 定时器名称
 * @param prevElapsed 前一个状态耗时
 */
using TriggerNormalStateCallback =
    std::function<void(int64_t timerId, const std::string& timerName, const std::chrono::steady_clock::duration& prevElapsed)>;

/**
 * @brief 触发器(异常)状态回调
 * @param timerId 定时器ID
 * @param timerName 定时器名称
 * @param msg 异常消息
 */
using TriggerExceptionStateCallback = std::function<void(int64_t timerId, const std::string& timerName, const std::string& msg)>;

/**
 * @brief 诊断信息收集模块
 */
class Diagnose final
{
    friend AsioExecutor;
    friend AsyncProxy;
    friend AsyncTask;
    friend FiberExecutor;
    friend ThreadProxy;
    friend Timer;

public:
    /**
	 * @brief 设置开启诊断功能, 注意: 如果要开启则必须要先于其他所有线程组件调用
	 */
    static void setEnable();

    /**
	 * @brief 设置任务创建回调
	 * @param createdCb 创建回调
	 */
    static void setTaskCreatedCallback(const TaskCreatedCallback& createdCb);

    /**
	 * @brief 设置任务运行状态回调
	 * @param stateCb 状态回调
	 */
    static void setTaskRunningStateCallback(const TaskNormalStateCallback& stateCb);

    /**
	 * @brief 设置任务结束状态回调
	 * @param stateCb 状态回调
	 */
    static void setTaskFinishedStateCallback(const TaskNormalStateCallback& stateCb);

    /**
	 * @brief 设置任务异常状态回调
	 * @param stateCb 状态回调
	 */
    static void setTaskExceptionStateCallback(const TaskExceptionStateCallback& stateCb);

    /**
	 * @brief 设置结束器创建回调
	 * @param createdCb 创建回调
	 */
    static void setFinisherCreatedCallback(const FinisherCreateCallback& createdCb);

    /**
	 * @brief 设置结束器运行状态回调
	 * @param stateCb 状态回调
	 */
    static void setFinisherRunningStateCallback(const FinisherNormalStateCallback& stateCb);

    /**
	 * @brief 设置结束器结束状态回调
	 * @param stateCb 状态回调
	 */
    static void setFinisherFinishedStateCallback(const FinisherNormalStateCallback& stateCb);

    /**
	 * @brief 设置结束器异常状态回调
	 * @param stateCb 状态回调
	 */
    static void setFinisherExceptionStateCallback(const FinisherExceptionStateCallback& stateCb);

    /**
	 * @brief 设置触发器创建回调
	 * @param createdCb 创建回调
	 */
    static void setTriggerCreatedCallback(const TriggerCreateCallback& createdCb);

    /**
	 * @brief 设置触发器运行状态回调
	 * @param stateCb 状态回调
	 */
    static void setTriggerRunningStateCallback(const TriggerNormalStateCallback& stateCb);

    /**
	 * @brief 设置触发器结束状态回调
	 * @param stateCb 状态回调
	 */
    static void setTriggerFinishedStateCallback(const TriggerNormalStateCallback& stateCb);

    /**
	 * @brief 设置触发器异常状态回调
	 * @param stateCb 状态回调
	 */
    static void setTriggerExceptionStateCallback(const TriggerExceptionStateCallback& stateCb);

    /**
     * @brief 获取任务诊断信息
     * @return 诊断信息列表
     */
    static std::vector<ExecutorDiagnoseInfo> getTaskDiagnoseInfo();

    /**
     * @brief 获取结束器诊断信息
     * @return 诊断信息列表
     */
    static std::vector<FinisherDiagnoseInfo> getFinisherDiagnoseInfo();

    /**
     * @brief 获取触发器诊断信息
     * @return 诊断信息列表
     */
    static std::vector<TriggerDiagnoseInfo> getTriggerDiagnoseInfo();

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
     * @brief 响应任务创建(模块内部接口)
     * @param executor 执行者
     * @param task 任务
     */
    static void onTaskCreated(const Executor* executor, const Task* task);

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
     * @brief 响应结束器创建(模块内部接口)
     * @param finisherCount 当前已创建的结束器数量(不包含当前新创建的)
     * @param oldestFinisherId 最早的触发ID
     * @param finisherId 结束器ID
     * @param task 任务
     */
    static DiscardType onFinisherCreated(int finisherCount, int64_t oldestFinisherId, int64_t finisherId, const AsyncTask* task);

    /**
     * @brief 结束结束器开始运行(模块内部接口)
     * @param finisherId 结束器ID
     * @param task 任务
     */
    static void onFinisherRunning(int64_t finisherId, const AsyncTask* task);

    /**
     * @brief 响应结束器运行结束(模块内部接口)
     * @param finisherId 结束器ID
     * @param task 任务
     */
    static void onFinisherFinished(int64_t finisherId, const AsyncTask* task);

    /**
     * @brief 响应结束器运行异常(模块内部接口)
     * @param finisherId 结束器ID
     * @param task 任务
     * @param msg 异常消息
     */
    static void onFinisherException(int64_t finisherId, const AsyncTask* task, const std::string& msg);

    /**
     * @brief 响应触发器创建(模块内部接口)
     * @param triggerCount 当前已创建的触发器数量(不包含当前新创建的)
     * @param oldestTriggerId 最早的触发器ID
     * @param triggerId 触发器ID
     * @param timer 定时器
     */
    static DiscardType onTriggerCreated(int triggerCount, int64_t oldestTriggerId, int64_t triggerId, const Timer* timer);

    /**
     * @brief 响应触发器开始运行(模块内部接口)
     * @param triggerId 触发器ID
     * @param timer 定时器
     */
    static void onTriggerRunning(int64_t triggerId, const Timer* timer);

    /**
     * @brief 响应触发器运行结束(模块内部接口)
     * @param triggerId 触发器ID
     * @param timer 定时器
     */
    static void onTriggerFinished(int64_t triggerId, const Timer* timer);

    /**
     * @brief 响应触发器运行异常(模块内部接口)
     * @param triggerId 触发器ID
     * @param timer 定时器
     * @param msg 异常消息
     */
    static void onTriggerException(int64_t triggerId, const Timer* timer, const std::string& msg);

    /**
     * @brief 响应定时器销毁(模块内部接口)
     * @param timer 定时器
     */
    static void onTimerDestroy(const Timer* timer);
};
} // namespace threading
