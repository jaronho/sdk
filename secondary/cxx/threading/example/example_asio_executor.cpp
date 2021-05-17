#include "threading/thread_proxy.hpp"

#include <iostream>

threading::ExecutorPtr g_workers;

int main()
{
    g_workers = threading::ThreadProxy::createAsioExecutor("workers", 6);
    int64_t index = 0;
    while (1)
    {
        threading::ThreadProxy::async(
            "task_" + std::to_string(++index), [index]() { std::this_thread::sleep_for(std::chrono::milliseconds(2000)); }, g_workers);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
