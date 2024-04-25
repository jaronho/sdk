#include "file_deleter.h"

#include <algorithm>

#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

namespace toolkit
{
struct InfoInner
{
    InfoInner(const std::string& name, const utility::FileAttribute& attr, int depth) : name(name), attr(attr), depth(depth) {}
    std::string name;
    utility::FileAttribute attr;
    int depth;
};

void FileDeleter::deleteOccupy(const OccupyConfig& cfg, const DeleteCheckFunc& folderCheckFunc, const DeletedCallback& folderDeletedCb,
                               const DeleteCheckFunc& fileCheckFunc, const DeletedCallback& fileDeletedCb)
{
    if (cfg.clearSize <= 0) /* 不需要清除 */
    {
        return;
    }
    utility::PathInfo pi(cfg.folder);
    if (!pi.exist()) /* 目录不存在 */
    {
        return;
    }
    std::vector<InfoInner> folderList;
    std::vector<InfoInner> fileList;
    auto folderCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
        if (1 == depth)
        {
            auto dirName = utility::FileInfo(name).filename();
            if ("$RECYCLE.BIN" == dirName || "System Volume Information" == dirName) /* 跳过Windows文件系统目录 */
            {
                return false;
            }
        }
        if (!folderCheckFunc || folderCheckFunc(name, attr, depth))
        {
            folderList.emplace_back(InfoInner(name, attr, depth));
        }
        return true;
    };
    auto fileCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
        for (auto ignoreFile : cfg.ignoreFileList)
        {
            if (name == ignoreFile) /* 忽略 */
            {
                return;
            }
        }
        if (fileCheckFunc && !fileCheckFunc(name, attr, depth))
        {
            return;
        }
        fileList.emplace_back(InfoInner(name, attr, depth));
    };
    pi.traverse(folderCb, fileCb, nullptr, true);
    /* 删除最早的文件 */
    std::sort(fileList.begin(), fileList.end(), [](InfoInner a, InfoInner b) { return a.attr.modifyTime < b.attr.modifyTime; });
    size_t deletedSize = 0;
    for (auto file : fileList)
    {
        if (deletedSize >= cfg.clearSize)
        {
            break;
        }
        auto ok = utility::FileInfo(file.name).remove();
        if (fileDeletedCb)
        {
            fileDeletedCb(file.name, file.attr, file.depth, ok);
        }
        if (ok)
        {
            deletedSize += file.attr.size;
        }
    }
    /* 删除空目录 */
    for (auto folder : folderList)
    {
        auto pi = utility::PathInfo(folder.name);
        if (pi.exist() && pi.empty(true))
        {
            auto ok = pi.remove();
            if (folderDeletedCb)
            {
                folderDeletedCb(folder.name, folder.attr, folder.depth, ok);
            }
        }
    }
}

void FileDeleter::deleteExpired(const std::vector<ExpireConfig>& cfgList, const DeleteCheckFunc& folderCheckFunc,
                                const DeletedCallback& folderDeletedCb, const DeleteCheckFunc& fileCheckFunc,
                                const DeletedCallback& fileDeletedCb)
{
    for (auto cfg : cfgList)
    {
        if (cfg.expireTime <= 0) /* 过期时间无效 */
        {
            continue;
        }
        utility::PathInfo pi(cfg.folder);
        if (!pi.exist()) /* 目录不存在 */
        {
            continue;
        }
        auto nowTimestamp = (int64_t)utility::DateTime::getNowTimestamp();
        std::vector<InfoInner> folderList;
        auto folderCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (1 == depth)
            {
                auto dirName = utility::FileInfo(name).filename();
                if ("$RECYCLE.BIN" == dirName || "System Volume Information" == dirName) /* 跳过Windows文件系统目录 */
                {
                    return false;
                }
            }
            if (!folderCheckFunc || folderCheckFunc(name, attr, depth))
            {
                folderList.emplace_back(InfoInner(name, attr, depth));
            }
            return true;
        };
        auto fileCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            for (auto ignoreFile : cfg.ignoreFileList)
            {
                if (name == ignoreFile) /* 忽略 */
                {
                    return;
                }
            }
            if (fileCheckFunc && !fileCheckFunc(name, attr, depth))
            {
                return;
            }
            auto modifyTimestamp = (int64_t)utility::DateTime(attr.modifyTimeFmt()).toTimestamp();
            if (nowTimestamp - modifyTimestamp >= cfg.expireTime) /* 过期, 需要删除 */
            {
                auto ok = utility::FileInfo(name).remove();
                if (fileDeletedCb)
                {
                    fileDeletedCb(name, attr, depth, ok);
                }
            }
        };
        pi.traverse(folderCb, fileCb, nullptr, true);
        /* 删除空目录 */
        for (auto folder : folderList)
        {
            auto pi = utility::PathInfo(folder.name);
            if (pi.exist() && pi.empty(true))
            {
                auto ok = pi.remove();
                if (folderDeletedCb)
                {
                    folderDeletedCb(folder.name, folder.attr, folder.depth, ok);
                }
            }
        }
    }
}
} // namespace toolkit
