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

void FileDeleter::tryOnce(const std::vector<Config>& cfgList, const DeleteCheckFunc& folderCheckFunc,
                          const DeletedCallback& folderDeletedCb, const DeleteCheckFunc& fileCheckFunc,
                          const DeletedCallback& fileDeletedCb)
{
    for (const auto& cfg : cfgList)
    {
        if (cfg.expireTime <= 0 && cfg.occupySize <= 0) /* 无过期时间限制, 且无分配空间限制 */
        {
            continue;
        }
        utility::PathInfo pi(cfg.folder);
        if (!pi.exist()) /* 目录不存在 */
        {
            continue;
        }
        auto nowTimestamp = (int64_t)utility::DateTime::getNowTimestamp();
        std::vector<InfoInner> folderList, expireFileList, occupyFileList;
        auto folderCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (folderCheckFunc && !folderCheckFunc(name, attr, depth)) /* 目录不允许删除 */
            {
                return false;
            }
            auto modifyTimestamp = (int64_t)utility::DateTime(attr.modifyTimeFmt()).toTimestamp();
            if (cfg.expireTime > 0 && nowTimestamp - modifyTimestamp >= cfg.expireTime) /* 有过期时间限制, 且文件夹过期 */
            {
                folderList.emplace_back(InfoInner(name, attr, depth));
            }
            return true;
        };
        auto fileCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            for (const auto& ignoreFile : cfg.ignoreFileList)
            {
                if (name == ignoreFile) /* 文件被过滤 */
                {
                    return;
                }
            }
            if (fileCheckFunc && !fileCheckFunc(name, attr, depth)) /* 文件不允许删除 */
            {
                return;
            }
            auto modifyTimestamp = (int64_t)utility::DateTime(attr.modifyTimeFmt()).toTimestamp();
            if (cfg.expireTime > 0 && nowTimestamp - modifyTimestamp >= cfg.expireTime) /* 有过期时间限制, 且文件过期 */
            {
                expireFileList.emplace_back(InfoInner(name, attr, depth));
            }
            else if (cfg.occupySize > 0) /* 有分配空间限制 */
            {
                occupyFileList.emplace_back(InfoInner(name, attr, depth));
            }
        };
        pi.traverse(folderCb, fileCb, nullptr, true, false);
        /* 删除空文件夹 */
        for (const auto& folder : folderList)
        {
            auto pi = utility::PathInfo(folder.name);
            if (pi.exist() && pi.empty())
            {
                auto ok = pi.remove();
                if (folderDeletedCb)
                {
                    folderDeletedCb(folder.name, folder.attr, folder.depth, ok);
                }
            }
        }
        /* 删除过期文件 */
        for (const auto& file : expireFileList)
        {
            auto ok = utility::FileInfo(file.name).remove();
            if (fileDeletedCb)
            {
                fileDeletedCb(file.name, file.attr, file.depth, ok);
            }
        }
        /* 删除超出分配空间的最早的文件 */
        if (cfg.occupySize > 0 && !occupyFileList.empty())
        {
            std::sort(occupyFileList.begin(), occupyFileList.end(),
                      [&](const InfoInner& a, const InfoInner& b) { return (a.attr.modifyTime < b.attr.modifyTime); });
            size_t deletedSize = 0;
            for (const auto& file : occupyFileList)
            {
                if (deletedSize >= cfg.occupySize * cfg.occupyLine) /* 判断分配空间的删除线是否已完全释放 */
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
        }
    }
}
} // namespace toolkit
