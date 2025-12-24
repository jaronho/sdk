#pragma once
#include <string>
#include <unordered_map>

namespace logger
{
/**
 * @brief 日志等级
 */
const int LEVEL_TRACE = 0; /* 跟踪, 指明程序运行轨迹 */
const int LEVEL_DEBUG = 1; /* 调试, 指明细致的事件信息 */
const int LEVEL_INFO = 2; /* 信息, 指明描述信息 */
const int LEVEL_WARN = 3; /* 警告, 指明可能潜在的危险状况 */
const int LEVEL_ERROR = 4; /* 错误, 指明错误事件, 但应用可能还能继续运行 */
const int LEVEL_FATAL = 5; /* 致命, 指明非常严重的可能会导致应用终止执行错误事件 */

/**
 * @brief 日志配置
 */
struct LogConfig
{
    bool operator==(const LogConfig& other) const
    {
        if (other.path != path || other.name != name || other.fileExtName != fileExtName || other.level != level
            || other.fileMaxSize != fileMaxSize || other.fileMaxCount != fileMaxCount || other.fileIndexFixed != fileIndexFixed
            || other.newFolderDaily != newFolderDaily || other.consoleMode != consoleMode)
        {
            return false;
        }
        for (const auto& kv : levelFile)
        {
            auto iter = other.levelFile.find(kv.first);
            if (other.levelFile.end() == iter || kv.second != iter->second)
            {
                return false;
            }
        }
        for (const auto& kv : other.levelFile)
        {
            auto iter = levelFile.find(kv.first);
            if (levelFile.end() == iter || kv.second != iter->second)
            {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const LogConfig& other) const
    {
        return !(operator==(other));
    }

    std::string path; /* (选填)日志文件路径, 默认: 程序文件路径 */
    std::string name; /* (选填)记录器名称, 默认: 程序文件名 */
    std::string fileExtName = ".log"; /* (选填)日志文件扩展名, 默认: ".log" */
    int level = LEVEL_TRACE; /* (选填)日志等级, 默认: LEVEL_TRACE */
    std::unordered_map<int, int> levelFile; /* 等级文件类型, key-日志等级, value-文件类型(同等级类型, 若不在范围内表示写入到通用文件) */
    int flushLevel = LEVEL_TRACE; /* (选填)刷新等级(当日志等级大等于刷新等级时, 日志写入后立即刷新), 默认: LEVEL_TRACE */
    size_t fileMaxSize = (20 * 1024 * 1024); /* (选填)每个日志文件最大长度(字节), 默认: 20M = 20 * 1024 * 1024 */
    size_t fileMaxCount = 0; /* (选填)每天允许最多的日志文件数, 默认: 0-表示不限制 */
    bool fileIndexFixed = false; /* 文件数最大时, 索引值固定还是递增, 默认: false-递增 */
    bool newFolderDaily = true; /* (选填)是否每天使用新文件夹, 默认: true-表示每天都创建新文件夹 */
    int consoleMode = 0; /* (选填)控制台日志输出模式: 0-不输出, 1-普通输出, 2-带样式输出 */
};
} // namespace logger
