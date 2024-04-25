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
     * @brief 计算目录MD5值
     * @param path 目录
     * @param progressCb 进度回调, 参数: name-文件路径, isDir-是否目录, fileSize-文件大小(字节, 目录恒为4个字节)
     * @return 目录MD5值
     */
    static std::string md5Directory(const std::string& path,
                                    const std::function<void(const std::string& name, bool isDir, size_t fileSize)>& progressCb = nullptr);
};
} // namespace toolkit
