#pragma once
#include <functional>
#include <string>

namespace toolkit
{
/**
 * @brief 工具类
 */
class Tool final
{
public:
    /**
     * @brief 计算文件MD5值
     * @param fullName 文件全路径
     * @param stopFunc 停止函数, 返回值: true-停止, false-继续
     * @param blockSize 内部文件MD5计算函数每次读文件的块大小(字节), 默认: 1Mb
     * @param enableMMFile 是否启用内存映射文件, true-启用, false-不启用
     * @return 返回文件MD5值
     */
    static std::string md5File(const std::string& fullName, const std::function<bool()>& stopFunc = nullptr, size_t blockSize = 1024 * 1024,
                               bool enableMMFile = false);

    /**
     * @brief 计算目录MD5值, 算法: 遍历计算文件内容MD5并保存到列表 -> 对列表(MD5值)从小到大排序 -> 计算列表MD5
     * @param path 目录
     * @param beginCb 开始回调, 参数: totalCount-总文件数量, totalSize-总文件大小(字节)
     * @param func 自定义执行函数, 参数: name-文件路径, fileSize-文件大小(字节), calcFunc-计算函数(必须调用, 允许异步调用)
     * @param stopFunc 停止函数, 返回值: true-停止, false-继续
     * @param blockSize 内部文件MD5计算函数每次读文件的块大小(字节), 默认: 1Mb
     * @param enableMMFile 是否启用内存映射文件, true-启用, false-不启用
     * @return 返回目录MD5值
     */
    static std::string md5Directory(
        const std::string& path, const std::function<void(size_t totalCount, size_t totalSize)>& beginCb = nullptr,
        const std::function<void(const std::string& name, size_t fileSize, const std::function<std::string()>& calcFunc)>& func = nullptr,
        const std::function<bool()>& stopFunc = nullptr, size_t blockSize = 1024 * 1024, bool enableMMFile = false);

    /**
     * @brief 计算文件xxhash值
     * @param fullName 文件全路径
     * @param stopFunc 停止函数, 返回值: true-停止, false-继续
     * @param blockSize 内部文件MD5计算函数每次读文件的块大小(字节), 默认: 1Mb
     * @param enableMMFile 是否启用内存映射文件, true-启用, false-不启用
     * @return 返回文件xxhash值
     */
    static uint64_t xxhashFile(const std::string& fullName, const std::function<bool()>& stopFunc = nullptr, size_t blockSize = 1024 * 1024,
                               bool enableMMFile = false);

    /**
     * @brief 计算目录xxhash值, 算法: 遍历计算文件内容xxhash并保存到列表 -> 对列表(xxhash值)从小到大排序 -> 计算列表xxhash
     * @param path 目录
     * @param beginCb 开始回调, 参数: totalCount-总文件数量, totalSize-总文件大小(字节)
     * @param func 自定义执行函数, 参数: name-文件路径, fileSize-文件大小(字节), calcFunc-计算函数(必须调用, 允许异步调用)
     * @param stopFunc 停止函数, 返回值: true-停止, false-继续
     * @param blockSize 内部文件xxhash计算函数每次读文件的块大小(字节), 默认: 1Mb
     * @param enableMMFile 是否启用内存映射文件, true-启用, false-不启用
     * @return 返回目录xxhash值
     */
    static uint64_t xxhashDirectory(
        const std::string& path, const std::function<void(size_t totalCount, size_t totalSize)>& beginCb = nullptr,
        const std::function<void(const std::string& name, size_t fileSize, const std::function<uint64_t()>& calcFunc)>& func = nullptr,
        const std::function<bool()>& stopFunc = nullptr, size_t blockSize = 1024 * 1024, bool enableMMFile = false);
};
} // namespace toolkit
