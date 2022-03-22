#pragma once

#include <stdio.h>
#include <string>

#include "../algorithm/sha1/hamc_sha1.h"
#include "../algorithm/sha1/sha1.h"

void testSha1()
{
    printf("\n============================== test sha1 =============================\n");
    std::string str = "0123456789abcdefg";
    printf("str: %s\n", str.c_str());
    char* output1 = sha1SignStr((const unsigned char*)str.c_str(), str.size());
    if (output1)
    {
        printf("sha1: %s\n", output1);
        free(output1);
    }
    printf("\n============================== test hmac-sha1 =============================\n");
    std::string data = "2022-03-22 15:20:04";
    std::string key = "qq123456";
    printf("data: %s\n", data.c_str());
    printf(" key: %s\n", key.c_str());
    unsigned char digest[20] = {0};
    hamcSha1((const unsigned char*)data.c_str(), data.size(), (const unsigned char*)key.c_str(), key.size(), digest);
    char* output2 = hamcSha1Str((const unsigned char*)data.c_str(), data.size(), (const unsigned char*)key.c_str(), key.size());
    if (output2)
    {
        printf("hamc-sha1: %s\n", output2);
        free(output2);
    }
}
