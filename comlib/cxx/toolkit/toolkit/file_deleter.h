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
     * @brief 配置
     */
    struct Config
    {
        Config() = default;
        Config(const std::string& folder, size_t expireTime, size_t occupySize = 0, float occupyLine = 0.0f,
               const std::vector<std::string>& ignoreFileList = {})
            : folder(folder)
            , expireTime(expireTime)
            , occupySize(occupySize)
            , occupyLine((occupyLine < 0.0f) ? 0.0f : ((occupyLine > 1.0f) ? 1.0f : occupyLine))
            , ignoreFileList(ignoreFileList)
        {
        }

        std::string folder; /* 目录 */
        size_t expireTime = (3600 * 24 * 30); /* 过期时间(秒), 文件最后修改时间过期则删除, 默认30天 */
        size_t occupySize = 0; /* 最大分配空间(字节) */
        float occupyLine = 0.0f; /* 分配空间删除线(百分比), 值: [0.0, 1.0] */
        std::vector<std::string> ignoreFileList; /* 忽略文件列表(此列表中的文件不删除) */
    };

    /**
     * @brief 删除目录/文件前检测函数
     * @param name 目录名/文件名(全路径)
     * @param attr 属性
     * @param depth 深度
     * @return true-允许删除, false-不允许删除
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
     * @brief 尝试执行一次
     * @param cfgList 配置列表
     * @param folderCheckFunc 目录检测函数
     * @param folderDeletedCb 目录被删除回调
     * @param fileCheckFunc 文件检测函数
     * @param fileDeletedCb 文件被删除回调
     */
    static void tryOnce(const std::vector<Config>& cfgList, const DeleteCheckFunc& folderCheckFunc, const DeletedCallback& folderDeletedCb,
                        const DeleteCheckFunc& fileCheckFunc, const DeletedCallback& fileDeletedCb);
};
} // namespace toolkit
