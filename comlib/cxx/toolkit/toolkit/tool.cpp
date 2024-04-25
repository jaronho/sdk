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
                               const std::function<void(const std::string& name, bool isDir, size_t fileSize)>& progressCb,
                               const std::function<std::string(const std::string& name)>& md5FileFunc, size_t blockSize)
{
    std::string value;
    utility::PathInfo pi(path);
    pi.traverse(
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (progressCb)
            {
                progressCb(name, attr.isDir, attr.size);
            }
            auto relativeName = utility::StrTool::replace(name.substr(pi.path().size()), "\\\\", "/");
            value += relativeName;
            value = algorithm::md5SignStr((const unsigned char*)value.c_str(), value.size());
            return true;
        },
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (progressCb)
            {
                progressCb(name, attr.isDir, attr.size);
            }
            auto relativeName = utility::StrTool::replace(name.substr(pi.path().size()), "\\\\", "/");
            value += relativeName;
            if (md5FileFunc)
            {
                value += md5FileFunc(name);
            }
            else
            {
                auto buf = algorithm::md5SignFile(name.c_str(), blockSize);
                if (buf)
                {
                    value += buf;
                    free(buf);
                }
            }
            value = algorithm::md5SignStr((const unsigned char*)value.c_str(), value.size());
        },
        nullptr, true, false);
    return value;
}
} // namespace toolkit
