#include <iostream>

#include "threading/async_proxy.h"
#include "threading/platform.h"
#include "threading/safe_queue.h"
#include "threading/signal/async_signal.h"
#include "threading/signal/basic_signal.h"
#include "threading/signal/scoped_signal_connection.h"
#include "threading/thread_proxy.hpp"
#include "threading/timer/deadline_timer.h"
#include "threading/timer/steady_timer.h"

/**
 * @brief 逻辑消息
 */
struct LogicMsg
{
    LogicMsg() = default;
    LogicMsg(const std::string& name, const std::function<void()>& handler) : name(name), handler(handler) {}

    std::string name;
    std::function<void()> handler = nullptr;
};

threading::ExecutorPtr s_logicExecutor; /* 逻辑线程 */
threading::SafeQueue<LogicMsg> s_logicMsgQueue; /* 逻辑消息队列 */
threading::ExecutorPtr g_workers; /* 工作线程 */
threading::BasicSignal<void()> g_sig1; /* 同步信号(不带参数无返回值) */
threading::BasicSignal<void(const std::string& str)> g_sig2; /* 同步信号(带参数无返回值) */
threading::BasicSignal<int(int a, int b)> g_sig3; /* 同步信号(带参数有返回值) */
threading::AsyncSignal<void(int num)> g_sig4; /* 异步信号(带有参数) */

/**
 * @brief 尝试处理逻辑消息
 */
void tryHandleLogicMsg()
{
    LogicMsg msg;
    while (s_logicMsgQueue.tryPop(msg))
    {
        if (msg.handler)
        {
            msg.handler();
        }
    }
}

/* 连接信号 */
void connectSignal()
{
    int mainPid = threading::Platform::getThreadId();

    g_sig1.connect([mainPid]() { printf("----- [%d:%d] BasicSignal\n", mainPid, threading::Platform::getThreadId()); });

    g_sig2.connect([mainPid](const std::string& str) {
        printf("----- [%d:%d] BasicSignal with args[%s]\n", mainPid, threading::Platform::getThreadId(), str.c_str());
    });

    g_sig3.connect([mainPid](int a, int b) -> int {
        int c = a + b;
        printf("----- [%d:%d] BasicSignal with args[%d, %d] and return[%d]\n", mainPid, threading::Platform::getThreadId(), a, b, c);
        return c;
    });
    g_sig3.connect([mainPid](int a, int b) -> int {
        int c = 2 * (a + b);
        printf("----- [%d:%d] BasicSignal with args[%d, %d] and return[%d]\n", mainPid, threading::Platform::getThreadId(), a, b, c);
        return c;
    });

    g_sig4.connect(
        [mainPid](int x) { printf("----- [%d:%d] AsyncSignal with args[%d]\n", mainPid, threading::Platform::getThreadId(), x); },
        g_workers);
}

int main()
{
    int mainPid = threading::Platform::getThreadId();
    s_logicExecutor = threading::ThreadProxy::createAsioExecutor("logic", 1);
    threading::AsyncProxy::start(6, s_logicExecutor, [&](const std::string& name, const std::function<void()>& finishCb) {
        s_logicMsgQueue.push(LogicMsg(name, finishCb));
    }); /* 创建异步任务线程(6个线程) */
    threading::Timer::setDefaultExecutor(
        s_logicExecutor, [&](const std::string& name, const std::function<void()>& func) { s_logicMsgQueue.push(LogicMsg(name, func)); });
    g_workers = threading::ThreadProxy::createAsioExecutor("workers", 6); /* 创建工作线程(6个线程) */
    /* 定时器1 */
    int count1 = 0;
    auto tm1 = std::make_shared<threading::SteadyTimer>("", std::chrono::seconds(0), std::chrono::milliseconds(5000), [mainPid, &count1]() {
        ++count1;
        printf("===== [%d:%d] SteadyTimer === %d\n", mainPid, threading::Platform::getThreadId(), count1);
    });
    tm1->start();
    /* 定时器2 */
    auto tm2 = std::make_shared<threading::DeadlineTimer>("", std::chrono::system_clock::now() + std::chrono::seconds(10),
                                                          [&]() { printf("========== DeadlineTimer over\n"); });
    tm2->start();
    /* 信号 */
    connectSignal();
    g_sig1();
    g_sig2("hello");
    auto results = g_sig3(1, 2);
    for (size_t i = 0; i < results.size(); ++i)
    {
        printf("----- [%d:%d] BasicSignal result[%d] = %d\n", mainPid, threading::Platform::getThreadId(), ((int)i + 1), results[i]);
    }
    g_sig4(0);
    /* 主循环 */
    int64_t index = 0;
    while (1)
    {
        if (index > 9999999999999999)
        {
            index = 0;
        }
        g_sig4(index);
        /* 抛任务到工作线程执行 */
        threading::ThreadProxy::async(
            THREADING_CALLER + "_" + std::to_string(++index),
            [mainPid, index]() {
                printf("----- [%d:%d] [%lld]\n", mainPid, threading::Platform::getThreadId(), index);
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            },
            g_workers);
        /* 抛任务到异步任务线程执行 */
        threading::AsyncProxy::execute(
            THREADING_CALLER,
            [&]() {
                printf("----- [%d:%d] async func\n", mainPid, threading::Platform::getThreadId());
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            },
            [&]() { printf("----- [%d:%d] async func finished\n", mainPid, threading::Platform::getThreadId()); });
        /* 主线程处理逻辑消息 */
        tryHandleLogicMsg();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
