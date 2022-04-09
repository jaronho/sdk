#pragma once
#include <atomic>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

#include "rotating_logfile.h"

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
     * @param extName 日志文件后缀名(允许为空), 例如: "log" 或 ".log"
     * @param maxSize 文件最大容量值(字节, 选填), 为0时表示不限制文件大小, 例如: 4M = 4 * 1024 * 1024
     * @param maxFiles 最多文件个数(选填), 为0时表示个数不受限制
     * @param indexFixed 文件数最大时(选填), true-索引值固定, false-递增
     * @param createDailyFolder 是否创建每日文件夹(选填)
     */
    explicit DailyLogfile(const std::string& path, const std::string& prefixName, const std::string& extName, size_t maxSize = 0,
                          size_t maxFiles = 0, bool indexFixed = false, bool createDailyFolder = true);

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
    size_t m_maxSize = 0; /* 文件最大容量值 */
    size_t m_maxFiles = 0; /* 最多文件个数 */
    bool m_indexFixed = false; /* 文件数最大时, 索引值固定还是递增 */
    bool m_createDailyFolder = true; /* 是否创建每日文件夹 */
    std::mutex m_mutex; /* 互斥锁 */
    std::shared_ptr<RotatingLogfile> m_rotatingLogfile = nullptr; /* 滚动日志文件 */
};
