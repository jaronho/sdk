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
     * @brief 计算目录MD5值, 算法: 遍历计算文件内容MD5并保存到列表 -> 对列表(MD5值)从小到大排序 -> 计算列表MD5
     * @param path 目录
     * @param progressCb 进度回调, 参数: name-文件路径, fileSize-文件大小(字节)
     * @param stopFunc 停止函数, 返回值: true-停止, false-继续
     * @param blockSize 内部文件MD5计算函数每次读文件的块大小(字节), 默认: 1Mb
     * @return 目录MD5值
     */
    static std::string md5Directory(const std::string& path,
                                    const std::function<void(const std::string& name, size_t fileSize)>& progressCb = nullptr,
                                    const std::function<bool()>& stopFunc = nullptr, size_t blockSize = 1024 * 1024);
};
} // namespace toolkit
