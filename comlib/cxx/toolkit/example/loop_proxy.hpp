#pragma once
#include <atomic>
#include <functional>

#include "algorithm/squeue/squeue.h"
#include "logger/logger_manager.h"

#define FPS_TIME 17 /* 每帧耗时(毫秒), 以目标60FPS(即1000/60)进行判断 */

/**
 * @brief 逻辑消息
 */
struct LogicMsg
{
    LogicMsg() = default;
    LogicMsg(const std::string& name, const std::function<void()>& handler, bool logFlag = true)
        : name(name), handler(handler), logFlag(logFlag)
    {
    }

    std::string name; /* 名称 */
    std::function<void()> handler = nullptr; /* 处理器 */
    bool logFlag = true; /* 是否打印日志 */
};

/**
 * @brief 循环代理(全局) 
 */
class LoopProxy final
{
public:
    /**
     * @brief 获取单例
     */
    static LoopProxy& getInstance()
    {
        static LoopProxy s_instance;
        return s_instance;
    }

    /**
     * @brief 添加消息
     * @param msg 消息
     */
    void addMsg(const std::shared_ptr<LogicMsg>& msg)
    {
        m_msgQueue.push(msg);
    }

    /**
     * @brief 添加消息
     * @param name 名称
     * @param handler 处理器
     * @param logFlag 是否打印日志
     */
    void addMsg(const std::string& name, const std::function<void()>& handler, bool logFlag = true)
    {
        m_msgQueue.push(std::make_shared<LogicMsg>(name, handler, logFlag));
    }

    /**
     * @brief 循环(外部循环调用)
     * @param func 回调函数
     * @param debugDiagnose 是否调试诊断
     */
    void onLoop(const std::function<void()>& func, bool debugDiagnose)
    {
        if (func)
        {
            auto beg = std::chrono::steady_clock::now();
            func();
            if (debugDiagnose)
            {
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
                if (ms >= FPS_TIME)
                {
                    DEBUG_LOG(m_logger, "主线程, 循环回调, 运行耗时: {} 毫秒", ms);
                }
            }
        }
        tryHandleLogicMsg(debugDiagnose);
    }

private:
    void tryHandleLogicMsg(bool debugDiagnose)
    {
        std::shared_ptr<LogicMsg> msg;
        size_t index = 0, costMs = 0, otherMs = 0;
        while (m_msgQueue.tryPop(msg))
        {
            ++index;
            if (msg && msg->handler)
            {
                if (msg->logFlag)
                {
                    TRACE_LOG(m_logger, "{}", msg->name);
                }
                auto beg = std::chrono::steady_clock::now();
                msg->handler();
                if (debugDiagnose)
                {
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
                    if (ms >= FPS_TIME)
                    {
                        DEBUG_LOG(m_logger, "主线程, [{}]逻辑消息[{}], 运行耗时: {} 毫秒", index, msg->name, ms);
                        costMs += ms;
                    }
                    else
                    {
                        otherMs += ms;
                    }
                }
            }
        }
        if (debugDiagnose)
        {
            if (otherMs >= FPS_TIME)
            {
                DEBUG_LOG(m_logger, "主线程, 总共{}条逻辑消息, {}运行耗时: {} 毫秒", index, costMs > 0 ? "其他" : "", otherMs);
            }
        }
    }

private:
    algorithm::SQueue<std::shared_ptr<LogicMsg>> m_msgQueue; /* 逻辑消息队列 */
    logger::Logger m_logger = logger::LoggerManager::getLogger();
};
