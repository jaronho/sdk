#include <chrono>
#include <iostream>
#include <thread>

#include "../serial/serial.h"

int main()
{
    auto portList = serial::getAllPorts();
    printf("=============== all serial:\n");
    for (auto iter = portList.begin(); portList.end() != iter; ++iter)
    {
        printf("%s, %s, %s\n", iter->port.c_str(), iter->description.c_str(), iter->hardwareId.c_str());
    }
    serial::Serial com;
    com.setPort("/dev/ttyUSB6");
    com.setBaudrate(38400);
    com.setDatabits(serial::Databits::SEVEN);
    com.setParity(serial::ParityType::EVEN);
    com.setStopbits(serial::Stopbits::TWO);
    com.setFlowcontrol(serial::FlowcontrolType::NONE);
    com.setTimeout(serial::Timeout::simpleTimeout(1000));
    bool ret = com.open();
    if (!ret)
    {
        printf("open serial fail\n");
        return 0;
    }
    printf("open serial ok\n");
    char buf[2] = {0};
    size_t len = 0;
    while (1)
    {
        while ((len = com.read(buf, sizeof(buf) - 1)) > 0)
        {
            printf("%s", buf);
            //com.write(buf, sizeof(buf) - 1);
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
