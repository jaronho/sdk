#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utilitiy/system/system.h"

void testSystem()
{
    printf("\n============================== test system =============================\n");
    std::vector<std::string> result;
#if _WIN32
    int ret = utilitiy::System::runCmd("dir", &result);
#else
    int ret = utilitiy::System::runCmd("ls", &result);
#endif
    printf("ret: %d\n", ret);
    for (auto line : result)
    {
        printf("%s\n", line.c_str());
    }
    printf("------------------------------\n");
}
