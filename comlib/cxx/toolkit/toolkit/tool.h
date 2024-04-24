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
     * @param progressCb 进度回调, 参数: nowFolderCount-已计算的文件夹数, nowFileCount-已计算的文件数, nowFileSize-已计算的文件总大小(字节)
     * @return 目录MD5值
     */
    static std::string
    md5Directory(const std::string& path,
                 const std::function<void(size_t nowFolderCount, size_t nowFileCount, size_t nowFileSize)>& progressCb = nullptr);
};
} // namespace toolkit
