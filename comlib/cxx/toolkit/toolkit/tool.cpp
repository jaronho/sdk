#include "tool.h"

#include <future>
#include <mutex>
#include <string.h>
#include <type_traits>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "algorithm/md5/md5ex.h"
#include "algorithm/xxhash/xxhashex.h"
#include "utility/charset/charset.h"
#include "utility/filesystem/path_info.h"
#include "utility/mmfile/mmfile.h"

namespace toolkit
{
std::string Tool::md5File(const std::string& fullName, const std::function<bool()>& stopFunc, size_t blockSize, bool enableMMFile)
{
    blockSize = blockSize <= 0 ? (1024 * 1024) : (blockSize > (50 * 1024 * 1024) ? (50 * 1024 * 1024) : blockSize);
    std::string output;
    if (enableMMFile) /* 启用内存映射文件 */
    {
        utility::MMFile mf;
        if (mf.open(fullName.c_str(), utility::MMFile::AccessMode::read_only, blockSize))
        {
            char* blockBuffer = (char*)malloc(blockSize);
            if (blockBuffer)
            {
                algorithm::md5_context_t ctx;
                algorithm::md5Init(&ctx);
                while (1)
                {
                    if (stopFunc && stopFunc())
                    {
                        free(blockBuffer);
                        mf.close();
                        return output;
                    }
                    memset(blockBuffer, 0, blockSize);
                    auto count = mf.read(blockBuffer, blockSize);
                    if (0 == count)
                    {
                        break;
                    }
                    md5Update(&ctx, (unsigned char*)blockBuffer, count);
                }
                unsigned char digest[16];
                char* buffer = md5Fini(&ctx, digest, 1);
                if (buffer)
                {
                    output = buffer;
                    free(buffer);
                }
                free(blockBuffer);
            }
            mf.close();
        }
    }
    else /* 本地IO方式读取文件(效率较低) */
    {
#ifdef _WIN32
        size_t codePage = CP_ACP;
        auto coding = utility::Charset::getCoding(fullName);
        if (utility::Charset::Coding::utf8 == coding || utility::Charset::Coding::utf8_bom == coding)
        {
            codePage = CP_UTF8;
        }
        FILE* f = _wfopen(utility::Charset::string2wstring(fullName, codePage).c_str(), L"rb");
#else
        FILE* f = fopen(fullName.c_str(), "rb");
#endif
        if (f)
        {
            output = algorithm::md5SignFileHandleEx(f, blockSize, stopFunc);
            fclose(f);
        }
    }
    return output;
}

std::string
Tool::md5Directory(const std::string& path, const std::function<void(size_t totalCount, size_t totalSize)>& beginCb,
                   const std::function<void(const std::string& name, size_t fileSize, const std::function<std::string()>& calcFunc)>& func,
                   const std::function<bool()>& stopFunc, size_t blockSize, bool enableMMFile)
{
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
        auto value = md5File(
            name,
            [&, stopFunc]() {
                stopFlag = stopFunc ? stopFunc() : false;
                return stopFlag;
            },
            blockSize, enableMMFile);
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
    std::string output;
    {
        std::lock_guard<std::mutex> locker(*mutexInner);
        output = algorithm::md5SignStrList(*md5List, 1);
    }
    return output;
}

uint64_t Tool::xxhashFile(const std::string& fullName, const std::function<bool()>& stopFunc, size_t blockSize, bool enableMMFile)
{
    blockSize = blockSize <= 0 ? (1024 * 1024) : (blockSize > (50 * 1024 * 1024) ? (50 * 1024 * 1024) : blockSize);
    uint64_t output;
    if (enableMMFile) /* 启用内存映射文件 */
    {
        utility::MMFile mf;
        if (mf.open(fullName.c_str(), utility::MMFile::AccessMode::read_only, blockSize))
        {
            char* blockBuffer = (char*)malloc(blockSize);
            if (blockBuffer)
            {
                XXH3_state_t* state = XXH3_createState();
                if (state)
                {
                    XXH3_64bits_reset(state);
                    while (1)
                    {
                        if (stopFunc && stopFunc())
                        {
                            free(blockBuffer);
                            mf.close();
                            return output;
                        }
                        memset(blockBuffer, 0, blockSize);
                        auto count = mf.read(blockBuffer, blockSize);
                        if (0 == count)
                        {
                            break;
                        }
                        XXH3_64bits_update(state, blockBuffer, count);
                    }
                    output = XXH3_64bits_digest(state);
                    XXH3_freeState(state);
                }
                free(blockBuffer);
            }
            mf.close();
        }
    }
    else /* 本地IO方式读取文件(效率较低) */
    {
#ifdef _WIN32
        size_t codePage = CP_ACP;
        auto coding = utility::Charset::getCoding(fullName);
        if (utility::Charset::Coding::utf8 == coding || utility::Charset::Coding::utf8_bom == coding)
        {
            codePage = CP_UTF8;
        }
        FILE* f = _wfopen(utility::Charset::string2wstring(fullName, codePage).c_str(), L"rb");
#else
        FILE* f = fopen(fullName.c_str(), "rb");
#endif
        if (f)
        {
            output = algorithm::xxhash64SignFileHandle(f, blockSize, stopFunc);
            fclose(f);
        }
    }
    return output;
}

uint64_t
Tool::xxhashDirectory(const std::string& path, const std::function<void(size_t totalCount, size_t totalSize)>& beginCb,
                      const std::function<void(const std::string& name, size_t fileSize, const std::function<uint64_t()>& calcFunc)>& func,
                      const std::function<bool()>& stopFunc, size_t blockSize, bool enableMMFile)
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
        return 0;
    }
    /* 遍历计算文件 */
    auto mutexInner = std::make_shared<std::mutex>();
    auto xxhashList = std::make_shared<std::vector<std::string>>();
    auto result = std::make_shared<std::promise<void>>();
    auto innerFunc = [&, stopFunc, blockSize, totalFileCount, mutexInner, xxhashList, result](const std::string& name,
                                                                                              const utility::FileAttribute& attr) {
        bool stopFlag = false;
        auto value = xxhashFile(
            name,
            [&, stopFunc]() {
                stopFlag = stopFunc ? stopFunc() : false;
                return stopFlag;
            },
            blockSize, enableMMFile);
        {
            std::lock_guard<std::mutex> locker(*mutexInner);
            if (stopFlag)
            {
                xxhashList->clear();
                result->set_value();
            }
            else
            {
                char tmp[48] = {0};
                sprintf(tmp, "%llx", value);
                xxhashList->emplace_back(tmp);
                if (xxhashList->size() == totalFileCount)
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
                xxhashList->clear();
                result->set_value();
            }
            return stopFlag;
        },
        true, false);
    result->get_future().share().get();
    uint64_t output;
    {
        std::lock_guard<std::mutex> locker(*mutexInner);
        output = algorithm::xxhash64SignList(*xxhashList, 1);
    }
    return output;
}
} // namespace toolkit
