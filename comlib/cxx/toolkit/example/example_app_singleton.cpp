#include <chrono>
#include <stdio.h>
#include <string>
#include <thread>

#include "../toolkit/app_singleton.h"

int main()
{
    toolkit::AppSingleton::create("", "", [&]() { printf("------------------ app already running\n"); });
    printf("====================== app start\n");
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
