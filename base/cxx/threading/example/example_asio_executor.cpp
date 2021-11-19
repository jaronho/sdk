#include <iostream>

#include "threading/thread_proxy.hpp"
#include "threading/timer/deadline_timer.h"
#include "threading/timer/steady_timer.h"
#include "threading/timer_proxy.h"

threading::ExecutorPtr g_workers;

int main()
{
    g_workers = threading::ThreadProxy::createAsioExecutor("workers", 6);
    threading::TimerProxy::start();
    /* 定时器1 */
    int count1 = 0;
    auto tm1 = std::make_shared<threading::SteadyTimer>(std::chrono::seconds(0), std::chrono::milliseconds(3000), "", [&count1]() {
        ++count1;
        printf("===== SteadyTimer[1] === %d\n", count1);
    });
    tm1->start();
    /* 定时器2 */
    int count2 = 0;
    auto tm2 = std::make_shared<threading::SteadyTimer>(std::chrono::seconds(0), std::chrono::milliseconds(2000), "", [&count2]() {
        ++count2;
        printf("===== SteadyTimer[2] === %d\n", count2);
    });
    tm2->start();
    /* 定时器3 */
    auto tm3 = std::make_shared<threading::DeadlineTimer>(std::chrono::system_clock::now() + std::chrono::seconds(10), "",
                                                          [&]() { printf("========== DeadlineTimer over\n"); });
    tm3->start();
    /* 主循环 */
    int64_t index = 0;
    while (1)
    {
        if (index > 9999999999999999)
        {
            index = 0;
        }
        /* 抛任务到工作线程执行 */
        threading::ThreadProxy::async(
            "task_" + std::to_string(++index),
            [index]() {
                printf("----- [%lld]\n", index);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            },
            g_workers);
        /* 监听定时器回调 */
        threading::TimerProxy::runOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
