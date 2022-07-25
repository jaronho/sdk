#pragma once
#include "base/cxx/fileparse/fileparse/nlohmann/helper.hpp"
#include "base/cxx/logger/logger_manager.h"
#include "base/cxx/threading/async_proxy.h"
#include "base/cxx/httpclient/httpclient/http_client.h"

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
        /* 任务 */
        threading::Diagnose::setTaskRunningStateCallback([&](const std::string& executorName, int threadId, const std::string& threadName,
                                                             int64_t taskId, const std::string& taskName,
                                                             const std::chrono::steady_clock::duration& prevElapsed) {
            onTaskRunningState(executorName, threadId, threadName, taskId, taskName, prevElapsed);
        });
        threading::Diagnose::setTaskFinishedStateCallback([&](const std::string& executorName, int threadId, const std::string& threadName,
                                                              int64_t taskId, const std::string& taskName,
                                                              const std::chrono::steady_clock::duration& prevElapsed) {
            onTaskFinishedState(executorName, threadId, threadName, taskId, taskName, prevElapsed);
        });
        threading::Diagnose::setTaskExceptionStateCallback(
            [&](const std::string& executorName, int threadId, const std::string& threadName, int64_t taskId, const std::string& taskName,
                const std::string& msg) { onTaskExceptionState(executorName, threadId, threadName, taskId, taskName, msg); });
        /* 结束器 */
        threading::Diagnose::setFinisherCreatedCallback([&](int finisherCount, int64_t taskId, const std::string& taskName) {
            return onFinisherCreated(finisherCount, taskId, taskName);
        });
        threading::Diagnose::setFinisherRunningStateCallback(
            [&](int64_t taskId, const std::string& taskName, const std::chrono::steady_clock::duration& prevElapsed) {
                onFinisherRunningState(taskId, taskName, prevElapsed);
            });
        threading::Diagnose::setFinisherFinishedStateCallback(
            [&](int64_t taskId, const std::string& taskName, const std::chrono::steady_clock::duration& prevElapsed) {
                onFinisherFinishedState(taskId, taskName, prevElapsed);
            });
        threading::Diagnose::setFinisherExceptionStateCallback(
            [&](int64_t taskId, const std::string& taskName, const std::string& msg) { onFinisherExecptionState(taskId, taskName, msg); });
        /* 触发器 */
        threading::Diagnose::setTriggerCreatedCallback([&](int triggerCount, int64_t timerId, const std::string& timerName) {
            return onTriggerCreated(triggerCount, timerId, timerName);
        });
        threading::Diagnose::setTriggerRunningStateCallback(
            [&](int64_t timerId, const std::string& timerName, const std::chrono::steady_clock::duration& prevElapsed) {
                onTriggerRunningState(timerId, timerName, prevElapsed);
            });
        threading::Diagnose::setTriggerFinishedStateCallback(
            [&](int64_t timerId, const std::string& timerName, const std::chrono::steady_clock::duration& prevElapsed) {
                onTriggerFinishedState(timerId, timerName, prevElapsed);
            });
        threading::Diagnose::setTriggerExceptionStateCallback([&](int64_t timerId, const std::string& timerName, const std::string& msg) {
            onTriggerExecptionState(timerId, timerName, msg);
        });
        /* HTTP */
        http::HttpClient::setResponseProcessFinishedStateCallback(
            [&](const std::string& url, int prevElapsed) { onHttpResponseProcessFinishedStateCallback(url, prevElapsed); });
        http::HttpClient::setResponseProcessExceptionStateCallback(
            [&](const std::string& url, const std::string& msg) { onHttpResponseProcessExceptionStateCallback(url, msg); });
    }

