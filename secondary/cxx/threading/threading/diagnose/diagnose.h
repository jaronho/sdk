#pragma once
#include "../task/executor.h"
#include "../task/task.h"
#include "diagnose_info.h"

#include <functional>
#include <string>

namespace threading
{
class AsioExecutor;
class FiberExecutor;
class ThreadProxy;

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
	 * @brief 设置日志函数
	 * @param logFunc 日志函数
	 */
    static void setLogFunc(const std::function<void(const std::string&)>& logFunc);

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
    static void onTaskRunning(int64_t threadId, const std::string& threadName, const Task* task);

    /**
     * @brief 响应任务运行结束(模块内部接口)
     * @param threadId 线程id
     * @param threadName 线程名称
     * @param task 任务
     */
    static void onTaskFinished(int64_t threadId, const std::string& threadName, const Task* task);

    /**
     * @brief 响应任务运行异常(模块内部接口)
     * @param threadId 线程id
     * @param threadName 线程名称
     * @param task 任务
     * @param msg 异常消息
     */
    static void onTaskException(int64_t threadId, const std::string& threadName, const Task* task, const std::string& msg);

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
