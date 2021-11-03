#include <iostream>
#include <thread>

#include "pcap_device.h"

int main(int argc, char* argv[])
{
    std::string name = "";
    std::string ip = "192.168.31.82";
    std::shared_ptr<PcapDevice> dev;
    auto devList = PcapDevice::getAllDevices();
    for (size_t i = 0; i < devList.size(); ++i)
    {
        if (0 == name.compare(devList[i]->getName()) || 0 == ip.compare(devList[i]->getAddress()))
        {
            dev = devList[i];
            break;
        }
    }
    if (!dev || !dev->open())
    {
        return 0;
    }
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
