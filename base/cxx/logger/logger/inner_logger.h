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
     * @brief 获取日志文件最大容量
     * @return 文件最大容量(字节)
     */
    virtual size_t getMaxSize() = 0;

    /**
     * @brief 设置日志文件最大容量
     * @param maxSize 文件最大容量(字节)
     */
    virtual void setMaxSize(size_t maxSize) = 0;

    /**
     * @brief 获取最多文件个数
     * @return 最多文件个数
     */
    virtual size_t getMaxFiles() = 0;

    /**
     * @brief 设置最多文件个数
     * @param maxFiles 最多文件个数
     */
    virtual void setMaxFiles(size_t maxFiles) = 0;

    /**
     * @brief 获取等级
     * @return 等级
     */
    virtual int getLevel() = 0;

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
     * @brief 获取刷新等级
     * @return 取刷新等级
     */
    virtual int getFlushLevel() = 0;

    /**
     * @brief 设置取刷新等级
     * @param level 取刷新等级
     */
    virtual void setFlushLevel(int level) = 0;

    /**
     * @brief 获取控制台日志输出模式
     * @return 0-不输出, 1-普通输出, 2-带样式输出
     */
    virtual int getConsoleMode() = 0;

    /**
     * @brief 设置控制台日志输出模式
     * @param mode 模式: 0-不输出, 1-普通输出, 2-带样式输出
     */
    virtual void setConsoleMode(int mode) = 0;

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

    /**
     * @brief 强制刷新日志内容
     */
    virtual void forceFlush() = 0;

private:
    const std::string m_path; /* 日志路径 */
    const std::string m_name; /* 日志记录器名称 */
};

using InnerLoggerPtr = std::shared_ptr<InnerLogger>;
} // namespace logger
