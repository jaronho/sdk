#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

#include "logfile.h"

namespace logger
{
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
     * @param extName 日志文件后缀名, 例如: "log" 或 ".log"
     * @param maxSize 文件最大容量值(字节), 例如: 4M = 4 * 1024 * 1024
     * @param maxFiles 最多文件个数, 为0时表示个数不受限制
     */
    RotatingLogfile(const std::string& path, const std::string& baseName, const std::string& extName, size_t maxSize, size_t maxFiles = 0);

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
     * @brief 获取当前日志文件索引值
     * @return 索引值
     */
    size_t getFileIndex() const;

    /**
     * @brief 记录日志内容
     * @param content 日志内容
     * @param newline 是否换行
     * @return 操作结果
     */
    Logfile::Result record(const std::string& content, bool newline = true);

private:
    /**
     * @brief 遍历指定路径文件
     * @param path 路径, 例如: "/home/workdpace/logs" 或 "/home/workdpace/logs/"
     * @param callback 回调
     * @param recursive 是否递归遍历
     */
    void traverseFile(std::string path, std::function<void(const std::string& fullName)> callback, bool recursive = true);

    /**
     * @brief 查找指定路径下匹配的文件名索引列表
     * @param path 路径
     * @param pattern 匹配模式(正则表达式)
     * @param indexList 匹配到的索引列表(按降序排序)
     * @return true-有找到, false-未找到
     */
    bool findIndexList(const std::string& path, const std::regex& pattern, std::vector<int>& indexList);

    /**
     * @brief 查找指定路径下的最后的索引值
     * @param path 路径
     * @param indexList 匹配到的索引列表(按降序排序)
     * @return 索引值
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
    size_t m_maxFiles; /* 最多文件个数 */
    std::atomic_size_t m_index; /* 当前日志文件索引值 */
    std::mutex m_mutex; /* 互斥锁 */
    std::shared_ptr<Logfile> m_logfile; /* 基础日志文件 */
};
} // namespace logger
