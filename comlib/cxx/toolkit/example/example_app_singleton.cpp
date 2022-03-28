#include <chrono>
#include <stdio.h>
#include <string>
#include <thread>

#include "../toolkit/app_singleton.h"

int main()
{
    toolkit::AppSingleton::create(
        "", "", [&](const std::string& pidFile) { printf("------------------ app already running, pidFile: %s\n", pidFile.c_str()); });
    printf("====================== app start\n");
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
