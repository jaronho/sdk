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
    auto timestamp = utilitiy::System::getTimestamp();
    printf("----- [1] timestamp: %f\n", timestamp);
    auto dt = utilitiy::System::getDateTime(timestamp);
    printf("----- [1] date time: %04d-%02d-%02d %02d:%02d:%02d.%03d, week day: %d, year day: %d\n", dt.year, dt.month, dt.day, dt.hour,
           dt.minute, dt.second, dt.millisecond, dt.wday, dt.yday);
    dt = utilitiy::System::getDateTime();
    printf("----- [2] date time: %04d-%02d-%02d %02d:%02d:%02d.%03d, week day: %d, year day: %d\n", dt.year, dt.month, dt.day, dt.hour,
           dt.minute, dt.second, dt.millisecond, dt.wday, dt.yday);
    timestamp = utilitiy::System::dateTimeToTimestamp(dt);
    printf("----- [2] timestamp: %f\n", timestamp);
    printf("\n");
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
