#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

#include "../serial/serial.h"

serial::Serial g_com;
std::mutex g_mutex;
std::chrono::steady_clock::time_point g_timestamp;
std::string g_buffer;

void threadHandler()
{
    char buf[2] = {0};
    size_t len = 0;
    while (1)
    {
        while ((len = g_com.read(buf, sizeof(buf) - 1)) > 0)
        {
            printf("%s", buf);
            std::lock_guard<std::mutex> locker(g_mutex);
            g_timestamp = std::chrono::steady_clock::now();
            g_buffer.append(buf);
        }
    }
}

int main()
{
    auto portList = serial::getAllPorts();
    printf("=============== all serial:\n");
    for (auto iter = portList.begin(); portList.end() != iter; ++iter)
    {
        printf("%s, %s, %s\n", iter->port.c_str(), iter->description.c_str(), iter->hardwareId.c_str());
    }

    g_com.setPort("/dev/ttyUSB6");
    g_com.setBaudrate(38400);
    g_com.setDatabits(serial::Databits::SEVEN);
    g_com.setParity(serial::ParityType::EVEN);
    g_com.setStopbits(serial::Stopbits::TWO);
    g_com.setFlowcontrol(serial::FlowcontrolType::NONE);
    g_com.setTimeout(serial::Timeout::simpleTimeout(1000));
    bool ret = g_com.open();
    if (!ret)
    {
        printf("open serial fail\n");
        return 0;
    }
    printf("open serial ok\n");

    std::thread th(threadHandler);
    th.detach();

    while (1)
    {
        auto nowTimestamp = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> locker(g_mutex);
            std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(nowTimestamp - g_timestamp);
            if (elapsed.count() >= 1000 && !g_buffer.empty())
            {
                printf("\n======= 收到串口数据长度: %zu\n", g_buffer.size());
                g_buffer.clear();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
