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

namespace toolkit
{
std::string Tool::md5File(const std::string& fullName, const std::vector<size_t>& segSizeList, const std::function<bool()>& stopFunc,
                          size_t blockSize)
{
    blockSize = blockSize <= 0 ? (1024 * 1024) : (blockSize > (50 * 1024 * 1024) ? (50 * 1024 * 1024) : blockSize);
    std::string output;
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
        char* blockBuffer = (char*)malloc(blockSize);
        if (blockBuffer)
        {
#ifdef _WIN32
            _fseeki64(f, 0, SEEK_END);
            long long fileSize = _ftelli64(f);
#else
            fseeko64(f, 0, SEEK_END);
            long long fileSize = ftello64(f);
#endif
            long long totalSegSize = 0;
            for (auto segSize : segSizeList)
            {
                totalSegSize += segSize;
            }
            algorithm::md5_context_t ctx;
            algorithm::md5Init(&ctx);
            unsigned long long offset = 0, count = blockSize;
            if (segSizeList.empty() || totalSegSize + segSizeList.size() >= fileSize) /* 计算全部内容 */
            {
                while (count > 0)
                {
                    if (stopFunc && stopFunc())
                    {
                        free(blockBuffer);
                        fclose(f);
                        return output;
                    }
#ifdef _WIN32
                    _fseeki64(f, offset, SEEK_SET);
#else
                    fseeko64(f, offset, SEEK_SET);
#endif
                    count = fread(blockBuffer, 1, blockSize, f);
                    offset += count;
                    algorithm::md5Update(&ctx, (unsigned char*)blockBuffer, count);
                }
            }
            else /* 计算分段内容 */
            {
                long long dist = 0;
                if (segSizeList.size() > 1)
                {
                    dist = (fileSize - totalSegSize) / (segSizeList.size() - 1); /* 计算分段间距 */
                }
                for (auto segSize : segSizeList)
                {
                    size_t readedSize = 0, buffSize = 0;
                    while (readedSize < segSize)
                    {
                        if (stopFunc && stopFunc())
                        {
                            free(blockBuffer);
                            fclose(f);
                            return output;
                        }
#ifdef _WIN32
                        _fseeki64(f, offset, SEEK_SET);
#else
                        fseeko64(f, offset, SEEK_SET);
#endif
                        buffSize = segSize - readedSize;
                        if (buffSize > blockSize)
                        {
                            buffSize = blockSize;
                        }
                        count = fread(blockBuffer, 1, buffSize, f);
                        readedSize += count;
                        offset += count;
                        algorithm::md5Update(&ctx, (unsigned char*)blockBuffer, count);
                    }
                    offset += dist;
                }
            }
            unsigned char digest[16];
            char* buffer = algorithm::md5Fini(&ctx, digest, 1);
            if (buffer)
            {
                output = buffer;
                free(buffer);
            }
            free(blockBuffer);
        }
        fclose(f);
    }
    return output;
}

