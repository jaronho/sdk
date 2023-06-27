#pragma once
#include <memory>
#include <string>

#include "logger_define.h"

namespace logger
{
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
    virtual int getLevel() const = 0;

    /**
     * @brief 设置等级
     * @param level 等级
     */
    virtual void setLevel(int level) = 0;

    /**
     * @brief 获取等级文件类型
     * @param level 等级
     * @return 文件类型
     */
    virtual int getLevelFile(int level) = 0;

    /**
     * @brief 设置等级文件类型
     * @param level 等级
     * @param fileType 日志等级要写入的文件类型(类型值同level), 不在类型值范围内表示记录到通用文件
     */
    virtual void setLevelFile(int level, int fileType = -1) = 0;

    /**
     * @brief 是否输出到控制台
     * @return true-是, false-否
     */
    virtual bool isConsoleEnable() const = 0;

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
    virtual void print(int level, const std::string& tag, const std::string& file, int line, const std::string& func,
                       const std::string& msg) = 0;

private:
    const std::string m_path; /* 日志路径 */
    const std::string m_name; /* 日志记录器名称 */
};

using InnerLoggerPtr = std::shared_ptr<InnerLogger>;
} // namespace logger
