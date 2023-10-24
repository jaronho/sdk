#pragma once
#include "logger/logger_manager.h"
#include "threading/diagnose/diagnose.h"

/**
 * @brief 诊断器
 */
class Diagnostor final
{
public:
    static Diagnostor& getInstance()
    {
        static Diagnostor s_instance;
        return s_instance;
    }

    /**
     * @brief 启动
     */
    void start()
    {
        threading::Diagnose::setEnable();
        threading::Diagnose::setTaskRunningStateCallback([&](const std::string& executorName, int threadId, const std::string& threadName,
                                                             const std::string& taskName,
                                                             const std::chrono::steady_clock::duration& prevElapsed) {
            onTaskRunningState(executorName, threadId, threadName, taskName, prevElapsed);
        });
        threading::Diagnose::setTaskFinishedStateCallback([&](const std::string& executorName, int threadId, const std::string& threadName,
                                                              const std::string& taskName,
                                                              const std::chrono::steady_clock::duration& prevElapsed) {
            onTaskFinishedState(executorName, threadId, threadName, taskName, prevElapsed);
        });
        threading::Diagnose::setTaskExceptionStateCallback(
            [&](const std::string& executorName, int threadId, const std::string& threadName, const std::string& taskName,
                const std::string& msg) { onTaskExceptionState(executorName, threadId, threadName, taskName, msg); });
    }

private:
    /**
     * @brief 响应任务运行状态
     */
    void onTaskRunningState(const std::string& executorName, int threadId, const std::string& threadName, const std::string& taskName,
                            const std::chrono::steady_clock::duration& prevElapsed)
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(prevElapsed).count();
        if (ms >= 100)
        {
            DEBUG_LOG(m_logger, "线程[{}, {}], 任务[{}], 等待耗时: {} 毫秒", threadId, threadName, taskName, ms);
        }
    }

    /**
     * @brief 响应任务结束状态
     */
    void onTaskFinishedState(const std::string& executorName, int threadId, const std::string& threadName, const std::string& taskName,
                             const std::chrono::steady_clock::duration& prevElapsed)
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(prevElapsed).count();
        if (ms >= 100)
        {
            DEBUG_LOG(m_logger, "线程[{}, {}], 任务[{}], 运行耗时: {} 毫秒", threadId, threadName, taskName, ms);
        }
    }

    /**
     * @brief 响应任务异常状态
     */
    void onTaskExceptionState(const std::string& executorName, int threadId, const std::string& threadName, const std::string& taskName,
                              const std::string& msg)
    {
        ERROR_LOG(m_logger, "线程[{}, {}], 任务[{}], 异常: {}", threadId, threadName, taskName, msg);
    }

private:
    logger::Logger m_logger = logger::LoggerManager::getLogger("诊断");
};
