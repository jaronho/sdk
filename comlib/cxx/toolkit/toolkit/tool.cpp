#include "tool.h"

#include <future>
#include <mutex>
#include <type_traits>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "algorithm/md5/md5ex.h"
#include "utility/charset/charset.h"
#include "utility/filesystem/path_info.h"

namespace toolkit
{
std::string
Tool::md5Directory(const std::string& path, const std::function<void(size_t totalCount, size_t totalSize)>& beginCb,
                   const std::function<void(const std::string& name, size_t fileSize, const std::function<std::string()>& calcFunc)>& func,
                   const std::function<bool()>& stopFunc, size_t blockSize)
{
    blockSize = blockSize <= 0 ? (1024 * 1024) : (blockSize > (50 * 1024 * 1024) ? (50 * 1024 * 1024) : blockSize);
    /* 计算文件数量和总大小 */
    size_t totalFileCount = 0, totalFileSize = 0;
    utility::PathInfo pi(path, true);
    pi.traverse(
        nullptr,
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            ++totalFileCount;
            totalFileSize += attr.size;
        },
        nullptr, true, false);
    if (beginCb)
    {
        beginCb(totalFileCount, totalFileSize);
    }
    if (0 == totalFileCount || 0 == totalFileSize)
    {
        return std::string();
    }
    /* 遍历计算文件 */
    auto mutexInner = std::make_shared<std::mutex>();
    auto md5List = std::make_shared<std::vector<std::string>>();
    auto result = std::make_shared<std::promise<void>>();
    auto innerFunc = [&, stopFunc, blockSize, totalFileCount, mutexInner, md5List, result](const std::string& name,
                                                                                           const utility::FileAttribute& attr) {
        bool stopFlag = false;
        std::string value;
#ifdef _WIN32
        size_t codePage = CP_ACP;
        auto coding = utility::Charset::getCoding(name);
        if (utility::Charset::Coding::utf8 == coding || utility::Charset::Coding::utf8_bom == coding)
        {
            codePage = CP_UTF8;
        }
        FILE* f = _wfopen(utility::Charset::string2wstring(name, codePage).c_str(), L"rb");
#else
        FILE* f = fopen(name.c_str(), "rb");
#endif
        if (f)
        {
            value = algorithm::md5SignFileHandleEx(f, blockSize, [&, stopFunc]() {
                stopFlag = stopFunc ? stopFunc() : false;
                return stopFlag;
            });
            fclose(f);
        }
        {
            std::lock_guard<std::mutex> locker(*mutexInner);
            if (stopFlag)
            {
                md5List->clear();
                result->set_value();
            }
            else
            {
                md5List->emplace_back(value);
                if (md5List->size() == totalFileCount)
                {
                    result->set_value();
                }
            }
        }
        return value;
    };
    pi.traverse(
        nullptr,
        [&, innerFunc](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (func)
            {
                func(name, attr.size, [&, innerFunc, name, attr]() { return innerFunc(name, attr); });
            }
            else
            {
                innerFunc(name, attr);
            }
        },
        [&, stopFunc]() {
            auto stopFlag = stopFunc ? stopFunc() : false;
            if (stopFlag)
            {
                std::lock_guard<std::mutex> locker(*mutexInner);
                md5List->clear();
                result->set_value();
            }
            return stopFlag;
        },
        true, false);
    result->get_future().share().get();
    std::string value;
    {
        std::lock_guard<std::mutex> locker(*mutexInner);
        value = algorithm::md5SignStrList(*md5List, 1);
    }
    return value;
}
} // namespace toolkit
