#include "xxhashex.h"

#include <algorithm>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "xxhash.h"

namespace algorithm
{
uint64_t xxhash64Sign(const char* data, size_t dataLen)
{
    XXH64_hash_t output = 0;
    if (data && dataLen > 0)
    {
        XXH3_state_t* state = XXH3_createState();
        if (state)
        {
            if (XXH_OK == XXH3_64bits_reset(state))
            {
                XXH3_64bits_update(state, data, dataLen);
                output = XXH3_64bits_digest(state);
            }
            XXH3_freeState(state);
        }
    }
    return output;
}

uint64_t xxhash64SignList(std::vector<std::string> dataList, int sortFlag)
{
    XXH64_hash_t output = 0;
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
        /* 计算总的xxhash值 */
        XXH3_state_t* state = XXH3_createState();
        if (state)
        {
            if (XXH_OK == XXH3_64bits_reset(state))
            {
                for (const auto& data : dataList)
                {
                    XXH3_64bits_update(state, data.data(), data.size());
                }
                output = XXH3_64bits_digest(state);
            }
            XXH3_freeState(state);
        }
    }
    return output;
}

uint64_t xxhash64SignFileHandle(FILE* handle, size_t blockSize, const std::function<bool()>& stopFunc)
{
    blockSize = blockSize >= 1024 ? blockSize : 1024;
    XXH64_hash_t output = 0;
    if (handle)
    {
        char* blockBuffer = (char*)malloc(blockSize);
        if (blockBuffer)
        {
            XXH3_state_t* state = XXH3_createState();
            if (state)
            {
                if (XXH_OK == XXH3_64bits_reset(state))
                {
                    unsigned long long offset = 0, count = blockSize;
                    while (count > 0)
                    {
                        if (stopFunc && stopFunc())
                        {
                            XXH3_freeState(state);
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
                        XXH3_64bits_update(state, blockBuffer, count);
                    }
                    output = XXH3_64bits_digest(state);
                }
                XXH3_freeState(state);
            }
            free(blockBuffer);
        }
    }
    return output;
}

uint64_t xxhash64SignFile(const std::string& filename, size_t blockSize, const std::function<bool()>& stopFunc)
{
    blockSize = blockSize >= 1024 ? blockSize : 1024;
    uint64_t output = 0;
    if (!filename.empty())
    {
        FILE* f = fopen(filename.c_str(), "rb");
        if (f)
        {
            output = xxhash64SignFileHandle(f, blockSize, stopFunc);
            fclose(f);
        }
    }
    return output;
}
} // namespace algorithm
