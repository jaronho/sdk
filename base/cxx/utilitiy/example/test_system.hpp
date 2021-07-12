#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utilitiy/system/system.h"

void testSystem()
{
    printf("\n============================== test system =============================\n");
#if _WIN32
    auto result = utilitiy::System::runCmd("dir");
#else
    auto result = utilitiy::System::runCmd("ls");
#endif
    for (auto line : result)
    {
        printf("%s\n", line.c_str());
    }
    printf("------------------------------\n");
}
