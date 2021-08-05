#pragma once
#include <atomic>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

#include "rotating_logfile.h"

namespace logger
{
/**
 * @brief 每天日志文件
 */
class DailyLogfile final
{
public:
    /**
     * @brief 构造函数
     * @param path 日志文件路径, 例如: "/home/workspace/logs" 或 "/home/workspace/logs/"
     * @param prefixName 日志文件前缀名(允许为空), 例如: "demo"
     * @param extName 日志文件后缀名, 例如: "log" 或 ".log"
     * @param maxSize 文件最大容量值(字节), 例如: 4M = 4 * 1024 * 1024
     * @param maxFiles 最多文件个数, 为0时表示个数不受限制
     * @param syncFreq 主动同步到磁盘的频率(单位:条数), 每记录几条就同步, 例如: 50条
     * @param createDailyFolder 是否创建每日文件夹
     */
    DailyLogfile(const std::string& path, const std::string& prefixName, const std::string& extName, size_t maxSize, size_t maxFiles = 0,
                 size_t syncFreq = 50, bool createDailyFolder = true);

    virtual ~DailyLogfile() = default;

    /**
     * @brief 记录日志内容
     * @param content 日志内容
     * @param newline 是否换行
     * @return 操作结果
     */
    Logfile::Result record(const std::string& content, bool newline = true);

private:
    std::string m_path; /* 日志文件路径 */
    std::string m_prefixName; /* 日志文件前缀名 */
    std::string m_baseName; /* 日志文件名 */
    std::string m_extName; /* 日志文件后缀名 */
    size_t m_maxSize; /* 文件最大容量值 */
    size_t m_maxFiles; /* 最多文件个数 */
    size_t m_syncFreq; /* 同步到磁盘的频率 */
    bool m_createDailyFolder; /* 是否创建每日文件夹 */
    std::recursive_mutex m_mutex; /* 互斥锁 */
    std::shared_ptr<RotatingLogfile> m_rotatingLogfile; /* 滚动日志文件 */
};
} // namespace logger
