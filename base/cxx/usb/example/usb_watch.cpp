#include <stdio.h>
#include <string.h>

#include "../usb/usb.h"

/**
 * USB设备查看工具
 * 用法: ./usb_watch [选项]
 * 选项: -h 帮助信息
 *       -t 是否树形显示
 * 例如: ./usb_watch -t
 */
int main(int argc, char** argv)
{
    /* 参数值 */
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
            printf("      -t    是否树形显示\n");
            return 0;
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
                usbListJson += info->describe(true, 0, 4);
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
            usbListJson += info->describe(false, 0, 4);
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
