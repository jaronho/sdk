#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <string.h>

#include "../usb/usb.h"
#ifndef _WIN32
#include <libudev.h>
#endif

int runCommand(const std::string& cmd, std::string* outStr = nullptr, std::vector<std::string>* outVec = nullptr)
{
    if (outStr)
    {
        (*outStr).clear();
    }
    if (outVec)
    {
        (*outVec).clear();
    }
    if (cmd.empty())
    {
        return -1;
    }
    FILE* stream = NULL;
#ifdef _WIN32
    stream = _popen(cmd.c_str(), "r");
#else
    stream = popen(cmd.c_str(), "r");
#endif
    if (!stream)
    {
        return -2;
    }
    if (outStr || outVec)
    {
        const size_t bufferSize = 1024;
        char buffer[bufferSize] = {0};
        std::string line;
        while (fread(buffer, 1, bufferSize, stream) > 0)
        {
            if (outStr)
            {
                (*outStr).append(buffer);
            }
            if (outVec)
            {
                line += buffer;
                while (1)
                {
                    size_t pos = line.find("\r\n"), offset = 2;
                    if (std::string::npos == pos)
                    {
                        pos = line.find("\n"), offset = 1;
                    }
                    if (std::string::npos == pos)
                    {
                        break;
                    }
                    (*outVec).emplace_back(line.substr(0, pos));
                    line = line.substr(pos + offset, line.size() - pos - offset);
                }
            }
            memset(buffer, 0, bufferSize);
        }
        if (outVec && !line.empty())
        {
            (*outVec).emplace_back(line);
        }
    }
#ifdef _WIN32
    return _pclose(stream);
#else
    int ret = pclose(stream);
    if (0 != ret && 10 == errno) /* 当进程某处设置了`signal(SIGCHLD, SIG_IGN)`时, 会出现"No child processes", 这里就设置不认为出错 */
    {
        ret = 0;
    }
    return ret;
#endif
}

bool stringContains(std::string str, std::string pattern, bool caseSensitive = true)
{
    if (pattern.empty())
    {
        return true;
    }
    if (pattern.size() > str.size())
    {
        return false;
    }
    if (!caseSensitive)
    {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        std::transform(pattern.begin(), pattern.end(), pattern.begin(), tolower);
    }
    if (std::string::npos == str.find(pattern))
    {
        return false;
    }
    return true;
}

#ifndef _WIN32
std::vector<std::string> getUsbDevNodes(int busNum, int portNum, int address)
{
    std::vector<std::string> devNodes;
    struct udev* udev = udev_new();
    if (udev)
    {
        udev_enumerate* enumerate = udev_enumerate_new(udev);
        if (enumerate)
        {
            udev_enumerate_add_match_is_initialized(enumerate); /* 只查找已经初始化的设备 */
            udev_enumerate_add_match_subsystem(enumerate, "block");
            udev_enumerate_add_match_subsystem(enumerate, "hidraw");
            udev_enumerate_scan_devices(enumerate);
            struct udev_list_entry* devEntryList = udev_enumerate_get_list_entry(enumerate);
            if (devEntryList)
            {
                struct udev_list_entry* devEntry;
                udev_list_entry_foreach(devEntry, devEntryList) /* 遍历设备 */
                {
                    const char* entryName = udev_list_entry_get_name(devEntry);
                    if (!entryName)
                    {
                        continue;
                    }
                    struct udev_device* dev = udev_device_new_from_syspath(udev, entryName);
                    if (!dev)
                    {
                        continue;
                    }
                    const char* devNode = udev_device_get_devnode(dev);
                    if (devNode)
                    {
                        struct udev_device* pDev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
                        if (pDev)
                        {
                            const char* busNumPtr = udev_device_get_property_value(pDev, "BUSNUM");
                            const char* portNumPtr = udev_device_get_sysnum(pDev);
                            const char* devNumPtr = udev_device_get_property_value(pDev, "DEVNUM");
                            /* 找到指定插口的USB设备 */
                            if ((busNumPtr && portNumPtr && devNumPtr)
                                && (std::atoi(busNumPtr) == busNum && std::atoi(portNumPtr) == portNum && std::atoi(devNumPtr) == address))
                            {
                                /* 如果是存储设备, 则需要判断节点是否为可挂载的分区 */
                                const char* subSystemPtr = udev_device_get_subsystem(dev);
                                if (subSystemPtr && 0 == strcmp(subSystemPtr, "block"))
                                {
                                    std::string outStr;
                                    runCommand(std::string("blkid ") + devNode, &outStr);
                                    /* 例如: /dev/sdb /dev/sdb1 几乎只有 /dev/sdb1 可挂载 */
                                    if (!stringContains(outStr, "PTUUID", false) && !stringContains(outStr, "PTTYPE", false)
                                        && stringContains(outStr, "UUID", false))
                                    {
                                        devNodes.emplace_back(devNode);
                                    }
                                }
                                else
                                {
                                    devNodes.emplace_back(devNode);
                                }
                            }
                        }
                    }
                    udev_device_unref(dev);
                }
            }
            udev_enumerate_unref(enumerate);
        }
        udev_unref(udev);
    }
    return devNodes;
}
#endif

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
    auto usbList = usb::Usb::getAllUsbs(true, true, true);
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
            std::vector<std::string> devNodes;
#ifndef _WIN32
            if (info.isHid() || info.isStorage()) /* 只需获取HID和存储类型的设备节点 */
            {
                devNodes = getUsbDevNodes(info.getBusNum(), info.getPortNum(), info.getAddress());
            }
#endif
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
                line += "\"speedDesc\":\"" + info.getSpeedDesc() + "\"";
                line += ",";
                line += "\"vid\":\"" + info.getVid() + "\"";
                line += ",";
                line += "\"pid\":\"" + info.getPid() + "\"";
                line += ",";
                line += "\"serial\":\"" + info.getSerial() + "\"";
                line += ",";
                {
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
