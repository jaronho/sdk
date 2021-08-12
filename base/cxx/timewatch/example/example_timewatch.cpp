#include <iostream>
#include <thread>

#include "timewatch/timeout_monitor.h"

/* 耗时检测器观察 */
void onWatcherWatch(const std::string& tag, const std::string& subTag, long long elapsed)
{
    std::cout << "watcher[watch]: [" << tag << "][" << subTag << "] " << elapsed << " ms" << std::endl;
}

/* 耗时检测器结束 */
void onWatcherEnd(const std::string& tag, long long elapsed)
{
    std::cout << "watcher[finish]: [" << tag << "] " << elapsed << " ms" << std::endl;
}

/* 超时监控器观察 */
void onMonitorCapture(const std::string& tag, const std::string& subTag, long long elapsed)
{
    std::cout << "monitor[watch]: [" << tag << "][" << subTag << "] " << elapsed << " ms" << std::endl;
}

/* 超时监控器结束 */
void onMonitorEnd(const std::string& tag, long long elapsed)
{
    std::cout << "monitor[finish]: [" << tag << "] " << elapsed << " ms" << std::endl;
}

/* 测试耗时检测器: 普通宏 */
void testTimeWather1()
{
    MAKE_TIME_WATCHER(watcher, onWatcherWatch, onWatcherEnd, "testTimeWather-1");
    for (int i = 0; i <= 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(i * 100));
        TIME_WATCHER_WATCH(watcher, std::to_string(i));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/* 测试耗时检测器: 简化后的宏 */
void testTimeWather2()
{
    START_SIMPLE_TIME_WATCHER([](const std::string& tag, long long elapsed) {
        std::cout << "watcher[finish]: [testTimeWather-1] " << elapsed << " ms" << std::endl;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/* 测试超时监控器: 普通宏 */
void testTimeoutMonitor1()
{
    MAKE_TIMEOUT_MONITOR(monitor, onMonitorCapture, onMonitorEnd, 1000, "testTimeoutMonitor-1");
    for (int i = 0; i <= 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(i * 100));
        TIMEOUT_MONITOR_CAPTURE(monitor, 50, std::to_string(i));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/* 测试超时监控器: 简化后的宏, 简易的宏 */
void testTimeoutMonitor2()
{
    START_SIMPLE_TIMEOUT_MONITOR(
        [](const std::string& tag, long long elapsed) {
            std::cout << "monitor[finish]: [testTimeoutMonitor-2] " << elapsed << " ms" << std::endl;
        },
        100);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main()
{
    testTimeWather1();
    std::cout << std::endl;
    testTimeWather2();
    std::cout << std::endl << std::endl;
    testTimeoutMonitor1();
    std::cout << std::endl;
    testTimeoutMonitor2();
    return 0;
}
