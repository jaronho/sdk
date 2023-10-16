#include <chrono>
#include <stdio.h>
#include <string.h>

#include "../usb/usb.h"

int main(int argc, char** argv)
{
    printf("***********************************************************************************************************\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-b busNum]            specify the bus number of USB, e.g. 1.                                         **\n");
    printf("** [-p portNum]           specify the port number of USB, e.g. 2.                                        **\n");
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    /* 参数值 */
    int busNum = -1;
    int portNum = -1;
    /* 解析参数 */
    for (int i = 1; i < argc;)
    {
        std::string key = argv[i];
        if (i + 1 >= argc)
        {
            break;
        }
        const char* val = argv[i + 1];
        if (0 == key.compare("-b")) /* 总线编号 */
        {
            busNum = std::atoi(val);
        }
        else if (0 == key.compare("-p")) /* 端口编号 */
        {
            portNum = std::atoi(val);
        }
        i += 2;
    }
    auto usbList = usb::Usb::getAllUsbs(true);
    for (size_t i = 0; i < usbList.size(); ++i)
    {
        const auto& info = usbList[i];
        bool busFlag = true;
        if (busNum >= 0)
        {
            busFlag = (info.getBusNum() == busNum);
        }
        bool portFlag = true;
        if (portNum >= 0)
        {
            portFlag = (info.getPortNum() == portNum);
        }
        if (busFlag && portFlag)
        {
            printf("============================================================ [%02d]\n", ((int)i + 1));
            printf("=       busNum: %d\n", info.getBusNum());
            printf("=      portNum: %d\n", info.getPortNum());
            printf("=      address: %d\n", info.getAddress());
            printf("=        class: %d\n", info.getClassCode());
            printf("=    classDesc: %s\n", info.getClassDesc().c_str());
            printf("=     subClass: %d\n", info.getSubClassCode());
            printf("=     protocol: %d\n", info.getProtocolCode());
            printf("=        speed: %s\n", info.getSpeedDesc().c_str());
            printf("=          vid: %s\n", info.getVid().c_str());
            printf("=          pid: %s\n", info.getPid().c_str());
            printf("=       serial: %s\n", info.getSerial().c_str());
            printf("=      product: %s\n", info.getProduct().c_str());
            printf("= manufacturer: %s\n", info.getManufacturer().c_str());
#ifdef _WIN32
            printf("=       vendor: %s\n", info.getVendor().c_str());
            printf("=        model: %s\n", info.getModel().c_str());
            if (info.isStorage())
            {
                printf("=  storageType: %s\n", info.getStorageType().c_str());
            }
#endif
        }
    }
    printf("\n");
    return 0;
}
