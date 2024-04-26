#include "tool.h"

#include "algorithm/md5/md5.h"
#include "utility/charset/charset.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/process/process.h"
#include "utility/strtool/strtool.h"
#include "utility/system/system.h"

namespace toolkit
{
std::string Tool::md5Directory(const std::string& path,
                               const std::function<void(const std::string& name, bool isDir, size_t fileSize)>& progressCb,
                               size_t blockSize)
{
    blockSize = blockSize > (50 * 1024 * 1024) ? (50 * 1024 * 1024) : blockSize;
    algorithm::md5_context_t ctx;
    md5Init(&ctx);
    char* buffer = NULL;
    utility::PathInfo pi(path, true);
    pi.traverse(
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (1 == depth)
            {
                auto dirName = utility::FileInfo(name).filename();
                if ("$RECYCLE.BIN" == dirName || "System Volume Information" == dirName) /* 跳过Windows文件系统目录 */
                {
                    return false;
                }
            }
            if (progressCb)
            {
                progressCb(name, attr.isDir, attr.size);
            }
            auto relativeName = utility::StrTool::replace(name.substr(pi.path().size()), "\\", "/");
            if (!relativeName.empty() && '/' == relativeName[0])
            {
                relativeName.erase(0);
            }
            if (utility::Charset::Coding::gbk == utility::Charset::getCoding(relativeName))
            {
                relativeName = utility::Charset::gbkToUtf8(relativeName);
            }
            md5Update(&ctx, (unsigned char*)relativeName.c_str(), relativeName.size());
            return true;
        },
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (progressCb)
            {
                progressCb(name, attr.isDir, attr.size);
            }
            auto relativeName = utility::StrTool::replace(name.substr(pi.path().size()), "\\", "/");
            if (!relativeName.empty() && '/' == relativeName[0])
            {
                relativeName.erase(0);
            }
            if (utility::Charset::Coding::gbk == utility::Charset::getCoding(relativeName))
            {
                relativeName = utility::Charset::gbkToUtf8(relativeName);
            }
            md5Update(&ctx, (unsigned char*)relativeName.c_str(), relativeName.size());
            FILE* f = fopen(name.c_str(), "rb");
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
        },
        nullptr, true, false);
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
