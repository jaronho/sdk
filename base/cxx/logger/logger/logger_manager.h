#pragma once
#include <fmt/format.h>
#include <mutex>
#include <string>
#include <unordered_map>

#include "logger.h"
#include "logger_define.h"

#define __LOG_IMPL(logger, func, filename, lineNumber, funcName, f, ...) \
    try \
    { \
        (logger).func(filename, lineNumber, funcName, fmt::format(FMT_STRING(f), ##__VA_ARGS__)); \
    } \
    catch (const std::exception& e) \
    { \
        (logger).warn(filename, lineNumber, funcName, e.what()); \
    } \
    catch (...) \
    { \
        (logger).warn(filename, lineNumber, funcName, "unknown exception"); \
    }

#ifdef _WIN32
#define __FN(x) strrchr(x, '\\') ? strrchr(x, '\\') + 1 : x
#else
#define __FN(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x
#endif

#if (1 == ENABLE_LOGGER_DETAIL) /* 显示: [文件名(全路径) 行号 函数名] 内容 */
#define TRACE_LOG(logger, f, ...) __LOG_IMPL(logger, trace, __FILE__, __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define DEBUG_LOG(logger, f, ...) __LOG_IMPL(logger, debug, __FILE__, __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define INFO_LOG(logger, f, ...) __LOG_IMPL(logger, info, __FILE__, __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define WARN_LOG(logger, f, ...) __LOG_IMPL(logger, warn, __FILE__, __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define ERROR_LOG(logger, f, ...) __LOG_IMPL(logger, error, __FILE__, __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define FATAL_LOG(logger, f, ...) __LOG_IMPL(logger, fatal, __FILE__, __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#elif (2 == ENABLE_LOGGER_DETAIL) /* 显示: [文件名 行号 函数名] 内容 */
#define TRACE_LOG(logger, f, ...) __LOG_IMPL(logger, trace, __FN(__FILE__), __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define DEBUG_LOG(logger, f, ...) __LOG_IMPL(logger, debug, __FN(__FILE__), __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define INFO_LOG(logger, f, ...) __LOG_IMPL(logger, info, __FN(__FILE__), __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define WARN_LOG(logger, f, ...) __LOG_IMPL(logger, warn, __FN(__FILE__), __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define ERROR_LOG(logger, f, ...) __LOG_IMPL(logger, error, __FN(__FILE__), __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#define FATAL_LOG(logger, f, ...) __LOG_IMPL(logger, fatal, __FN(__FILE__), __LINE__, __FUNCTION__, f, ##__VA_ARGS__)
#elif (3 == ENABLE_LOGGER_DETAIL) /* 显示: [文件名(全路径) 行号] 内容 */
#define TRACE_LOG(logger, f, ...) __LOG_IMPL(logger, trace, __FILE__, __LINE__, "", f, ##__VA_ARGS__)
#define DEBUG_LOG(logger, f, ...) __LOG_IMPL(logger, debug, __FILE__, __LINE__, "", f, ##__VA_ARGS__)
#define INFO_LOG(logger, f, ...) __LOG_IMPL(logger, info, __FILE__, __LINE__, "", f, ##__VA_ARGS__)
#define WARN_LOG(logger, f, ...) __LOG_IMPL(logger, warn, __FILE__, __LINE__, "", f, ##__VA_ARGS__)
#define ERROR_LOG(logger, f, ...) __LOG_IMPL(logger, error, __FILE__, __LINE__, "", f, ##__VA_ARGS__)
#define FATAL_LOG(logger, f, ...) __LOG_IMPL(logger, fatal, __FILE__, __LINE__, "", f, ##__VA_ARGS__)
#elif (4 == ENABLE_LOGGER_DETAIL) /* 显示: [文件名 行号] 内容 */
#define TRACE_LOG(logger, f, ...) __LOG_IMPL(logger, trace, __FN(__FILE__), __LINE__, "", f, ##__VA_ARGS__)
#define DEBUG_LOG(logger, f, ...) __LOG_IMPL(logger, debug, __FN(__FILE__), __LINE__, "", f, ##__VA_ARGS__)
#define INFO_LOG(logger, f, ...) __LOG_IMPL(logger, info, __FN(__FILE__), __LINE__, "", f, ##__VA_ARGS__)
#define WARN_LOG(logger, f, ...) __LOG_IMPL(logger, warn, __FN(__FILE__), __LINE__, "", f, ##__VA_ARGS__)
#define ERROR_LOG(logger, f, ...) __LOG_IMPL(logger, error, __FN(__FILE__), __LINE__, "", f, ##__VA_ARGS__)
#define FATAL_LOG(logger, f, ...) __LOG_IMPL(logger, fatal, __FN(__FILE__), __LINE__, "", f, ##__VA_ARGS__)
#elif (5 == ENABLE_LOGGER_DETAIL) /* 显示: [函数名] 内容 */
#define TRACE_LOG(logger, f, ...) __LOG_IMPL(logger, trace, "", 0, __FUNCTION__, f, ##__VA_ARGS__)
#define DEBUG_LOG(logger, f, ...) __LOG_IMPL(logger, debug, "", 0, __FUNCTION__, f, ##__VA_ARGS__)
#define INFO_LOG(logger, f, ...) __LOG_IMPL(logger, info, "", 0, __FUNCTION__, f, ##__VA_ARGS__)
#define WARN_LOG(logger, f, ...) __LOG_IMPL(logger, warn, "", 0, __FUNCTION__, f, ##__VA_ARGS__)
#define ERROR_LOG(logger, f, ...) __LOG_IMPL(logger, error, "", 0, __FUNCTION__, f, ##__VA_ARGS__)
#define FATAL_LOG(logger, f, ...) __LOG_IMPL(logger, fatal, "", 0, __FUNCTION__, f, ##__VA_ARGS__)
#else /* 显示: 内容 */
#define TRACE_LOG(logger, f, ...) __LOG_IMPL(logger, trace, "", 0, "", f, ##__VA_ARGS__)
#define DEBUG_LOG(logger, f, ...) __LOG_IMPL(logger, debug, "", 0, "", f, ##__VA_ARGS__)
#define INFO_LOG(logger, f, ...) __LOG_IMPL(logger, info, "", 0, "", f, ##__VA_ARGS__)
#define WARN_LOG(logger, f, ...) __LOG_IMPL(logger, warn, "", 0, "", f, ##__VA_ARGS__)
#define ERROR_LOG(logger, f, ...) __LOG_IMPL(logger, error, "", 0, "", f, ##__VA_ARGS__)
#define FATAL_LOG(logger, f, ...) __LOG_IMPL(logger, fatal, "", 0, "", f, ##__VA_ARGS__)
#endif
/* 显示(纯净版): 内容 */
#define TRACE_LOG_PURE(logger, f, ...) __LOG_IMPL(logger, trace, "", 0, "", f, ##__VA_ARGS__)
#define DEBUG_LOG_PURE(logger, f, ...) __LOG_IMPL(logger, debug, "", 0, "", f, ##__VA_ARGS__)
#define INFO_LOG_PURE(logger, f, ...) __LOG_IMPL(logger, info, "", 0, "", f, ##__VA_ARGS__)
#define WARN_LOG_PURE(logger, f, ...) __LOG_IMPL(logger, warn, "", 0, "", f, ##__VA_ARGS__)
#define ERROR_LOG_PURE(logger, f, ...) __LOG_IMPL(logger, error, "", 0, "", f, ##__VA_ARGS__)
#define FATAL_LOG_PURE(logger, f, ...) __LOG_IMPL(logger, fatal, "", 0, "", f, ##__VA_ARGS__)

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
    static void start(const LogConfig& cfg, const std::string& defultTagName = std::string());

    /**
     * @brief 获取日志记录器
     * @param tagName 日志标签
     * @param level 日志等级, -1表示使用配置中的等级
     * @param loggerName 日志记录器名称
     * @return 日志记录器
     */
    static Logger getLogger(const std::string& tagName = std::string(), int level = -1, const std::string& loggerName = std::string());

private:
    /**
     * @brief 创建内部日志记录器
     * @param cfg 配置
     * @return 内部日志记录器
     */
    static InnerLoggerPtr createInnerLogger(const LogConfig& cfg);

private:
    static std::mutex m_mutex; /* 资源锁 */
    static LogConfig m_logCfg; /* 日志配置 */
    static std::unordered_map<std::string, InnerLoggerPtr> m_loggerMap; /* 内部日志记录器映射表 */
    static std::string m_defaultTagName; /* 默认日志标签名称 */
};
} // namespace logger
