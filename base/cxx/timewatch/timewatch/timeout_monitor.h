#pragma once
#include "time_watcher.h"

namespace timewatch
{
/**
 * @brief 默认的超时监控时长(单位:毫秒)
 */
static const auto DEFAULT_TIMEOUT = 50;

/**
 * @brief 超时监控器(注意:非线程安全)
 */
class TimeoutMonitor
{
public:
    /**
	 * @brief 构造函数
	 * @param captureFunc 抓拍函数
	 * @param endFunc 结束函数
     * @param timeout 超时(毫秒)
	 * @param tag 标签
	 */
    TimeoutMonitor(const WatchFunc& captureFunc, const EndFunc& endFunc, long long timeout = DEFAULT_TIMEOUT, const std::string& tag = "")
    {
        m_watcher = new TimeWatcher(
            nullptr,
            [timeout, endFunc](const std::string& tag, long long elapsed) {
                if (elapsed >= timeout)
                {
                    if (endFunc)
                    {
                        endFunc(tag, elapsed);
                    }
                }
            },
            tag);
        m_captureFunc = captureFunc;
    }
    TimeoutMonitor() = default;

    ~TimeoutMonitor()
    {
        if (m_watcher)
        {
            delete m_watcher;
            m_watcher = nullptr;
        }
    }

    /**
	 * @brief 抓拍
     * @param timeout 超时(毫秒)
	 * @param subTag 子标签
	 */
    void capture(long long timeout = DEFAULT_TIMEOUT, const std::string& subTag = "")
    {
        if (m_watcher)
        {
            m_watcher->watch(subTag,
                             [timeout, captureFunc = m_captureFunc](const std::string& tag, const std::string& subTag, long long elapsed) {
                                 if (elapsed >= timeout)
                                 {
                                     if (captureFunc)
                                     {
                                         captureFunc(tag, subTag, elapsed);
                                     }
                                 }
                             });
        }
    }

private:
    TimeWatcher* m_watcher; /* 检测器 */
    WatchFunc m_captureFunc; /* 抓拍函数 */
};
} // namespace timewatch

/* 创建超时监控器(非线程安全) */
#define MAKE_TIMEOUT_MONITOR(monitor, captureFunc, endFunc, timeout, tag) \
    timewatch::TimeoutMonitor monitor(captureFunc, endFunc, timeout, tag);

/* 超时监控器抓拍(非线程安全) */
#define TIMEOUT_MONITOR_CAPTURE(monitor, timeout, subTag) monitor.capture(timeout, subTag);

/* 启动简单超时监控器(非线程安全,自动命名变量) */
#define START_SIMPLE_TIMEOUT_MONITOR(endFunc, timeout) \
    MAKE_TIMEOUT_MONITOR(TIME_WATCHER_VAR(_timeoutMonitor_), nullptr, endFunc, timeout, "")
