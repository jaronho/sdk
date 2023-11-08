#pragma once

#include <stdio.h>
#include <string>

#include "../algorithm/rc4/rc4.h"

void testRc4()
{
    printf("\n============================== test rc4 =============================\n");
    std::string str = "0123456789abcdefg";
    std::string key = "qq123456";
    printf("str: %s\n", str.c_str());
    printf("key: %s\n", key.c_str());
    unsigned char* output = algorithm::rc4Crypto((unsigned char*)str.data(), str.size(), (const unsigned char*)key.c_str(), key.size());
    printf("encrypt: %s\n         ", (char*)output);
    for (size_t i = 0; i < str.size(); ++i)
    {
        printf("%02x ", output[i]);
    }
    printf("\n");
    output = algorithm::rc4Crypto(output, str.size(), (const unsigned char*)key.c_str(), key.size());
    printf("decrypt: %s\n", (char*)output);
}
