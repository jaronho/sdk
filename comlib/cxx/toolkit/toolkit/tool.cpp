#include "tool.h"

#include "algorithm/md5/md5.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/process/process.h"
#include "utility/strtool/strtool.h"
#include "utility/system/system.h"

namespace toolkit
{
std::string Tool::md5Directory(const std::string& path,
                               const std::function<void(size_t nowFolderCount, size_t nowFileCount, size_t nowFileSize)>& progressCb)
{
    std::string value;
    size_t nowFolderCount = 0, nowFileCount = 0, nowFileSize = 0;
    utility::PathInfo pi(path);
    pi.traverse(
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            auto relativeName = utility::StrTool::replace(name.substr(pi.path().size()), "\\\\", "/");
            value += relativeName;
            value = algorithm::md5SignStr((const unsigned char*)value.c_str(), value.size());
            ++nowFolderCount;
            if (progressCb)
            {
                progressCb(nowFolderCount, nowFileCount, nowFileSize);
            }
            return true;
        },
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            auto relativeName = utility::StrTool::replace(name.substr(pi.path().size()), "\\\\", "/");
            value += relativeName;
            auto buf = algorithm::md5SignFile(name.c_str(), 1024 * 1024);
            if (buf)
            {
                value += buf;
                free(buf);
            }
            value = algorithm::md5SignStr((const unsigned char*)value.c_str(), value.size());
            ++nowFileCount;
            nowFileSize += attr.size;
            if (progressCb)
            {
                progressCb(nowFolderCount, nowFileCount, nowFileSize);
            }
        },
        nullptr, true, false);
    return value;
}
} // namespace toolkit
