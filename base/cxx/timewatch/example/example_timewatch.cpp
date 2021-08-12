#include <iostream>
#include <thread>

#include "timewatch/timeout_monitor.h"

/* ºÄÊ±¼ì²âÆ÷¹Û²ì */
void onWatcherWatch(const std::string& tag, const std::string& subTag, long long elapsed)
{
    std::cout << "watcher[watch]: [" << tag << "][" << subTag << "] " << elapsed << " ms" << std::endl;
}

/* ºÄÊ±¼ì²âÆ÷½áÊø */
void onWatcherEnd(const std::string& tag, long long elapsed)
{
    std::cout << "watcher[finish]: [" << tag << "] " << elapsed << " ms" << std::endl;
}

/* ³¬Ê±¼à¿ØÆ÷¹Û²ì */
void onMonitorCapture(const std::string& tag, const std::string& subTag, long long elapsed)
{
    std::cout << "monitor[watch]: [" << tag << "][" << subTag << "] " << elapsed << " ms" << std::endl;
}

/* ³¬Ê±¼à¿ØÆ÷½áÊø */
void onMonitorEnd(const std::string& tag, long long elapsed)
{
    std::cout << "monitor[finish]: [" << tag << "] " << elapsed << " ms" << std::endl;
}

/* ²âÊÔºÄÊ±¼ì²âÆ÷: ÆÕÍ¨ºê */
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

/* ²âÊÔºÄÊ±¼ì²âÆ÷: ¼ò»¯ºóµÄºê */
void testTimeWather2()
{
    START_SIMPLE_TIME_WATCHER(onWatcherEnd, "testTimeWather-2");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/* ²âÊÔ³¬Ê±¼à¿ØÆ÷: ÆÕÍ¨ºê */
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

/* ²âÊÔ³¬Ê±¼à¿ØÆ÷: ¼ò»¯ºóµÄºê, ¼òÒ×µÄºê */
void testTimeoutMonitor2()
{
    START_SIMPLE_TIMEOUT_MONITOR(onMonitorEnd, 100, "testTimeoutMonitor-2");
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
