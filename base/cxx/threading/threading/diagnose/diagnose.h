#pragma once
#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace threading
{
class AsioExecutor;
class Executor;
class FiberExecutor;
class Task;
class ThreadProxy;

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
 * @brief 任务创建回调
 * @param executorName 执行者名称
 * @param maxCount 当前执行者所能允许同时执行的最多任务数
 * @param nowCount 当前执行者中的正在执行的任务数量(不包含当前任务)
 * @param taskName 任务名称
 */
using TaskCreatedCallback =
    std::function<void(const std::string& executorName, size_t maxCount, size_t nowCount, const std::string& taskName)>;

/**
 * @brief 任务(正常)状态回调
 * @param executorName 执行者名称
 * @param threadId 线程ID
 * @param threadName 线程名称
 * @param taskName 任务名称
 * @param prevElapsed 前一个状态耗时
 */
using TaskNormalStateCallback = std::function<void(const std::string& executorName, int threadId, const std::string& threadName,
                                                   const std::string& taskName, const std::chrono::steady_clock::duration& prevElapsed)>;

/**
 * @brief 任务(异常)状态回调
 * @param executorName 执行者名称
 * @param threadId 线程ID
 * @param threadName 线程名称
 * @param taskName 任务名称
 * @param msg 异常消息
 */
using TaskExceptionStateCallback = std::function<void(const std::string& executorName, int threadId, const std::string& threadName,
                                                      const std::string& taskName, const std::string& msg)>;

/**
 * @brief 诊断信息收集模块
 */
class Diagnose final
{
    friend AsioExecutor;
    friend FiberExecutor;
    friend ThreadProxy;

public:
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
	 * @brief 设置打开/关闭诊断功能
	 * @param enable 是否打开: true-打开, false-关闭
	 */
    static void setEnable(bool enable);

    /**
     * @brief 获取任务诊断信息
     * @return 诊断信息列表
     */
    static std::vector<ExecutorDiagnoseInfo> getTaskDiagnoseInfo();

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
};
} // namespace threading
