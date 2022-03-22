#pragma once

#include <stdio.h>
#include <string>

#include "../algorithm/md5/md5.h"

void testMd5()
{
    printf("\n============================== test md5 =============================\n");
    std::string str = "0123456789abcdefg";
    printf("str: %s\n", str.c_str());
    char* output = algorithm::md5SignStr((const unsigned char*)str.c_str(), str.size());
    if (output)
    {
        printf("md5: %s\n", output);
        free(output);
    }
}
