#pragma once

#include <stdio.h>
#include <string>

#include "../algorithm/sha1/sha1.h"

void testSha1()
{
    printf("\n============================== test sha1 =============================\n");
    std::string str = "0123456789abcdefg";
    printf("str: %s\n", str.c_str());
    char* output = sha1Sign((const unsigned char*)str.c_str(), str.size());
    if (output)
    {
        printf("sha1: %s\n", output);
        free(output);
    }
}
