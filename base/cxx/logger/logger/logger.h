#pragma once
#include <memory>
#include <string>

#include "inner_logger.h"

namespace logger
{
/**
 * @brief 日志记录器
 */
class Logger
{
public:
    /**
     * @brief 构造函数
     * @param tag 标签
     * @param inner 内部日志记录器
     */
    Logger(const std::string& tag, const std::shared_ptr<InnerLogger>& inner);
    Logger() = default;
    virtual ~Logger() = default;

    /**
     * @brief 是否有效
     * @return true-有效, false-无效
     */
    bool isValid() const;

    /**
     * @brief 获取标签
     * @return 标签
     */
    std::string getTag() const;

    /**
     * @brief 获取名称
     * @return 名称
     */
    std::string getName() const;

    /**
     * @brief 获取等级
     * @return 等级
     */
    int getLevel() const;

    /**
     * @brief 设置等级
     * @param level 等级
     */
    void setLevel(int level);

    /**
     * @brief 获取等级文件类型
     * @param level 等级
     * @return 文件类型
     */
    int getLevelFile(int level) const;

    /**
     * @brief 设置等级文件类型
     * @param level 等级
     * @param fileType 日志等级要写入的文件类型(类型值同level), 不在类型值范围内表示记录到通用文件
     */
    void setLevelFile(int level, int fileType = -1);

    /**
     * @brief 是否输出到控制台
     * @return true-是, false-否
     */
    bool isConsoleEnable() const;

    /**
     * @brief 设置是否输出到控制台
     * @param enable 开关标识
     */
    void setConsoleEnable(bool enable);

    void trace(const std::string& file, int line, const std::string& fun, const std::string& msg) const;
    void debug(const std::string& file, int line, const std::string& fun, const std::string& msg) const;
    void info(const std::string& file, int line, const std::string& fun, const std::string& msg) const;
    void warn(const std::string& file, int line, const std::string& fun, const std::string& msg) const;
    void error(const std::string& file, int line, const std::string& fun, const std::string& msg) const;
    void fatal(const std::string& file, int line, const std::string& fun, const std::string& msg) const;

private:
    std::string m_tag; /* 标签 */
    std::shared_ptr<InnerLogger> m_inner = nullptr; /* 内部日志记录器 */
};
} // namespace logger
