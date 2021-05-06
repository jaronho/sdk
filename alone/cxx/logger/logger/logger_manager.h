#pragma once
#include "logger.h"

#include <fmt/format.h>
#include <mutex>
#include <string>
#include <unordered_map>

#if (1 == ENABLE_LOGGER_DETAIL)
#define TRACE_LOG(logger, f, ...) (logger).trace(__FILE__, __LINE__, __FUNCTION__, fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define DEBUG_LOG(logger, f, ...) (logger).debug(__FILE__, __LINE__, __FUNCTION__, fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define INFO_LOG(logger, f, ...) (logger).info(__FILE__, __LINE__, __FUNCTION__, fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define WARN_LOG(logger, f, ...) (logger).warn(__FILE__, __LINE__, __FUNCTION__, fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define ERROR_LOG(logger, f, ...) (logger).error(__FILE__, __LINE__, __FUNCTION__, fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define FATAL_LOG(logger, f, ...) (logger).fatal(__FILE__, __LINE__, __FUNCTION__, fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#else
#define TRACE_LOG(logger, f, ...) (logger).trace("", 0, "", fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define DEBUG_LOG(logger, f, ...) (logger).debug("", 0, "", fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define INFO_LOG(logger, f, ...) (logger).info("", 0, "", fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define WARN_LOG(logger, f, ...) (logger).warn("", 0, "", fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define ERROR_LOG(logger, f, ...) (logger).error("", 0, "", fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#define FATAL_LOG(logger, f, ...) (logger).fatal("", 0, "", fmt::format(FMT_STRING(f), ##__VA_ARGS__))
#endif

namespace logger
{
/**
 * @brief 日志记录管理器
 */
class LoggerManager final
{
public:
    /**
     * @brief 启动日志
     * @param logPath 日志文件目录
     * @param defLoggerName 默认的日志记录器名称
     * @param defTagName 默认的日志标签
     */
    static void start(const std::string& logPath, const std::string& defLoggerName = "default", const std::string& defTagName = "unknown");

    /**
     * @brief 获取日志记录器
     * @param tagName 日志标签
     * @param loggerName 日志记录器名称
     * @return 日志记录器
     */
    static Logger getLogger(const std::string& tagName, const std::string& loggerName = std::string());

private:
    /**
     * @brief 创建内部日志记录器
     * @param path 日志文件路径
     * @param name 日志记录器名称
     * @return 内部日志记录器
     */
    static InnerLoggerPtr createInnerLogger(const std::string& path, const std::string& name);

private:
    static std::recursive_mutex m_mutex; /* 资源锁 */
    static std::string m_logPath; /* 日志文件路径 */
    static std::unordered_map<std::string, InnerLoggerPtr> m_loggerMap; /* 内部日志记录器映射表 */
    static std::string m_defaultLoggerName; /* 默认日志记录器名称 */
    static std::string m_defaultTagName; /* 默认日志标签名称 */
};
} // namespace logger
