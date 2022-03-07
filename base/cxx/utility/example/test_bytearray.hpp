#pragma once

#include <stdio.h>
#include <string.h>

#include "../utility/bytearray/bytearray.h"

void testBytearry()
{
    printf("\n");
    bool bigEndium = utility::ByteArray::isBigEndium();
    if (bigEndium)
    {
        printf("Host byte order: Big-Endium\n");
    }
    else
    {
        printf("Host byte order: Little-Endium\n");
    }
    long long l_1 = 0x0102030405060708;
    printf("--- l_1(%s): 0x%016llx\n", bigEndium ? "   Big" : "Little", l_1);
    long long l_2 = utility::ByteArray::swab64(l_1);
    printf("--- l_2(%s): 0x%016llx\n", bigEndium ? "Little" : "   Big", l_2);
    long long l_3 = utility::ByteArray::swab64(l_2);
    printf("--- l_3(%s): 0x%016llx\n", bigEndium ? "   Big" : "Little", l_3);
    unsigned char b_1[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    printf("=== b_1: 0x");
    for (int i = 0; i < 8; ++i)
    {
        printf("%02x", b_1[i]);
    }
    printf("\n");
    long long b_2 = utility::ByteArray::swab64(b_1);
    printf("=== b_2: 0x%016llx\n", b_2);
    printf("\n");
}
