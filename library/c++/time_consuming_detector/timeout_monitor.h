#ifndef __TIMEOUT_MONITOR__
#define __TIMEOUT_MONITOR__
#pragma once

#include "time_consuming_detector.h"

/*
 * @brief 默认的超时监控时长(单位:毫秒)
 */
const auto DEFAULT_TIMEOUT_MONITORING_TIME = std::chrono::milliseconds(50);

/*
 * @brief 超时监控器(注意:非线程安全)
 */
class TimeoutMonitor
{
public:
	TimeoutMonitor(const TimeConsumingDetector::CAPTURE_FUNC& captureFunc, const TimeConsumingDetector::END_FUNC& endFunc,
		const std::chrono::steady_clock::duration& timeout = DEFAULT_TIMEOUT_MONITORING_TIME,
		const std::string& tag = "", const std::string& file = "", int line = 0, const std::string& func = "")
	{
		m_detector = new TimeConsumingDetector(
			nullptr, nullptr,
			[timeout, endFunc](
				const std::string& tag,
				const std::string& file,
				int line,
				const std::string& func,
				const std::chrono::milliseconds& duration)
			{
				if (duration >= timeout)
				{
					if (endFunc)
					{
						endFunc(tag, file, line, func, duration);
					}
				}
			}, tag, file, line, func);
		m_captureFunc = captureFunc;
	}

	~TimeoutMonitor()
	{
		if (m_detector)
		{
			delete m_detector;
			m_detector = nullptr;
		}
	}

	void capture(const std::chrono::steady_clock::duration& timeout = DEFAULT_TIMEOUT_MONITORING_TIME,
		const std::string& subTag = "", const std::string& file = "", int line = 0, const std::string& func = "")
	{
		if (m_detector)
		{
			m_detector->capture(subTag, file, line, func,
				[timeout, captureFunc = m_captureFunc](
					const std::string& tag,
					const std::string& subTag,
					const std::string& file,
					int line,
					const std::string& func,
					const std::chrono::milliseconds& duration)
				{
					if (duration >= timeout)
					{
						if (captureFunc)
						{
							captureFunc(tag, subTag, file, line, func, duration);
						}
					}
				});
		}
	}

private:
	TimeConsumingDetector* m_detector;	/* 检测器 */
	TimeConsumingDetector::CAPTURE_FUNC m_captureFunc;	/* 抓拍函数 */
};

/* 启动超时监控器(非线程安全) */
#define START_TIMEOUT_MONITOR(monitor, captureFunc, endFunc, timeout, tag) \
	TimeoutMonitor monitor(captureFunc, endFunc, timeout, tag, __FILE__, __LINE__, __FUNCTION__);

/* 超时监控抓拍(非线程安全) */
#define TIMEOUT_MONITOR_CAPTURE(monitor, timeout, subTag) \
	monitor.capture(timeout, subTag, __FILE__, __LINE__, __FUNCTION__);

/* 启动简单超时监控器(非线程安全,自动命名变量) */
#define START_SIMPLE_TIMEOUT_MONITOR(endFunc, timeout, tag) \
	START_TIMEOUT_MONITOR(TIME_CONSUMINT_DETECTOR_VAR(_timeoutMonitor_), nullptr, endFunc, timeout, tag)

/* 启动简单超时监控器(非线程安全,自动命名变量,默认指定监控时长) */
#define START_EASY_TIMEOUT_MONITOR(endFunc, tag) START_SIMPLE_TIMEOUT_MONITOR(endFunc, DEFAULT_TIMEOUT_MONITORING_TIME, tag)

#endif /* __TIMEOUT_MONITOR__ */