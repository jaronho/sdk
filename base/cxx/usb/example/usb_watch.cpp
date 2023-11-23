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
 *       -p 是否显示路径,, 会显示设备路径信息, 格式: 总线_1级接口.2级接口.N级接口
 *       -d 是否显示设备节点
 *       -t 是否树形显示
 * 例如: ./usb_watch -p -d -t
 */
int main(int argc, char** argv)
{
    /* 参数值 */
    bool pathFlag = false;
    bool devFlag = false;
    bool treeFlag = false;
    /* 解析参数 */
    for (int i = 1; i < argc;)
    {
        std::string key = argv[i];
        if (0 == key.compare("-h")) /* 帮助 */
        {
            printf("USB设备查看工具\n");
            printf("用法: ./usb_watch [选项]\n");
            printf("选项: -h    帮助信息\n");
            printf("选项: -p    是否显示路径, 格式: 总线_1级接口.2级接口.N级接口\n");
#ifndef _WIN32
            printf("      -d    是否显示设备节点信息\n");
#endif
            printf("      -t    是否树形显示\n");
            return 0;
        }
        else if (0 == key.compare("-p")) /* 路径 */
        {
            pathFlag = true;
            i += 1;
            continue;
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
                usbListJson += info->describe(pathFlag, devFlag, true, 0, 4);
            }
        }
        else
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
            usbListJson += info->describe(pathFlag, devFlag, false, 0, 4);
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
