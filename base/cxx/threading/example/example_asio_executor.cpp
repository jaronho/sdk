#include <iostream>

#include "threading/platform.h"
#include "threading/signal/async_signal.h"
#include "threading/signal/basic_signal.h"
#include "threading/thread_proxy.hpp"
#include "threading/timer/deadline_timer.h"
#include "threading/timer/steady_timer.h"
#include "threading/timer_proxy.h"

threading::ExecutorPtr g_workers;

int main()
{
    int mainPid = threading::Platform::getThreadId();
    g_workers = threading::ThreadProxy::createAsioExecutor("workers", 6);
    threading::TimerProxy::start();
    /* 定时器1 */
    int count1 = 0;
    auto tm1 = std::make_shared<threading::SteadyTimer>(std::chrono::seconds(0), std::chrono::milliseconds(5000), "", [mainPid, &count1]() {
        ++count1;
        printf("===== [%d:%d] SteadyTimer === %d\n", mainPid, threading::Platform::getThreadId(), count1);
    });
    tm1->start();
    /* 定时器3 */
    auto tm2 = std::make_shared<threading::DeadlineTimer>(std::chrono::system_clock::now() + std::chrono::seconds(10), "",
                                                          [&]() { printf("========== DeadlineTimer over\n"); });
    tm2->start();
    /* 同步信号(不带参数无返回值) */
    threading::BasicSignal<void()> sig1;
    sig1.connect(
        [mainPid]() { printf("----- [%d:%d] BasicSignal without args and not return\n", mainPid, threading::Platform::getThreadId()); });
    sig1();
    /* 同步信号(带参数无返回值) */
    threading::BasicSignal<void(int x)> sig2;
    sig2.connect([mainPid](int x) {
        printf("----- [%d:%d] BasicSignal with args[%d] and not return\n", mainPid, threading::Platform::getThreadId(), x);
    });
    sig2(5);
    /* 同步信号(带参数有返回值) */
    threading::BasicSignal<int(const std::string& str)> sig3;
    sig3.connect([mainPid](const std::string& str) -> int {
        int ret = 1;
        printf("----- [%d:%d] BasicSignal with args[%s] and return[%d]\n", mainPid, threading::Platform::getThreadId(), str.c_str(), ret);
        return ret;
    });
    sig3.connect([mainPid](const std::string& str) -> int {
        int ret = 2;
        printf("----- [%d:%d] BasicSignal with args[%s] and return[%d]\n", mainPid, threading::Platform::getThreadId(), str.c_str(), ret);
        return ret;
    });
    auto results = sig3("hello");
    for (size_t i = 0; i < results.size(); ++i)
    {
        printf("----- [%d:%d] BasicSignal result[%d] = %d\n", mainPid, threading::Platform::getThreadId(), ((int)i + 1), results[i]);
    }
    /* 异步信号 */
    auto executor = std::make_shared<threading::AsioExecutor>("async_signal");
    threading::AsyncSignal<void(int x)> sig4;
    sig4.connect([mainPid](int x) { printf("----- [%d:%d] AsyncSignal with args[%d]\n", mainPid, threading::Platform::getThreadId(), x); },
                 executor);
    sig4(6);
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
            [mainPid, index]() {
                printf("----- [%d:%d] [%lld]\n", mainPid, threading::Platform::getThreadId(), index);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            },
            g_workers);
        /* 监听定时器回调 */
        threading::TimerProxy::runOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
