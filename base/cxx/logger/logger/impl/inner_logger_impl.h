#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "../inner_logger.h"
#include "../logfile/daily_logfile.h"

namespace logger
{
/**
 * @brief 内部日志记录器
 */
class InnerLoggerImpl final : public InnerLogger
{
public:
    /**
     * @brief 构造函数
     * @param cfg 日志配置
     */
    InnerLoggerImpl(const LogConfig& cfg);
    virtual ~InnerLoggerImpl() = default;
    InnerLoggerImpl& operator=(const InnerLoggerImpl& src) = delete;

    /**
     * @brief 获取日志文件最大容量
     * @return 文件最大容量(字节)
     */
    size_t getMaxSize() override;

    /**
     * @brief 设置日志文件最大容量
     * @param maxSize 文件最大容量(字节)
     */
    void setMaxSize(size_t maxSize) override;

    /**
     * @brief 获取最多文件个数
     * @return 最多文件个数
     */
    size_t getMaxFiles() override;

    /**
     * @brief 设置最多文件个数
     * @param maxFiles 最多文件个数
     */
    void setMaxFiles(size_t maxFiles) override;

    /**
     * @brief 获取等级
     * @return 等级
     */
    int getLevel() override;

    /**
     * @brief 设置等级
     * @param level 等级
     */
    void setLevel(int level) override;

    /**
     * @brief 获取等级文件类型
     * @param level 等级
     * @return 文件类型
     */
    int getLevelFile(int level) override;

    /**
     * @brief 设置等级文件
     * @param level 等级
     * @param fileType 日志等级要写入的文件类型(类型值同level), 不在类型值范围内表示记录到通用文件
     */
    void setLevelFile(int level, int fileType = -1) override;

    /**
     * @brief 获取刷新等级
     * @return 取刷新等级
     */
    int getFlushLevel() override;

    /**
     * @brief 设置取刷新等级
     * @param level 取刷新等级
     */
    void setFlushLevel(int level) override;

    /**
     * @brief 获取控制台日志输出模式
     * @return 0-不输出, 1-普通输出, 2-带样式输出
     */
    int getConsoleMode() override;

    /**
     * @brief 设置控制台日志输出模式
     * @param mode 模式: 0-不输出, 1-普通输出, 2-带样式输出
     */
    void setConsoleMode(int mode) override;

    /**
     * @brief 打印日志
     * @param level 日志等级
     * @param tag 日志标签
     * @param file 文件名
     * @param line 行号
     * @param func 函数名
     * @param msg 日志消息
     * @param msgLen 日志消息长度
     */
    void print(int level, const std::string& tag, const char* file, int line, const char* func, const char* msg, size_t msgLen) override;

    /**
     * @brief 打印日志
     * @param level 日志等级
     * @param tag 日志标签
     * @param file 文件名
     * @param line 行号
     * @param func 函数名
     * @param msg 日志消息
     */
    void print(int level, const std::string& tag, const char* file, int line, const char* func, const std::string& msg) override;

    /**
     * @brief 强制刷新日志内容
     */
    void forceFlush() override;

private:
    /**
     * @brief 获取每天日志文件
     * @param level 日志等级
     * @return 每天日志文件
     */
    std::shared_ptr<DailyLogfile> getDailyLog(int level);

private:
    std::shared_ptr<DailyLogfile> m_dailyLog = nullptr; /* 每天日志文件(通用) */
    std::shared_ptr<DailyLogfile> m_dailyLogTrace = nullptr; /* 每天日志文件(跟踪) */
    std::shared_ptr<DailyLogfile> m_dailyLogDebug = nullptr; /* 每天日志文件(调试) */
    std::shared_ptr<DailyLogfile> m_dailyLogInfo = nullptr; /* 每天日志文件(信息) */
    std::shared_ptr<DailyLogfile> m_dailyLogWarn = nullptr; /* 每天日志文件(警告) */
    std::shared_ptr<DailyLogfile> m_dailyLogError = nullptr; /* 每天日志文件(错误) */
    std::shared_ptr<DailyLogfile> m_dailyLogFatal = nullptr; /* 每天日志文件(致命) */
    std::atomic<size_t> m_fileMaxSize = {0}; /* 日志最大文件大小 */
    std::atomic<size_t> m_fileMaxCount = {0}; /* 日志最大文件数量 */
    std::atomic_int m_level = {LEVEL_TRACE}; /* 日志等级 */
    std::mutex m_mutexLevelFile;
    std::unordered_map<int, int> m_levelFile; /* 等级文件类型, key-日志等级, value-文件类型(同等级类型, 若不在范围内表示写入到通用文件) */
    std::atomic_int m_flushLevel = {LEVEL_TRACE}; /* 刷新等级(当日志等级大等于刷新等级时, 日志写入后立即刷新) */
    std::atomic_int m_consoleMode = {0}; /* 控制台日志输出模式: 0-不输出, 1-普通输出, 2-带样式输出 */
};

using InnerLoggerPtr = std::shared_ptr<InnerLogger>;
} // namespace logger
