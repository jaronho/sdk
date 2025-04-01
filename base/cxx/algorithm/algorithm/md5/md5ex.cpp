#include "md5ex.h"

#include <algorithm>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"

namespace algorithm
{
std::string md5SignStrEx(const unsigned char* data, size_t dataLen)
{
    std::string output;
    char* buffer = md5SignStr(data, dataLen);
    if (buffer)
    {
        output = buffer;
        free(buffer);
    }
    return output;
}

std::string md5SignStrList(std::vector<std::string> dataList, int sortFlag)
{
    std::string output;
    if (!dataList.empty())
    {
        if (1 == sortFlag) /* 从小到大排序 */
        {
            std::sort(dataList.begin(), dataList.end(), [](const std::string& a, const std::string& b) { return (a < b); });
        }
        else if (2 == sortFlag) /* 从大到小排序 */
        {
            std::sort(dataList.begin(), dataList.end(), [](const std::string& a, const std::string& b) { return (a > b); });
        }
        /* 计算总的MD5值 */
        algorithm::md5_context_t ctx;
        algorithm::md5Init(&ctx);
        for (const auto& data : dataList)
        {
            algorithm::md5Update(&ctx, (unsigned char*)data.c_str(), data.size());
        }
        unsigned char digest[16];
        auto buffer = algorithm::md5Fini(&ctx, digest, 1);
        if (buffer)
        {
            output = buffer;
            free(buffer);
        }
    }
    return output;
}

std::string md5SignFileHandleEx(FILE* handle, size_t blockSize, const std::function<bool()>& stopFunc)
{
    blockSize = blockSize >= 1024 ? blockSize : 1024;
    std::string output;
    if (handle)
    {
        char* blockBuffer = (char*)malloc(blockSize);
        if (blockBuffer)
        {
            md5_context_t ctx;
            md5Init(&ctx);
            unsigned long long offset = 0, count = blockSize;
            while (count > 0)
            {
                if (stopFunc && stopFunc())
                {
                    free(blockBuffer);
                    return output;
                }
#ifdef _WIN32
                _fseeki64(handle, offset, SEEK_SET);
#else
                fseeko64(handle, offset, SEEK_SET);
#endif
                count = fread(blockBuffer, 1, blockSize, handle);
                offset += count;
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
    }
    return output;
}

std::string md5SignFileEx(const std::string& filename, size_t blockSize, const std::function<bool()>& stopFunc)
{
    blockSize = blockSize >= 1024 ? blockSize : 1024;
    std::string output;
    if (!filename.empty())
    {
        FILE* f = fopen(filename.c_str(), "rb");
        if (f)
        {
            output = md5SignFileHandleEx(f, blockSize, stopFunc);
            fclose(f);
        }
    }
    return output;
}
} // namespace algorithm
