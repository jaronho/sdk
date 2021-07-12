#pragma once

#include <stdio.h>
#include <string>

#include "../algorithm/base64/base64.h"

void testBase64()
{
    printf("\n============================== test base64 =============================\n");
    std::string str = "0123456789abcdefg";
    std::string base64;
    printf("str: %s\n", str.c_str());
    unsigned char* encodeOutput;
    unsigned int encodeOutputLen = base64Encode((const unsigned char*)str.c_str(), str.size(), &encodeOutput);
    if (encodeOutputLen > 0)
    {
        base64 = (char*)encodeOutput;
        free(encodeOutput);
    }
    printf("encode out: %s\n", base64.c_str());
    if (!base64.empty())
    {
        unsigned char* decodeOutput;
        unsigned int decodeOutputLen = base64Decode((const unsigned char*)base64.c_str(), base64.size(), &decodeOutput);
        if (decodeOutputLen > 0)
        {
            printf("decode out: %s\n", (char*)decodeOutput);
            free(decodeOutput);
        }
    }
}
