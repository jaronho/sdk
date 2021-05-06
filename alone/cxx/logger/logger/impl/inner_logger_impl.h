#pragma once
#include "../inner_logger.h"
#include "../logfile/daily_logfile.h"

#include <atomic>
#include <memory>
#include <string>

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
     * @param path 日志路径
     * @param name 记录器名称
     */
    InnerLoggerImpl(const std::string& path, const std::string& name);
    virtual ~InnerLoggerImpl() = default;
    InnerLoggerImpl& operator=(const InnerLoggerImpl& src) = delete;

    /**
     * @brief 获取等级
     * @return 等级
     */
    Level getLevel() const override;

    /**
     * @brief 设置等级
     * @param level 等级
     */
    void setLevel(const Level& level) override;

    /**
     * @brief 是否输出到控制台
     * @return true-是, false-否
     */
    bool isConsoleEnable() override;

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
    void print(const Level& level, const std::string& tag, const std::string& file, int line, const std::string& func, const std::string& msg) override;

private:
    std::atomic<Level> m_level = {Level::TRACE}; /* 日志等级 */
    std::shared_ptr<DailyLogfile> m_dailyLog; /* 每天日志文件 */
    std::atomic_bool m_consoleEnable = {true}; /* 是否输出到控制台 */
};

using InnerLoggerPtr = std::shared_ptr<InnerLogger>;
} // namespace logger
