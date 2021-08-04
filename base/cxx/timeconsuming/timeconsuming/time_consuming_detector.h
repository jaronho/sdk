#pragma once
#include <chrono>
#include <functional>
#include <string>

namespace timeconsuming
{
/**
 * @brief 开始回调函数
 * @param tag 标签
 * @param file 触发的文件名
 * @param line 触发的行号
 * @param func 触发的函数名
 */
using BeginFunc = std::function<void(const std::string& tag, const std::string& file, int line, const std::string& func)>;

/**
 * @brief 抓拍回调函数
 * @param tag 标签
 * @param subTag 子标签
 * @param file 触发的文件名
 * @param line 触发的行号
 * @param func 触发的函数名
 * @param duration 耗时
 */
using CaptureFunc = std::function<void(const std::string& tag, const std::string& subTag, const std::string& file, int line,
                                       const std::string& func, const std::chrono::milliseconds& duration)>;

/**
 * @brief 结束回调函数
 * @param tag 标签
 * @param file 触发的文件名
 * @param line 触发的行号
 * @param func 触发的函数名
 * @param duration 耗时
 */
using EndFunc = std::function<void(const std::string& tag, const std::string& file, int line, const std::string& func,
                                   const std::chrono::milliseconds& duration)>;

/**
 * @brief 耗时检测器(注意:非线程安全)
 */
class TimeDetector
{
public:
    /**
	 * @brief 构造函数
	 * @param beginFunc 开始函数
	 * @param captureFunc 抓拍函数
	 * @param endFunc 结束函数
	 * @param tag 标签
	 * @param file 调用的文件名
	 * @param line 调用的行号
	 * @param func 调用的函数名
	 */
    TimeDetector(const BeginFunc& beginFunc, const CaptureFunc& captureFunc, const EndFunc& endFunc, const std::string& tag = "",
                 const std::string& file = "", int line = 0, const std::string& func = "")
        : m_begin(std::chrono::steady_clock::now())
        , m_capture(m_begin)
        , m_captureFunc(captureFunc)
        , m_endFunc(endFunc)
        , m_tag(tag)
        , m_file(file)
        , m_line(line)
        , m_func(func)
    {
        if (beginFunc)
        {
            beginFunc(tag, file, line, func);
        }
    }

    ~TimeDetector()
    {
        if (m_endFunc)
        {
            m_endFunc(m_tag, m_file, m_line, m_func,
                      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_begin));
        }
    }

    /**
	 * @brief 抓拍
	 * @param subTag 子标签
	 * @param file 调用的文件名
	 * @param line 调用的行号
	 * @param func 调用的函数名
	 * @param captureFunc 抓拍函数(若有设置,则回调该函数,否则回调构造时所设置的函数)
	 */
    void capture(const std::string& subTag = "", const std::string& file = "", int line = 0, const std::string& func = "",
                 const CaptureFunc& captureFunc = nullptr)
    {
        std::chrono::steady_clock::time_point preCapture = m_capture;
        m_capture = std::chrono::steady_clock::now();
        std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(m_capture - preCapture);
        if (captureFunc)
        {
            captureFunc(m_tag, subTag, file, line, func, duration);
        }
        else if (m_captureFunc)
        {
            m_captureFunc(m_tag, subTag, file, line, func, duration);
        }
    }

private:
    std::chrono::steady_clock::time_point m_begin; /* 开始时间 */
    std::chrono::steady_clock::time_point m_capture; /* 抓拍时间 */
    CaptureFunc m_captureFunc; /* 抓拍函数 */
    EndFunc m_endFunc; /* 结束函数 */
    std::string m_tag; /* 标签 */
    std::string m_file; /* 开始时调用位置的文件名 */
    int m_line; /* 开始时调用位置的行号 */
    std::string m_func; /* 开始时调用位置的函数名 */
};
} // namespace timeconsuming

#define TIME_CONSUMING_DETECTOR_VAR_NAME_CONNECTION(var1, var2) var1##var2
#define TIME_CONSUMING_DETECTOR_VAR_NAME(var1, var2) TIME_CONSUMING_DETECTOR_VAR_NAME_CONNECTION(var1, var2)
#define TIME_CONSUMING_DETECTOR_VAR(var) TIME_CONSUMING_DETECTOR_VAR_NAME(var, __LINE__)

/* 定义耗时检测器对象(非线程安全) */
#define TIME_CONSUMING_DETECTOR(detector, beginFunc, captureFunc, endFunc, tag) \
    timeconsuming::TimeDetector detector(beginFunc, captureFunc, endFunc, tag, __FILE__, __LINE__, __FUNCTION__);

/* 耗时检测器对象抓拍(非线程安全) */
#define TIME_CONSUMING_DETECTOR_CAPTURE(detector, subTag) detector.capture(subTag, __FILE__, __LINE__, __FUNCTION__);

/* 定义带标识的简单耗时检测器对象(非线程安全,自动命名变量) */
#define SIMPLE_TIME_CONSUMING_DETECTOR(endFunc, tag) \
    TIME_CONSUMING_DETECTOR(TIME_CONSUMING_DETECTOR_VAR(_timeConsumingDetector_), nullptr, nullptr, endFunc, tag)
