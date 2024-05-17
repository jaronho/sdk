#include "tool.h"

#include <algorithm>

#include "algorithm/md5/md5.h"
#include "utility/charset/charset.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

namespace toolkit
{
std::string
Tool::md5Directory(const std::string& path, int type,
                   const std::function<void(size_t folderCount, size_t folderCalcCount, size_t fileCount, size_t totalSize)>& beginCb,
                   const std::function<void(const std::string& name, const std::string& relName, bool isDir, size_t fileSize)>& progressCb,
                   const std::function<bool()>& stopFunc, size_t blockSize)
{
    blockSize = blockSize <= 0 ? (1024 * 1024) : (blockSize > (50 * 1024 * 1024) ? (50 * 1024 * 1024) : blockSize);
    struct _TmpInfo
    {
        std::string name;
        std::string relativeName;
        std::string relativeNameLowcase;
        utility::FileAttribute attr;
        int depth;
    };
    size_t folderCount = 0, folderCalcCount = 0, fileCount = 0, totalSize = 0;
    std::vector<_TmpInfo> infoList;
    /* 遍历目录收集信息 */
    utility::PathInfo pi(path, true);
    pi.traverse(
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            auto relativeName = utility::StrTool::replace(name.substr(pi.path().size()), "\\", "/");
            if (!relativeName.empty() && '/' == relativeName[0])
            {
                relativeName.erase(0);
            }
            ++folderCount;
            bool calcFlag = false;
            if (1 == type)
            {
                calcFlag = true;
            }
            else if (2 == type)
            {
                calcFlag = (utility::Charset::isAscii(name) && utility::isValidFilename(utility::FileInfo(name).filename(), 0));
            }
            if (calcFlag)
            {
                ++folderCalcCount;
                _TmpInfo info;
                info.name = name;
                info.relativeName = relativeName;
                info.relativeNameLowcase = relativeName;
                std::transform(info.relativeNameLowcase.begin(), info.relativeNameLowcase.end(), info.relativeNameLowcase.begin(), tolower);
                info.attr = attr;
                info.depth = depth;
                infoList.emplace_back(info);
            }
            return true;
        },
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            auto relativeName = utility::StrTool::replace(name.substr(pi.path().size()), "\\", "/");
            if (!relativeName.empty() && '/' == relativeName[0])
            {
                relativeName.erase(0);
            }
            ++fileCount;
            totalSize += attr.size;
            _TmpInfo info;
            info.name = name;
            info.relativeName = relativeName;
            info.relativeNameLowcase = relativeName;
            std::transform(info.relativeNameLowcase.begin(), info.relativeNameLowcase.end(), info.relativeNameLowcase.begin(), tolower);
            info.attr = attr;
            info.depth = depth;
            infoList.emplace_back(info);
        },
        stopFunc, true, false);
    if (beginCb)
    {
        beginCb(folderCount, folderCalcCount, fileCount, totalSize);
    }
    /* 按路径从小到到大排序 */
    std::sort(infoList.begin(), infoList.end(), [](const _TmpInfo& a, const _TmpInfo& b) {
        if (a.relativeNameLowcase < b.relativeNameLowcase)
        {
            return true;
        }
        else if (a.relativeNameLowcase == b.relativeNameLowcase)
        {
            return a.relativeName < b.relativeName;
        }
        return false;
    });
    /* 按顺序计算MD5值 */
    algorithm::md5_context_t ctx;
    md5Init(&ctx);
    char* buffer = NULL;
    for (const auto& info : infoList)
    {
        if (progressCb)
        {
            progressCb(info.name, info.relativeName, info.attr.isDir, info.attr.size);
        }
        if (info.attr.isDir)
        {
            md5Update(&ctx, (unsigned char*)info.relativeName.c_str(), info.relativeName.size());
        }
        else if (info.attr.isFile)
        {
            bool calcFlag = false;
            if (1 == type)
            {
                calcFlag = true;
            }
            else if (2 == type)
            {
                calcFlag = (utility::Charset::isAscii(info.name) && utility::isValidFilename(utility::FileInfo(info.name).filename(), 0));
            }
            if (calcFlag)
            {
                md5Update(&ctx, (unsigned char*)info.relativeName.c_str(), info.relativeName.size());
            }
            FILE* f = fopen(info.name.c_str(), "rb");
            if (f)
            {
                if (!buffer)
                {
                    buffer = (char*)malloc(blockSize);
                }
                if (buffer)
                {
                    unsigned long long offset = 0, count = blockSize;
                    while (count > 0)
                    {
#ifdef _WIN32
                        _fseeki64(f, offset, SEEK_SET);
#else
                        fseeko64(f, offset, SEEK_SET);
#endif
                        count = fread(buffer, 1, blockSize, f);
                        offset += count;
                        md5Update(&ctx, (unsigned char*)buffer, count);
                    }
                }
                fclose(f);
            }
        }
    }
    if (buffer)
    {
        free(buffer);
    }
    std::string value;
    unsigned char digest[16];
    auto buf = md5Fini(&ctx, digest, 1);
    if (buf)
    {
        value = buf;
        free(buf);
    }
    return value;
}
} // namespace toolkit
