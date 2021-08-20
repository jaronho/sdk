#pragma once

#include <chrono>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>
#ifdef _WIN32
#else
#include <fcntl.h>
#include <unistd.h>
#endif

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
#ifndef _WIN32
    std::string filename = "/root/workspace/1.txt";
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
    if (fd >= 0)
    {
        printf("=========== is lock: %s\n", utilitiy::System::checkFileLock(fd) ? "true" : "false");
        printf("=========== lock: %s\n", utilitiy::System::tryLockFile(fd, true) ? "true" : "false");
        std::this_thread::sleep_for(std::chrono::seconds(60));
        printf("=========== unlock: %s\n", utilitiy::System::tryLockFile(fd, false) ? "true" : "false");
        close(fd);
    }
#endif
}
