#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include <string>

namespace logger
{
/**
 * @brief 日志文件
 */
class Logfile
{
public:
    enum class Result
    {
        OK, /* 成功 */
        INVALID, /* 日志文件无效 */
        DISABLED, /* 日志被禁止写入 */
        TOO_LARGE, /* 要写入的内容太长(超过文件容量) */
        WILL_FULL, /* 内容写入后文件将满 */
        NEWLINE_FAILED, /* 写入换行出错 */
        CONTENT_FAILED, /* 写入内容出错 */
        FLUSH_FAILED /* 刷新出错 */
    };

public:
    /**
     * @brief 创建路径
     * @param path 路径, 例如: "/home/workdpace/logs" 或 "/home/workdpace/logs/"
     * @return true-创建成功, false-创建失败
     */
    static bool createPath(const std::string& path);

    /**
     * @brief 遍历指定路径
     * @param path 路径, 例如: "/home/workdpace/logs" 或 "/home/workdpace/logs/"
     * @param folderCallback 目录回调
     * @param fileCallback 文件回调
     * @param recursive 是否递归遍历
     */
    static void traverse(std::string path, std::function<void(const std::string& name, long createTime, long writeTime, long accessTime)> folderCallback,
                         std::function<void(const std::string& name, long createTime, long writeTime, long accessTime, unsigned long size)> fileCallback,
                         bool recursive = true);

public:
    /**
     * @brief 构造函数
     * @param path 日志文件路径, 例如: "/home/workdpace/logs" 或 "/home/workdpace/logs/"
     * @param filename 日志文件名, 例如: "demo.log"
     * @param maxSize 文件最大容量值(字节), 例如: 4M = 4 * 1024 * 1024
     */
    Logfile(const std::string& path, const std::string& filename, size_t maxSize);

    ~Logfile();

    /**
     * @brief 文件是否已打开
     * @return true-有效, false-无效(打开失败)
     */
    bool isOpened();

    /**
     * @brief 打开
     * @return true-成功, false-失败
     */
    bool open();

    /**
     * @brief 关闭
     */
    void close();

    /**
     * @brief 获取日志文件路径
     * @return 日志文件路径
     */
    std::string getPath() const;

    /**
     * @brief 获取日志文件名称
     * @return 日志文件名
     */
    std::string getFilename() const;

    /**
     * @brief 获取日志文件全名(包含路径)
     * @return 日志文件全名
     */
    std::string getFullName() const;

    /**
     * @brief 获取日志文件最大容量
     * @return 文件最大容量(字节)
     */
    size_t getMaxSize() const;

    /**
     * @brief 是否启用记录功能
     * @return true-启用, false-禁用
     */
    bool isEnable() const;

    /**
     * @brief 设置是否启用记录功能
     * @param enable 启用标识
     */
    void setEnable(bool enable);

    /**
     * @brief 获取文件当前大小(字节)
     * @return 当前大小
     */
    size_t getSize();

    /**
     * @brief 清除日志文件内容
     */
    void clear();

    /**
     * @brief 记录日志内容
     * @param content 日志内容
     * @param newline 是否换行
     * @return 操作结果
     */
    Result record(const std::string& content, bool newline = true);

private:
    std::string m_path; /* 日志文件路径 */
    std::string m_filename; /* 日志文件名 */
    std::string m_fullName; /* 日志文件全名(包含路径) */
    size_t m_maxSize; /* 文件最大容量值 */
    std::recursive_mutex m_mutex; /* 互斥锁 */
    FILE* m_fp = nullptr; /* 文件指针 */
    std::atomic_size_t m_size = 0; /* 文件当前大小 */
    std::atomic_bool m_enable = true; /* 是否启用日志记录功能 */
};
} // namespace logger
