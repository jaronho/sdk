#pragma once
#include <memory>
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
 * @brief 内部日志记录器
 */
class InnerLogger
{
public:
    /**
     * @brief 构造函数
     * @param path 日志路径
     * @param name 记录器名称
     */
    InnerLogger(const std::string& path, const std::string& name);
    virtual ~InnerLogger() = default;
    InnerLogger& operator=(const InnerLogger& src) = delete;

    /**
     * @brief 获取路径
     * @return 路径
     */
    std::string getPath() const;

    /**
     * @brief 获取名称
     * @return 名称
     */
    std::string getName() const;

    /**
     * @brief 获取等级
     * @return 等级
     */
    virtual Level getLevel() const = 0;

    /**
     * @brief 设置等级
     * @param level 等级
     */
    virtual void setLevel(const Level& level) = 0;

    /**
     * @brief 是否输出到控制台
     * @return true-是, false-否
     */
    virtual bool isConsoleEnable() = 0;

    /**
     * @brief 设置是否输出到控制台
     * @param enable 开关标识
     */
    virtual void setConsoleEnable(bool enable) = 0;

    /**
     * @brief 打印日志
     * @param level 日志等级
     * @param tag 日志标签
     * @param file 文件名
     * @param line 行号
     * @param func 函数名
     * @param msg 日志消息
     */
    virtual void print(const Level& level, const std::string& tag, const std::string& file, int line, const std::string& func,
                       const std::string& msg) = 0;

private:
    const std::string m_path; /* 日志路径 */
    const std::string m_name; /* 日志记录器名称 */
};

using InnerLoggerPtr = std::shared_ptr<InnerLogger>;
} // namespace logger
