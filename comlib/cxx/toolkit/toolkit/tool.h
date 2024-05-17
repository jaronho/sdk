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
     * @brief 计算目录MD5值, 算法: 遍历收集目录/文件信息 -> 按顺序(路径从小到大)计算目录/文件名/文件内容的MD5值
     * @param path 目录
     * @param type 算法类型: 0-文件内容, 1-目录/文件名和文件内容, 2-(ASCII且同时符合所有平台命名规则)目录/文件名和文件内容
     * @param beginCb 开始回调, 参数: folderCount-目录总数, folderCalcCount-纳入计算的目录总数, fileCount-文件总数, totalSize-文件总大小(字节)
     * @param progressCb 进度回调, 参数: name-文件路径, relName-相对路径, isDir-是否目录, fileSize-文件大小(字节, 目录恒为4个字节)
     * @param stopFunc 停止函数, 返回值: true-停止, false-继续
     * @param blockSize 内部文件MD5计算函数每次读文件的块大小(字节)
     * @return 目录MD5值
     */
    static std::string md5Directory(
        const std::string& path, int type = 0,
        const std::function<void(size_t folderCount, size_t folderCalcCount, size_t fileCount, size_t totalSize)>& beginCb = nullptr,
        const std::function<void(const std::string& name, const std::string& relName, bool isDir, size_t fileSize)>& progressCb = nullptr,
        const std::function<bool()>& stopFunc = nullptr, size_t blockSize = 1024 * 1024);
};
} // namespace toolkit
