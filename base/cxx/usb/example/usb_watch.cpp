#include <chrono>
#include <stdio.h>
#include <string.h>

#include "../usb/usb.h"

bool isOptionName(const std::string& str)
{
    if ("-t" == str || "-b" == str || "-p" == str)
    {
        return true;
    }
    return false;
}

/**
 * USB设备查看工具
 * 用法: ./usb_watch [选项]
 * 选项: -h 帮助信息
 *       -a 是否显示所有, 当设置该选项时, 会显示设备节点信息
 *       -t 是否树形显示, 当设置该选项时, -b, -p选项无效
 *       -b busNum 总线编号, 指定要查找哪个总线上的设备, 例如: 1
 *       -p portNum 端口编号, 指定要查找哪个端口上的设备, 例如: 2
 * 例如: ./usb_watch -b 1 -p 2
 */
int main(int argc, char** argv)
{
    /* 参数值 */
    bool devFlag = false;
    bool treeFlag = false;
    int busNum = -1;
    int portNum = -1;
    /* 解析参数 */
    for (int i = 1; i < argc;)
    {
        std::string key = argv[i];
        if (0 == key.compare("-h")) /* 帮助 */
        {
            printf("USB设备查看工具\n");
            printf("用法: ./usb_watch [选项]\n");
            printf("选项: -h             帮助信息\n");
#ifndef _WIN32
            printf("      -d             是否显示设备节点信息, 当设置该选项时, 会显示设备节点信息\n");
#endif
            printf("      -t             是否树形显示, 当设置该选项时, -b, -p选项无效\n");
            printf("      -b 总线编号    指定要查找哪个总线上的设备, 例如: 1\n");
            printf("      -p 端口编号    指定要查找哪个端口上的设备, 例如: 2\n");
            return 0;
        }
        else if (0 == key.compare("-d")) /* 设备节点 */
        {
            devFlag = true;
            i += 1;
            continue;
        }
        else if (0 == key.compare("-t")) /* 树形 */
        {
            treeFlag = true;
            i += 1;
            continue;
        }
        if (i + 1 >= argc)
        {
            break;
        }
        const char* val = argv[i + 1];
        if (isOptionName(val))
        {
            i += 1;
            continue;
        }
        if (0 == key.compare("-b")) /* 总线编号 */
        {
            try
            {
                busNum = std::atoi(val);
            }
            catch (...)
            {
            }
        }
        else if (0 == key.compare("-p")) /* 端口编号 */
        {
            try
            {
                portNum = std::atoi(val);
            }
            catch (...)
            {
            }
        }
        i += 2;
    }
    bool firstLine = true;
    std::string usbListJson;
    usbListJson += "[";
    auto usbList = usb::Usb::getAllUsbs(true);
    for (size_t i = 0; i < usbList.size(); ++i)
    {
        const auto& info = usbList[i];
        if (treeFlag)
        {
            if (!info->getParent())
            {
                if (firstLine)
                {
                    firstLine = false;
                }
                else
                {
                    usbListJson += ",";
                }
                usbListJson += "\n";
                usbListJson += info->describe(devFlag, true, 0, 4);
            }
        }
        else
        {
            bool busFlag = true;
            if (busNum >= 0)
            {
                busFlag = (info->getBusNum() == busNum);
            }
            bool portFlag = true;
            if (portNum >= 0)
            {
                portFlag = (info->getPortNum() == portNum);
            }
            if (busFlag && portFlag)
            {
                if (firstLine)
                {
                    firstLine = false;
                }
                else
                {
                    usbListJson += ",";
                }
                usbListJson += "\n";
                usbListJson += info->describe(devFlag, false, 0, 4);
            }
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
