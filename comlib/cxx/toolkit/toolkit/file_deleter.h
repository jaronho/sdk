#pragma once
#include <functional>
#include <vector>

#include "utility/filesystem/fs_define.h"

namespace toolkit
{
/**
 * @brief 文件删除器
 */
class FileDeleter final
{
public:
    /**
     * @brief 占满配置
     */
    struct OccupyConfig
    {
        OccupyConfig() = default;
        OccupyConfig(const std::string& folder, size_t clearSize, const std::vector<std::string>& ignoreFileList = {})
            : folder(folder), clearSize(clearSize), ignoreFileList(ignoreFileList)
        {
        }

        std::string folder; /* 目录 */
        size_t clearSize = 0; /* 要删除的文件总大小(字节) */
        std::vector<std::string> ignoreFileList; /* 忽略文件列表(此列表中的文件不删除) */
    };

    /**
     * @brief 过期配置
     */
    struct ExpireConfig
    {
        ExpireConfig() = default;
        ExpireConfig(const std::string& folder, size_t expireTime, const std::vector<std::string>& ignoreFileList = {})
            : folder(folder), expireTime(expireTime), ignoreFileList(ignoreFileList)
        {
        }

        std::string folder; /* 目录 */
        size_t expireTime = (3600 * 24 * 30); /* 过期时间(秒), 文件最后修改时间过期则删除, 默认30天 */
        std::vector<std::string> ignoreFileList; /* 忽略文件列表(此列表中的文件不删除) */
    };

    /**
     * @brief 删除目录/文件前检测函数
     * @param name 目录名/文件名(全路径)
     * @param attr 属性
     * @param depth 深度
     * @return true-删除, false-不删除
     */
    using DeleteCheckFunc = std::function<bool(const std::string& name, const utility::FileAttribute& attr, int depth)>;

    /**
     * @brief 目录/文件被删除回调
     * @param name 目录名/文件名(全路径)
     * @param attr 属性
     * @param depth 深度
     * @param ok 是否删除成功
     */
    using DeletedCallback = std::function<void(const std::string& name, const utility::FileAttribute& attr, int depth, bool ok)>;

public:
    /**
     * @brief 删除占满的文件
     * @param cfg 配置
     * @param folderCheckFunc 目录检测函数
     * @param folderDeletedCb 目录被删除回调
     * @param fileCheckFunc 文件检测函数
     * @param fileDeletedCb 文件被删除回调
     */
    static void deleteOccupy(const OccupyConfig& cfg, const DeleteCheckFunc& folderCheckFunc, const DeletedCallback& folderDeletedCb,
                             const DeleteCheckFunc& fileCheckFunc, const DeletedCallback& fileDeletedCb);

    /**
     * @brief 删除过期的文件
     * @param cfgList 配置列表
     * @param folderCheckFunc 目录检测函数
     * @param folderDeletedCb 目录被删除回调
     * @param fileCheckFunc 文件检测函数
     * @param fileDeletedCb 文件被删除回调
     */
    static void deleteExpired(const std::vector<ExpireConfig>& cfgList, const DeleteCheckFunc& folderCheckFunc,
                              const DeletedCallback& folderDeletedCb, const DeleteCheckFunc& fileCheckFunc,
                              const DeletedCallback& fileDeletedCb);
};
} // namespace toolkit
