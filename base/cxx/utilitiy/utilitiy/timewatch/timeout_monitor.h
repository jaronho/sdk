#pragma once
#include "time_watcher.h"

namespace utilitiy
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
    TimeoutMonitor(const WatchFunc& captureFunc, const EndFunc& endFunc, long long timeout = DEFAULT_TIMEOUT, const std::string& tag = "");
    TimeoutMonitor() = default;

    ~TimeoutMonitor();

    /**
	 * @brief 抓拍
     * @param timeout 超时(毫秒)
	 * @param subTag 子标签
	 */
    void capture(long long timeout = DEFAULT_TIMEOUT, const std::string& subTag = "");

private:
    TimeWatcher* m_watcher; /* 检测器 */
    WatchFunc m_captureFunc; /* 抓拍函数 */
};
} // namespace utilitiy

/* 创建超时监控器(非线程安全) */
#define MAKE_TIMEOUT_MONITOR(monitor, captureFunc, endFunc, timeout, tag) \
    utilitiy::TimeoutMonitor monitor(captureFunc, endFunc, timeout, tag)

/* 超时监控器抓拍(非线程安全) */
#define TIMEOUT_MONITOR_CAPTURE(monitor, timeout, subTag) monitor.capture(timeout, subTag)

/* 启动简单超时监控器(非线程安全,自动命名变量) */
#define START_SIMPLE_TIMEOUT_MONITOR(endFunc, timeout) \
    MAKE_TIMEOUT_MONITOR(TIME_WATCHER_VAR(_timeoutMonitor_), nullptr, endFunc, timeout, "")
