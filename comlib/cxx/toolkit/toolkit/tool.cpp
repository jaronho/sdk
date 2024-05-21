#include "tool.h"

#include <algorithm>

#include "algorithm/md5/md5.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

namespace toolkit
{
std::string Tool::md5Directory(const std::string& path, const std::function<void(const std::string& name, size_t fileSize)>& progressCb,
                               const std::function<bool()>& stopFunc, size_t blockSize)
{
    blockSize = blockSize <= 0 ? (1024 * 1024) : (blockSize > (50 * 1024 * 1024) ? (50 * 1024 * 1024) : blockSize);
    std::vector<std::string> md5List;
    /* 遍历计算文件MD5 */
    utility::PathInfo pi(path, true);
    pi.traverse(
        nullptr,
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (progressCb)
            {
                progressCb(name, attr.size);
            }
            auto buffer = algorithm::md5SignFile(name.c_str(), blockSize);
            if (buffer)
            {
                md5List.emplace_back(buffer);
                free(buffer);
            }
            else
            {
                md5List.emplace_back(std::string());
            }
        },
        stopFunc, true, false);
    /* 从小到大排序 */
    std::sort(md5List.begin(), md5List.end(), [](const std::string& a, const std::string& b) { return (a < b); });
    /* 计算总的MD5值 */
    algorithm::md5_context_t ctx;
    md5Init(&ctx);
    for (const auto& value : md5List)
    {
        md5Update(&ctx, (unsigned char*)value.c_str(), value.size());
    }
    std::string value;
    unsigned char digest[16];
    auto buffer = md5Fini(&ctx, digest, 1);
    if (buffer)
    {
        value = buffer;
        free(buffer);
    }
    return value;
}
} // namespace toolkit
