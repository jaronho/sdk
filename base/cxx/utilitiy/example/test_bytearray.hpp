#pragma once

#include <stdio.h>
#include <string.h>

#include "../utilitiy/bytearray/bytearray.h"

void testBytearry()
{
    printf("\n");
    if (utilitiy::ByteArray::isBigEndium())
    {
        printf("Host byte order: Big-Endium\n");
    }
    else
    {
        printf("Host byte order: Little-Endium\n");
    }
    printf("\n");
}
