#pragma once
#include <fmt/format.h>
#include <mutex>
#include <string>
#include <unordered_map>

#include "logger.h"
#include "logger_define.h"

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
     * @param cfg 配置
     * @param defultTagName 默认的日志标签
     */
    static void start(const LogConfig& cfg, const std::string& defultTagName = "unknown");

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
     * @param cfg 配置
     * @return 内部日志记录器
     */
    static InnerLoggerPtr createInnerLogger(const LogConfig& cfg);

private:
    static std::recursive_mutex m_mutex; /* 资源锁 */
    static LogConfig m_logCfg; /* 日志配置 */
    static std::unordered_map<std::string, InnerLoggerPtr> m_loggerMap; /* 内部日志记录器映射表 */
    static std::string m_defaultTagName; /* 默认日志标签名称 */
};
} // namespace logger
