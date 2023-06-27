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
     * @brief 获取等级
     * @return 等级
     */
    int getLevel() const override;

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
     * @brief 是否输出到控制台
     * @return true-是, false-否
     */
    bool isConsoleEnable() const override;

    /**
     * @brief 设置是否输出到控制台
     * @param enable 开关标识
     */
    void setConsoleEnable(bool enable) override;

    /**
     * @brief 打印日志
     * @param level 日志等级
     * @param tag 日志标签
     * @param file 文件名
     * @param line 行号
     * @param func 函数名
     * @param msg 日志消息
     */
    void print(int level, const std::string& tag, const std::string& file, int line, const std::string& func,
               const std::string& msg) override;

private:
    /**
     * @brief 获取每天日志文件
     * @param level 日志等级
     * @return 每天日志文件
     */
    std::shared_ptr<DailyLogfile> getDailyLog(int level);

private:
    std::shared_ptr<DailyLogfile> m_dailyLog; /* 每天日志文件(通用) */
    std::shared_ptr<DailyLogfile> m_dailyLogTrace; /* 每天日志文件(跟踪) */
    std::shared_ptr<DailyLogfile> m_dailyLogDebug; /* 每天日志文件(调试) */
    std::shared_ptr<DailyLogfile> m_dailyLogInfo; /* 每天日志文件(信息) */
    std::shared_ptr<DailyLogfile> m_dailyLogWarn; /* 每天日志文件(警告) */
    std::shared_ptr<DailyLogfile> m_dailyLogError; /* 每天日志文件(错误) */
    std::shared_ptr<DailyLogfile> m_dailyLogFatal; /* 每天日志文件(致命) */
    std::atomic_int m_level = {LEVEL_TRACE}; /* 日志等级 */
    std::mutex m_mutexLevelFile;
    std::unordered_map<int, int> m_levelFile; /* 等级文件类型, key-日志等级, value-文件类型(同等级类型, 若不在范围内表示写入到通用文件) */
    std::atomic_bool m_consoleEnable = {false}; /* 是否输出到控制台(默认不输出) */
};

using InnerLoggerPtr = std::shared_ptr<InnerLogger>;
} // namespace logger
