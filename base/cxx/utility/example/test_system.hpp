#pragma once

#include <chrono>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include "../utility/system/system.h"

void testSystem()
{
    printf("\n============================== test system =============================\n");
#ifdef _WIN32
    if (utility::System::isRunAsAdmin())
    {
        printf("running as admin\n");
    }
    else
    {
        printf("running not as admin\n");
    }
#endif
    auto hostname = utility::System::getHostname();
    printf("hostname: %s\n", hostname.c_str());
    printf("------------------------------\n");
    std::string outStr;
#ifdef _WIN32
    int ret = utility::System::runCmd("dir", &outStr);
#else
    int ret = utility::System::runCmd("ls", &outStr);
#endif
    printf("ret: %d\n", ret);
    printf("out: %s\n", outStr.c_str());
    printf("------------------------------\n");
    std::string filename = "1.txt";
#ifdef _WIN32
    HANDLE fd = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, (DWORD)0, NULL);
#else
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
#endif
    if (fd >= 0)
    {
        printf("=========== is lock: %s\n", utility::System::checkFileLock(fd) ? "true" : "false");
        printf("=========== lock: %s\n", utility::System::tryLockUnlockFile(fd, true) ? "true" : "false");
        std::this_thread::sleep_for(std::chrono::seconds(10));
        printf("=========== unlock: %s\n", utility::System::tryLockUnlockFile(fd, false) ? "true" : "false");
#ifdef _WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif
    }
}
