#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "logfile.h"

/**
 * @brief 滚动日志文件
 */
class RotatingLogfile final
{
public:
    /**
     * @brief 构造函数
     * @param path 日志文件路径, 例如: "/home/workspace/logs" 或 "/home/workspace/logs/"
     * @param baseName 日志文件名, 例如: "demo"
     * @param extName 日志文件后缀名(允许为空), 例如: "log" 或 ".log"
     * @param maxSize 文件最大容量值(字节, 选填), 为0时表示不限制文件大小, 例如: 4M = 4 * 1024 * 1024
     * @param maxFiles 最多文件个数(选填), 为0时表示个数不受限制
     * @param indexFixed 文件数最大时(选填), true-索引值固定, false-递增
     */
    explicit RotatingLogfile(const std::string& path, const std::string& baseName, const std::string& extName, size_t maxSize = 0,
                             size_t maxFiles = 0, bool indexFixed = false);

    virtual ~RotatingLogfile() = default;

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
    std::string getPath();

    /**
     * @brief 获取日志文件名称
     * @return 日志文件名
     */
    std::string getFilename();

    /**
     * @brief 获取日志文件全名(包含路径)
     * @return 日志文件全名
     */
    std::string getFullName();

    /**
     * @brief 获取日志文件最大容量
     * @return 文件最大容量(字节)
     */
    size_t getMaxSize();

    /**
     * @brief 设置日志文件最大容量
     * @param maxSize 文件最大容量(字节)
     */
    void setMaxSize(size_t maxSize);

    /**
     * @brief 是否启用记录功能
     * @return true-启用, false-禁用
     */
    bool isEnable();

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
     * @brief 获取最多文件个数
     * @return 最多文件个数
     */
    size_t getMaxFiles() const;

    /**
     * @brief 设置最多文件个数
     * @param maxFiles 最多文件个数
     */
    void setMaxFiles(size_t maxFiles);

    /**
     * @brief 获取当前日志文件索引值
     * @return 索引值
     */
    int getFileIndex() const;

    /**
     * @brief 记录日志内容
     * @param content 日志内容
     * @param newline 是否换行
     * @param immediateFlush 是否立即刷新
     * @return 操作结果
     */
    Logfile::Result record(const std::string& content, bool newline = true, bool immediateFlush = true);

    /**
     * @brief 强制刷新日志内容
     * @return true-成功, false-失败
     */
    bool forceFlush();

private:
    /**
     * @brief 遍历指定路径文件
     * @param path 路径, 例如: "/home/workdpace/logs" 或 "/home/workdpace/logs/"
     * @param callback 回调
     */
    void traverseFile(std::string path, std::function<void(const std::string& fullName)> callback);

    /**
     * @brief 查找指定路径下匹配的文件名索引列表
     * @param path 路径
     * @param pattern 匹配模式(正则表达式)
     * @return 匹配到的索引列表(按降序排序)
     */
    std::vector<int> findIndexList(const std::string& path, const std::string& pattern);

    /**
     * @brief 查找指定路径下的最后的索引值
     * @param path 路径
     * @param indexList [输出]匹配到的索引列表(按降序排序)
     * @return 索引值, -1.未找到
     */
    int findLastIndex(const std::string& path, std::vector<int>& indexList);

    /**
     * @brief 根据索引值计算文件名
     * @param index 索引值
     * @return 文件名
     */
    std::string calcFilenameByIndex(int index);

    /**
     * @brief 滚动文件
     * @return true-成功, false-失败
     */
    bool rotateFileList();

private:
    std::string m_baseName; /* 日志文件名 */
    std::string m_extName; /* 日志文件后缀名 */
    std::atomic<size_t> m_maxFiles = {0}; /* 最多文件个数 */
    std::atomic_int m_index = {0}; /* 当前日志文件索引值 */
    bool m_indexFixed = false; /* 文件数最大时, 索引值固定还是递增 */
    std::mutex m_mutex; /* 互斥锁 */
    std::shared_ptr<Logfile> m_logfile = nullptr; /* 基础日志文件 */
};
