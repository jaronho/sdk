#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utilitiy/system/system.h"

void testSystem()
{
    printf("\n============================== test system =============================\n");
    std::string outStr;
#ifdef _WIN32
    int ret = utilitiy::System::runCmd("dir", &outStr);
#else
    int ret = utilitiy::System::runCmd("ls", &outStr);
#endif
    printf("ret: %d\n", ret);
    printf("out: %s\n", outStr.c_str());
    printf("------------------------------\n");
}
