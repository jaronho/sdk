#pragma once
#include <string>

namespace logger
{
/**
 * @brief 日志等级
 */
enum class Level
{
    TRACE = 0, /* 跟踪, 指明程序运行轨迹 */
    DEBUG = 1, /* 调试，指明细致的事件信息 */
    INFO = 2, /* 信息，指明描述信息 */
    WARN = 3, /* 警告，指明可能潜在的危险状况 */
    ERROR = 4, /* 错误，指明错误事件, 但应用可能还能继续运行 */
    FATAL = 5 /* 致命，指明非常严重的可能会导致应用终止执行错误事件 */
};

/**
 * @brief 日志配置
 */
struct LogConfig
{
    LogConfig() : name("app"), fileExtName(".log"), fileMaxSize(20 * 1024 * 1024), fileMaxCount(0U), newFolderDaily(true) {}

    std::string path; /* (必填)日志文件路径 */
    std::string name; /* (选填)记录器名称, 默认: "app" */
    std::string fileExtName; /* (选填)日志文件扩展名, 默认: ".log" */
    size_t fileMaxSize; /* (选填)每个日志文件最大长度(字节), 默认: 20M = 20 * 1024 * 1024 */
    size_t fileMaxCount; /* (选填)每天允许最多的日志文件数, 默认: 0-表示不限制 */
    bool newFolderDaily; /* (选填)是否每天使用新文件夹, 默认: true-表示每天都创建新文件夹 */
};
} // namespace logger