std::string
Tool::md5Directory(const std::string& path, const std::function<std::vector<size_t>(const std::string& name, size_t fileSize)>& segSizeFunc,
                   const std::function<void(size_t totalCount, size_t totalSize)>& beginCb,
                   const std::function<void(const std::string& name, size_t fileSize, const std::function<std::string()>& calcFunc)>& func,
                   const std::function<bool()>& stopFunc, size_t blockSize)
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
    auto innerFunc = [&, segSizeFunc, stopFunc, blockSize, totalFileCount, mutexInner, md5List,
                      result](const std::string& name, const utility::FileAttribute& attr) {
        std::vector<size_t> segSizeList;
        if (segSizeFunc)
        {
            segSizeList = segSizeFunc(name, attr.size);
        }
        bool stopFlag = false;
        auto value = md5File(
            name, segSizeList,
            [&, stopFunc]() {
                stopFlag = stopFunc ? stopFunc() : false;
                return stopFlag;
            },
            blockSize);
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

uint64_t Tool::xxhashFile(const std::string& fullName, const std::vector<size_t>& segSizeList, const std::function<bool()>& stopFunc,
                          size_t blockSize)
{
    blockSize = blockSize <= 0 ? (1024 * 1024) : (blockSize > (50 * 1024 * 1024) ? (50 * 1024 * 1024) : blockSize);
    uint64_t output = 0;
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
        char* blockBuffer = (char*)malloc(blockSize);
        if (blockBuffer)
        {
#ifdef _WIN32
            _fseeki64(f, 0, SEEK_END);
            long long fileSize = _ftelli64(f);
#else
            fseeko64(f, 0, SEEK_END);
            long long fileSize = ftello64(f);
#endif
            long long totalSegSize = 0;
            for (auto segSize : segSizeList)
            {
                totalSegSize += segSize;
            }
            XXH3_state_t* state = XXH3_createState();
            if (state)
            {
                if (XXH_OK == XXH3_64bits_reset(state))
                {
                    unsigned long long offset = 0, count = blockSize;
                    if (segSizeList.empty() || totalSegSize + segSizeList.size() >= fileSize) /* 计算全部内容 */
                    {
                        while (count > 0)
                        {
                            if (stopFunc && stopFunc())
                            {
                                XXH3_freeState(state);
                                free(blockBuffer);
                                fclose(f);
                                return output;
                            }
#ifdef _WIN32
                            _fseeki64(f, offset, SEEK_SET);
#else
                            fseeko64(f, offset, SEEK_SET);
#endif
                            count = fread(blockBuffer, 1, blockSize, f);
                            offset += count;
                            XXH3_64bits_update(state, blockBuffer, count);
                        }
                    }
                    else /* 计算分段内容 */
                    {
                        long long dist = 0;
                        if (segSizeList.size() > 1)
                        {
                            dist = (fileSize - totalSegSize) / (segSizeList.size() - 1); /* 计算分段间距 */
                        }
                        for (auto segSize : segSizeList)
                        {
                            size_t readedSize = 0, buffSize = 0;
                            while (readedSize < segSize)
                            {
                                if (stopFunc && stopFunc())
                                {
                                    XXH3_freeState(state);
                                    free(blockBuffer);
                                    fclose(f);
                                    return output;
                                }
#ifdef _WIN32
                                _fseeki64(f, offset, SEEK_SET);
#else
                                fseeko64(f, offset, SEEK_SET);
#endif
                                buffSize = segSize - readedSize;
                                if (buffSize > blockSize)
                                {
                                    buffSize = blockSize;
                                }
                                count = fread(blockBuffer, 1, buffSize, f);
                                readedSize += count;
                                offset += count;
                                XXH3_64bits_update(state, blockBuffer, count);
                            }
                            offset += dist;
                        }
                    }
                    output = XXH3_64bits_digest(state);
                }
                XXH3_freeState(state);
            }
            free(blockBuffer);
        }
        fclose(f);
    }
    return output;
}

uint64_t
Tool::xxhashDirectory(const std::string& path,
                      const std::function<std::vector<size_t>(const std::string& name, size_t fileSize)>& segSizeFunc,
                      const std::function<void(size_t totalCount, size_t totalSize)>& beginCb,
                      const std::function<void(const std::string& name, size_t fileSize, const std::function<uint64_t()>& calcFunc)>& func,
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
        return 0;
    }
    /* 遍历计算文件 */
    auto mutexInner = std::make_shared<std::mutex>();
    auto xxhashList = std::make_shared<std::vector<std::string>>();
    auto result = std::make_shared<std::promise<void>>();
    auto innerFunc = [&, segSizeFunc, stopFunc, blockSize, totalFileCount, mutexInner, xxhashList,
                      result](const std::string& name, const utility::FileAttribute& attr) {
        std::vector<size_t> segSizeList;
        if (segSizeFunc)
        {
            segSizeList = segSizeFunc(name, attr.size);
        }
        bool stopFlag = false;
        auto value = xxhashFile(
            name, segSizeList,
            [&, stopFunc]() {
                stopFlag = stopFunc ? stopFunc() : false;
                return stopFlag;
            },
            blockSize);
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
