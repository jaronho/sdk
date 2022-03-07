#include <chrono>
#include <stdio.h>
#include <string.h>

#include "../usb/usb_info.h"

/**
 * USB设备查看工具
 * 用法: ./usb_watch [选项]
 * 选项: -b busNum      总线编号, 指定要查找哪个总线上的设备, 例如: 1
 *       -p portNum     端口编号, 指定要查找哪个端口上的设备, 例如: 2
 * 例如: ./usb_watch -b 1 -p 2
 */
int main(int argc, char** argv)
{
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
    bool firstLine = true;
    std::string usbListJson;
    usbListJson += "[";
    auto usbList = usb::UsbInfo::queryUsbInfos(nullptr, true, true);
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
            std::string line;
            line += "{";
            {
                line += "\"busNum\":" + std::to_string(info.getBusNum());
                line += ",";
                line += "\"portNum\":" + std::to_string(info.getPortNum());
                line += ",";
                line += "\"address\":" + std::to_string(info.getAddress());
                line += ",";
                line += "\"classCode\":" + std::to_string(info.getClassCode());
                line += ",";
                line += "\"classDesc\":\"" + info.getClassDesc() + "\"";
                line += ",";
                line += "\"speedLevel\":" + std::to_string(info.getSpeedLevel());
                line += ",";
                line += "\"speedDesc\":\"" + info.getSpeedDesc() + "\"";
                line += ",";
                line += "\"vid\":\"" + info.getVid() + "\"";
                line += ",";
                line += "\"pid\":\"" + info.getPid() + "\"";
                line += ",";
                line += "\"serial\":\"" + info.getSerial() + "\"";
                line += ",";
                line += "\"product\":\"" + info.getProduct() + "\"";
                line += ",";
                line += "\"manufacturer\":\"" + info.getManufacturer() + "\"";
#ifndef _WIN32
                line += ",";
                {
                    auto devNodes = info.getDevNodes();
                    line += "\"devNodes\":";
                    line += "[";
                    for (size_t i = 0; i < devNodes.size(); ++i)
                    {
                        if (i > 0)
                        {
                            line += ",";
                        }
                        line += "\"" + devNodes[i] + "\"";
                    }
                    line += "]";
                }
#endif
            }
            line += "}";
            if (firstLine)
            {
                firstLine = false;
            }
            else
            {
                usbListJson += ",";
            }
            usbListJson += "\n";
            usbListJson += line;
        }
    }
    if (!firstLine)
    {
        usbListJson += "\n";
    }
    usbListJson += "]";
    printf("%s", usbListJson.c_str());
    return 0;
}