private:
    /**
     * @brief 响应任务运行状态
     */
    void onTaskRunningState(const std::string& executorName, int threadId, const std::string& threadName, int64_t taskId,
                            const std::string& taskName, const std::chrono::steady_clock::duration& prevElapsed)
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(prevElapsed).count();
        if (ms > 500)
        {
            DEBUG_LOG(m_logger, "线程[{}, {}], 任务[{}], 等待耗时: {} 毫秒", threadId, threadName, taskName, ms);
        }
    }

    /**
     * @brief 响应任务结束状态
     */
    void onTaskFinishedState(const std::string& executorName, int threadId, const std::string& threadName, int64_t taskId,
                             const std::string& taskName, const std::chrono::steady_clock::duration& prevElapsed)
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(prevElapsed).count();
        if (ms > 500)
        {
            DEBUG_LOG(m_logger, "线程[{}, {}], 任务[{}], 运行耗时: {} 毫秒", threadId, threadName, taskName, ms);
        }
    }

    /**
     * @brief 响应任务异常状态
     */
    void onTaskExceptionState(const std::string& executorName, int threadId, const std::string& threadName, int64_t taskId,
                              const std::string& taskName, const std::string& msg)
    {
        ERROR_LOG(m_logger, "线程[{}, {}], 任务[{}], 异常: {}", threadId, threadName, taskName, msg);
    }

    /**
     * @brief 响应结束器创建
     */
    threading::DiscardType onFinisherCreated(int finisherCount, int64_t taskId, const std::string& taskName)
    {
        if (finisherCount >= 1024)
        {
            WARN_LOG(m_logger, "结束器太多[{}], 请检测主线程是否被阻塞", finisherCount);
            showDiagnoseInfo();
            return threading::DiscardType::discard_oldest;
        }
        return threading::DiscardType::none;
    }

    /**
     * @brief 响应结束器运行状态
     */
    void onFinisherRunningState(int64_t taskId, const std::string& taskName, const std::chrono::steady_clock::duration& prevElapsed)
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(prevElapsed).count();
        if (ms > 500)
        {
            DEBUG_LOG(m_logger, "结束器[{}], 等待耗时: {} 毫秒", taskName, ms);
        }
    }

    /**
     * @brief 响应结束器结束状态
     */
    void onFinisherFinishedState(int64_t taskId, const std::string& taskName, const std::chrono::steady_clock::duration& prevElapsed)
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(prevElapsed).count();
        if (ms > 500)
        {
            DEBUG_LOG(m_logger, "结束器[{}], 运行耗时: {} 毫秒", taskName, ms);
        }
    }

    /**
     * @brief 响应结束器异常状态
     */
    void onFinisherExecptionState(int64_t taskId, const std::string& taskName, const std::string& msg)
    {
        ERROR_LOG(m_logger, "结束器[{}], 异常: {}", taskName, msg);
    }

    /**
     * @brief 响应触发器创建
     */
    threading::DiscardType onTriggerCreated(int triggerCount, int64_t timerId, const std::string& timerName)
    {
        if (triggerCount >= 1024)
        {
            WARN_LOG(m_logger, "触发器太多[{}], 请检测主线程是否被阻塞", triggerCount);
            showDiagnoseInfo();
            return threading::DiscardType::discard_oldest;
        }
        return threading::DiscardType::none;
    }

    /**
     * @brief 响应触发器运行状态
     */
    void onTriggerRunningState(int64_t timerId, const std::string& timerName, const std::chrono::steady_clock::duration& prevElapsed)
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(prevElapsed).count();
        if (ms > 500)
        {
            DEBUG_LOG(m_logger, "触发器[{}], 等待耗时: {} 毫秒", timerName, ms);
        }
    }

    /**
     * @brief 响应触发器结束状态
     */
    void onTriggerFinishedState(int64_t timerId, const std::string& timerName, const std::chrono::steady_clock::duration& prevElapsed)
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(prevElapsed).count();
        if (ms > 500)
        {
            DEBUG_LOG(m_logger, "触发器[{}], 运行耗时: {} 毫秒", timerName, ms);
        }
    }

    /**
     * @brief 响应触发器异常状态
     */
    void onTriggerExecptionState(int64_t timerId, const std::string& timerName, const std::string& msg)
    {
        ERROR_LOG(m_logger, "触发器[{}], 异常: {}", timerName, msg);
    }

    /**
     * @brief 响应HTTP响应处理结束状态
     */
    void onHttpResponseProcessFinishedStateCallback(const std::string& url, int prevElapsed)
    {
        if (prevElapsed > 500)
        {
            DEBUG_LOG(m_logger, "HTTP请求[{}], 响应处理耗时: {} 毫秒", url, prevElapsed);
        }
    }

    /**
     * @brief 响应HTTP响应异常结束状态
     */
    void onHttpResponseProcessExceptionStateCallback(const std::string& url, const std::string& msg)
    {
        ERROR_LOG(m_logger, "HTTP请求[{}], 异常: {}", url, msg);
    }

    /**
     * 显示诊断信息
     */
    void showDiagnoseInfo()
    {
        DEBUG_LOG(m_logger, "-------------------- 显示诊断详情 --------------------")
        auto finisherInfoList = threading::Diagnose::getFinisherDiagnoseInfo();
        nlohmann::json finisherList = nlohmann::json::array();
        for (auto info : finisherInfoList)
        {
            nlohmann::json item;
            item["taskId"] = info.taskId;
            item["taskName"] = info.taskName;
            item["state"] = diagnoseStateDesc(info.state, info.exceptionMsg);
            item["queue"] = std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(info.queue).count()) + " 毫秒";
            if (info.state >= threading::DiagnoseState::running)
            {
                item["run"] = std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(info.run).count()) + " 毫秒";
            }
            if (!info.exceptionMsg.empty())
            {
                item["exception"] = info.exceptionMsg;
            }
            finisherList.emplace_back(item);
        }
        DEBUG_LOG(m_logger, "当前结束器详情(共 {} 个)\n{}", finisherList.size(), finisherList.dump(4));
        auto triggerInfoList = threading::Diagnose::getTriggerDiagnoseInfo();
        nlohmann::json triggerList = nlohmann::json::array();
        for (auto info : triggerInfoList)
        {
            nlohmann::json item;
            item["timerId"] = info.timerId;
            item["timerName"] = info.timerName;
            item["state"] = diagnoseStateDesc(info.state, info.exceptionMsg);
            item["queue"] = std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(info.queue).count()) + " 毫秒";
            if (info.state >= threading::DiagnoseState::running)
            {
                item["run"] = std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(info.run).count()) + " 毫秒";
            }
            if (!info.exceptionMsg.empty())
            {
                item["exception"] = info.exceptionMsg;
            }
            triggerList.emplace_back(item);
        }
        DEBUG_LOG(m_logger, "当前触发器详情(共 {} 个)\n{}", triggerList.size(), triggerList.dump(4));
        DEBUG_LOG(m_logger, "---------------------------------------------")
    }

    std::string diagnoseStateDesc(const threading::DiagnoseState& state, const std::string& exceptionMsg)
    {
        switch (state)
        {
        case threading::DiagnoseState::created:
            return "已创建";
        case threading::DiagnoseState::queuing:
            return "排队中";
        case threading::DiagnoseState::running:
            return "运行中";
        case threading::DiagnoseState::finished:
            if (exceptionMsg.empty())
            {
                return "已完成";
            }
            return "异常";
        }
    }

private:
    logger::Logger m_logger = logger::LoggerManager::getLogger("诊断");
};
