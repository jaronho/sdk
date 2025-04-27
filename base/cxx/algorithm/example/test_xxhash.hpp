#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../algorithm/xxhash/xxhash.h"

char buffer[1 * 1024 * 1024]; /* 经测试, 对于大文件(比如超过1G), 使用1Mb读取文件效率最高 */

void testXxhash()
{
    printf("\n============================== test xxhash =============================\n");
    {
        printf("test string\n");
        std::string str = "hello, world!";
        XXH3_state_t* state = XXH3_createState();
        if (state)
        {
            if (XXH_OK == XXH3_64bits_reset(state))
            {
                XXH3_64bits_update(state, str.c_str(), str.size());
                XXH64_hash_t result = XXH3_64bits_digest(state);
                printf("xxh64: %llx\n", result);
            }
            XXH3_freeState(state);
        }
    }
    {
        printf("test file\n");
        FILE* f = fopen("D:\\system\\iso\\Kylin-Server-10-SP2-Release-Build09-20210524-x86_64.iso", "rb");
        if (f)
        {
            XXH3_state_t* state = XXH3_createState();
            if (state)
            {
                if (XXH_OK == XXH3_64bits_reset(state))
                {
                    size_t count;
                    while ((count = fread(buffer, 1, sizeof(buffer), f)) != 0)
                    {
                        XXH3_64bits_update(state, buffer, count);
                    }
                    XXH64_hash_t result = XXH3_64bits_digest(state);
                    printf("xxh64: %llx\n", result);
                }
                XXH3_freeState(state);
            }
            fclose(f);
        }
    }
}
