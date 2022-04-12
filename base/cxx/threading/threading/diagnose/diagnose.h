#pragma once
#include <functional>
#include <string>

#include "../task/executor.h"
#include "../task/task.h"
#include "diagnose_info.h"

namespace threading
{
class AsioExecutor;
class FiberExecutor;
class ThreadProxy;

/**
 * @brief 任务绑定到执行者回调
 * @param executorName 执行者名称
 * @param taskName 任务名称
 */
using TaskBindCallback = std::function<void(const std::string& executorName, const std::string& taskName)>;

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
     * @brief 获取诊断信息
     * @return 诊断信息字符串
     */
    static std::string getDiagnoseInfo();

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

private:
    static void printLog(const std::string& msg);
    static TaskInfoPtr getTaskInfo(const Task* task);
    static void delTaskInfo(const Task* task);
    static std::string taskStateToString(const Task::State& state);
    static std::string durationToString(const std::chrono::steady_clock::duration& duration);
    static std::string taskInfoToString(const TaskInfoPtr& taskInfo, bool showRun = true);
    static std::string executorInfoToString(const ExecutorInfoPtr& executorInfo);
};
} // namespace threading
