#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "pcap_device.h"

int main(int argc, char* argv[])
{
    std::string name;
    std::string ip;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-name")) /* 本地设备名称 */
        {
            ++i;
            if (i < argc)
            {
                name = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-ip")) /* 本地设备IP */
        {
            ++i;
            if (i < argc)
            {
                ip = argv[i];
                ++i;
            }
        }
    }
    if (name.empty() && ip.empty())
    {
        printf("device name and ip is empty\n");
        return 0;
    }
    printf("device name: %s, ip: %s\n", name.c_str(), ip.c_str());
    std::shared_ptr<PcapDevice> dev;
    auto devList = PcapDevice::getAllDevices();
    for (size_t i = 0; i < devList.size(); ++i)
    {
        if (0 == name.compare(devList[i]->getName()) || 0 == ip.compare(devList[i]->getIpv4Address()))
        {
            dev = devList[i];
            break;
        }
    }
    if (!dev || !dev->open())
    {
        printf("device not found\n");
        return 0;
    }
    printf("device found, name: %s, ipv4: %s, describe: %s\n", dev->getName().c_str(), dev->getIpv4Address().c_str(),
           dev->getDescribe().c_str());
    printf("start caputre ...\n");
    printf("\n");
    dev->setDataCallback([&](const unsigned char* data, int dataLen) {
        printf("----- on data: %d\n", dataLen);
        /* Ethernet */
        if (dataLen < 14)
        {
            return;
        }
        /* IP */
        /* TCP/UDP */
    });
    dev->startCapture();
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    dev->close();
    return 0;
}
