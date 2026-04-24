#include <chrono>
#include <mutex>
#include <stdio.h>
#include <string>
#include <thread>

#include "loop_proxy.hpp"

int main()
{
    while (1)
    {
        LoopProxy::getInstance().onLoop([&]() {}, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
