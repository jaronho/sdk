#include "usb_info.h"

#include <algorithm>
#include <map>
#include <string.h>
#ifndef _WIN32
#include <libudev.h>
#endif

namespace usb
{
static std::string runCommand(const std::string& cmd)
{
    std::string outStr;
    FILE* stream = NULL;
#ifdef _WIN32
    stream = _popen(cmd.c_str(), "r");
#else
    stream = popen(cmd.c_str(), "r");
#endif
    if (stream)
    {
        const size_t bufferSize = 1024;
        char buffer[bufferSize] = {0};
        while (fread(buffer, 1, bufferSize, stream) > 0)
        {
            outStr.append(buffer);
            memset(buffer, 0, bufferSize);
        }
#ifdef _WIN32
        _pclose(stream);
#else
        pclose(stream);
#endif
    }
    return outStr;
}

#ifndef _WIN32
/**
 * @brief 查询USB设备节点列表
 * @param busNum 总线
 * @param portNum 端口
 * @param address 地址
 * @return USB设备节点列表 
 */
static std::map<std::string, std::string> queryUsbDevNodes(int busNum, int portNum, int address)
{
    std::map<std::string, std::string> devNodes;
    struct udev* udev = udev_new();
    if (!udev)
    {
        return devNodes;
    }
    udev_enumerate* enumerate = udev_enumerate_new(udev);
    if (!enumerate)
    {
        udev_unref(udev);
        return devNodes;
    }
    udev_enumerate_add_match_is_initialized(enumerate); /* 只查找已经初始化的设备 */
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry* devEntryList = udev_enumerate_get_list_entry(enumerate);
    if (!devEntryList)
    {
        udev_enumerate_unref(enumerate);
        udev_unref(udev);
        return devNodes;
    }
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
                        auto outStr = runCommand(std::string("blkid ") + devNode);
                        std::transform(outStr.begin(), outStr.end(), outStr.begin(), toupper);
                        /* 例如: /dev/sdb /dev/sdb1 几乎只有 /dev/sdb1 可挂载 */
                        static const std::string UUID_FLAG = " UUID=\"";
                        static const std::string TYPE_FLAG = " TYPE=\"";
                        auto typePos = outStr.find(TYPE_FLAG);
                        if (std::string::npos != outStr.find(UUID_FLAG) && std::string::npos != typePos)
                        {
                            std::string fstype;
                            for (size_t i = typePos + TYPE_FLAG.size(); i < outStr.size(); ++i)
                            {
                                if ('"' == outStr[i])
                                {
                                    break;
                                }
                                fstype.push_back(outStr[i]);
                            }
                            std::transform(fstype.begin(), fstype.end(), fstype.begin(), tolower);
                            devNodes.insert(std::make_pair(devNode, fstype));
                        }
                    }
                    else
                    {
                        devNodes.insert(std::make_pair(devNode, ""));
                    }
                }
            }
        }
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return devNodes;
}
#endif

UsbInfo::UsbInfo(const Usb& other) : Usb(other) {}

UsbInfo::UsbInfo(const UsbInfo& other, const std::vector<DevNode>& devNodes) : Usb(other), m_devNodes(devNodes) {}

bool UsbInfo::operator==(const UsbInfo& other) const
{
    if (getVid() == other.getVid() && getPid() == other.getPid() && getSerial() == other.getSerial())
    {
        return true;
    }
    return false;
}

bool UsbInfo::operator!=(const UsbInfo& other) const
{
    if (getVid() == other.getVid() && getPid() == other.getPid() && getSerial() == other.getSerial())
    {
        return false;
    }
    return true;
}

std::vector<UsbInfo::DevNode> UsbInfo::getDevNodes() const
{
    return m_devNodes;
}

bool UsbInfo::isValid() const
{
    return (getBusNum() < 0 || getPortNum() < 0 || getAddress() < 0 || getVid().empty() || getPid().empty());
}

std::string UsbInfo::describe() const
{
    std::string desc;
    desc.append("busNum: ").append(std::to_string(getBusNum()));
    desc.append(", ");
    desc.append("portNum: ").append(std::to_string(getPortNum()));
    desc.append(", ");
    desc.append("address: ").append(std::to_string(getAddress()));
    desc.append(", ");
    desc.append("class: ").append(std::to_string(getClassCode()));
    desc.append(", ");
    desc.append("classDesc: ").append(getClassDesc());
    desc.append(", ");
    desc.append("subClass: ").append(std::to_string(getSubClassCode()));
    desc.append(", ");
    desc.append("protocol: ").append(std::to_string(getProtocolCode()));
    desc.append(", ");
    desc.append("speed: ").append(getSpeedDesc());
    desc.append("\n");
    desc.append("vid: ").append(getVid());
    desc.append(", ");
    desc.append("pid: ").append(getPid());
    desc.append(", ");
    desc.append("serial: ").append(getSerial());
    desc.append(", ");
    desc.append("product: ").append(getProduct());
    desc.append(", ");
    desc.append("manufacturer: ").append(getManufacturer());
#ifdef _WIN32
    desc.append(", ");
    desc.append("deviceName: ").append(getDeviceName());
    desc.append(", ");
    desc.append("deviceDesc: ").append(getDeviceDesc());
#endif
    if (m_devNodes.size() > 0)
    {
        desc.append("\n");
        desc.append("devNodes: ");
        for (size_t i = 0; i < m_devNodes.size(); ++i)
        {
            if (i > 0)
            {
                desc.append(", ");
            }
            desc.append(m_devNodes[i].name);
            if (!m_devNodes[i].fstype.empty())
            {
                desc.append("(").append(m_devNodes[i].fstype).append(")");
            }
        }
    }
    return desc;
}

std::vector<UsbInfo> UsbInfo::queryUsbInfos(const std::function<bool(const UsbInfo& info)>& filterFunc, bool withDevNode, bool mf)
{
    std::vector<UsbInfo> usbInfoList;
    auto usbList = Usb::getAllUsbs(true, true, mf);
    for (size_t i = 0; i < usbList.size(); ++i)
    {
        UsbInfo info(usbList[i]);
        if (!filterFunc || filterFunc(info))
        {
            if (withDevNode)
            {
                info.m_devNodes = queryDevNodes(info);
            }
            usbInfoList.emplace_back(info);
        }
    }
    return usbInfoList;
}

std::vector<UsbInfo::DevNode> UsbInfo::queryDevNodes(const UsbInfo& info)
{
    std::vector<UsbInfo::DevNode> devNodes;
#ifndef _WIN32
    if (info.isHid() || info.isStorage()) /* 只需获取HID和存储类型的设备节点 */
    {
        auto usbDevNodes = queryUsbDevNodes(info.getBusNum(), info.getPortNum(), info.getAddress());
        for (auto iter = usbDevNodes.begin(); usbDevNodes.end() != iter; ++iter)
        {
            devNodes.emplace_back(DevNode(iter->first, iter->second));
        }
    }
#endif
    return devNodes;
}
} // namespace usb
