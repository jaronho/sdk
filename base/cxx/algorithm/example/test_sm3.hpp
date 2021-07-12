#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../algorithm/sm3/sm3.h"

void testSm3()
{
    printf("\n============================== test sm3 =============================\n");
    std::string input = "0123456789abcdefg";
    int ilen = 3;
    unsigned char output[32];
    int i;
    sm3_context ctx;

    printf("Message:\n   ");
    printf("%s\n", input.c_str());

    sm3Sign((const unsigned char*)input.c_str(), ilen, output);
    printf("Hash:\n   ");
    for (i = 0; i < 32; i++)
    {
        printf("%02x ", output[i]);
    }
    printf("\n\n");

    printf("Message:\n   ");
    for (i = 0; i < 16; i++)
    {
        printf("abcd");
    }
    printf("\n");

    sm3Start(&ctx);
    for (i = 0; i < 16; i++)
    {
        sm3Update(&ctx, (unsigned char*)"abcd", 4);
    }
    sm3Finish(&ctx, output);
    memset(&ctx, 0, sizeof(sm3_context));

    printf("Hash:\n   ");
    for (i = 0; i < 32; i++)
    {
        printf("%02x ", output[i]);
    }
    printf("\n");
}
