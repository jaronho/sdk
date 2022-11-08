#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../algorithm/snowflake/snowflake.h"

void testSnowflake()
{
    printf("\n============================== test snowflake =============================\n");
    algorithm::Snowflake sf(11111, 22222);
    printf("snowflake seq id[0]: %llu\n", sf.generate());
    printf("snowflake seq id[1]: %llu\n", algorithm::Snowflake::easyGenerate());
    printf("snowflake seq id[2]: %llu\n", algorithm::Snowflake::easyGenerate());
}
