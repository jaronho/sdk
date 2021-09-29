#pragma once

#include <stdio.h>
#include <string.h>

#include "../utilitiy/bytearray/bytearray.h"

void testBytearry()
{
    if (utilitiy::ByteArray::isBigEndium())
    {
        printf("Host: Big Endium\n");
    }
    else
    {
        printf("Host: Little Endium\n");
    }
    printf("\n");
}
