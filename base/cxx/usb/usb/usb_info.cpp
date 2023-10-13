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

/**
 * @brief 查询USB设备节点列表
 * @param busNum 总线
 * @param portNum 端口
 * @param address 地址
 * @param devRootNode [输出]设备根节点
 * @return USB设备节点列表 
 */
std::vector<DevNode> queryUsbDevNodes(int busNum, int portNum, int address, DevNode& devRootNode)
{
    devRootNode = DevNode();
    std::vector<DevNode> devNodes;
#ifndef _WIN32
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
                        auto pos = std::string(devNode).rfind('/');
                        auto devName = std::string::npos == pos ? std::string() : std::string(devNode).substr(pos + 1);
                        auto command = std::string("lsblk -aOP | grep -E 'NAME=\"") + devName + "\" KNAME=\"" + devName + "\" '";
                        auto outStr = runCommand(command);
                        static const std::string GROUP_FLAG = " GROUP=\""; /* 组名 */
                        static const std::string FSTYPE_FLAG = " FSTYPE=\""; /* 文件系统类型 */
                        static const std::string LABEL_FLAG = " LABEL=\""; /* 文件系统标签 */
                        static const std::string PARTLABEL_FLAG = " PARTLABEL=\""; /* 分区标签 */
                        static const std::string TYPE_FLAG = " TYPE=\""; /* 设备类型 */
                        static const std::string MODEL_FLAG = " MODEL=\""; /* 设备标识符 */
                        static const std::string VENDOR_FLAG = " VENDOR=\""; /* 设备制造商 */
                        std::string group;
                        auto groupPos = outStr.find(GROUP_FLAG);
                        if (std::string::npos != groupPos)
                        {
                            auto ep = outStr.find('"', groupPos + GROUP_FLAG.size());
                            if (std::string::npos != ep)
                            {
                                group = outStr.substr(groupPos + GROUP_FLAG.size(), ep - groupPos - GROUP_FLAG.size());
                            }
                        }
                        std::string fstype;
                        auto fstypePos = outStr.find(FSTYPE_FLAG);
                        if (std::string::npos != fstypePos)
                        {
                            auto ep = outStr.find('"', fstypePos + FSTYPE_FLAG.size());
                            if (std::string::npos != ep)
                            {
                                fstype = outStr.substr(fstypePos + FSTYPE_FLAG.size(), ep - fstypePos - FSTYPE_FLAG.size());
                            }
                        }
                        std::string label;
                        auto labelPos = outStr.find(LABEL_FLAG);
                        if (std::string::npos != labelPos)
                        {
                            auto ep = outStr.find('"', labelPos + LABEL_FLAG.size());
                            if (std::string::npos != ep)
                            {
                                label = outStr.substr(labelPos + LABEL_FLAG.size(), ep - labelPos - LABEL_FLAG.size());
                            }
                        }
                        std::string partlabel;
                        auto partlabelPos = outStr.find(PARTLABEL_FLAG);
                        if (std::string::npos != partlabelPos)
                        {
                            auto ep = outStr.find('"', partlabelPos + PARTLABEL_FLAG.size());
                            if (std::string::npos != ep)
                            {
                                partlabel = outStr.substr(partlabelPos + PARTLABEL_FLAG.size(), ep - partlabelPos - PARTLABEL_FLAG.size());
                            }
                        }
                        std::string type;
                        auto typePos = outStr.find(TYPE_FLAG);
                        if (std::string::npos != typePos)
                        {
                            auto ep = outStr.find('"', typePos + TYPE_FLAG.size());
                            if (std::string::npos != ep)
                            {
                                type = outStr.substr(typePos + TYPE_FLAG.size(), ep - typePos - TYPE_FLAG.size());
                            }
                        }
                        std::string model;
                        auto modelPos = outStr.find(MODEL_FLAG);
                        if (std::string::npos != modelPos)
                        {
                            auto ep = outStr.find('"', modelPos + MODEL_FLAG.size());
                            if (std::string::npos != ep)
                            {
                                model = outStr.substr(modelPos + MODEL_FLAG.size(), ep - modelPos - MODEL_FLAG.size());
                            }
                        }
                        std::string vendor;
                        auto vendorPos = outStr.find(VENDOR_FLAG);
                        if (std::string::npos != vendorPos)
                        {
                            auto ep = outStr.find('"', vendorPos + VENDOR_FLAG.size());
                            if (std::string::npos != ep)
                            {
                                vendor = outStr.substr(vendorPos + VENDOR_FLAG.size(), ep - vendorPos - VENDOR_FLAG.size());
                            }
                        }
                        if ("disk" == group) /* 磁盘 */
                        {
                            if ("disk" == type) /* 超块(不可挂载) */
                            {
                                devRootNode = DevNode(devNode, group, fstype, label, partlabel, model, vendor);
                            }
                            else if ("part" == type) /* 分区 */
                            {
                                devNodes.emplace_back(DevNode(devNode, group, fstype, label, partlabel, model, vendor));
                            }
                        }
                        else if ("cdrom" == group) /* 光驱 */
                        {
                            devNodes.emplace_back(DevNode(devNode, group, fstype, label, partlabel, model, vendor));
                        }
                    }
                    else
                    {
                        devNodes.emplace_back(DevNode(devNode));
                    }
                }
            }
        }
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
#endif
    return devNodes;
} // namespace usb

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

DevNode UsbInfo::getDevRootNode() const
{
    return m_devRootNode;
}

std::vector<DevNode> UsbInfo::getDevNodes() const
{
    return m_devNodes;
}

bool UsbInfo::isValid() const
{
    return (getBusNum() > 0 && getPortNum() > 0 && getAddress() > 0 && !getVid().empty() && !getPid().empty());
}

std::string UsbInfo::describe(int allIntend, int intend) const
{
    std::string allIntendStr(allIntend, ' '), intendStr(intend, ' ');
    std::string desc;
    desc += allIntendStr + "{";
    desc += "\n"; /* 换行 */
    desc += allIntendStr + intendStr;
    desc += "\"busNum\": " + std::to_string(getBusNum());
    desc += ", ";
    desc += "\"portNum\": " + std::to_string(getPortNum());
    desc += ", ";
    desc += "\"address\": " + std::to_string(getAddress());
    desc += ", ";
    desc += "\"classCode\": " + std::to_string(getClassCode());
    desc += ", ";
    desc += "\"classHex\": \"" + getClassHex() + "\"";
    desc += ", ";
    desc += "\"classDesc\": \"" + getClassDesc() + "\"";
    desc += ", ";
    desc += "\"subClass\": " + std::to_string(getSubClassCode());
    desc += ", ";
    desc += "\"protocol\": " + std::to_string(getProtocolCode());
    desc += ", ";
    desc += "\"speed\": " + std::to_string(getSpeedLevel());
    desc += ", ";
    desc += "\"speedDesc\": \"" + getSpeedDesc() + "\"";
    desc += ",";
    desc += "\n"; /* 换行 */
    desc += allIntendStr + intendStr;
    desc += "\"vid\": \"" + getVid() + "\"";
    desc += ", ";
    desc += "\"pid\": \"" + getPid() + "\"";
    desc += ", ";
    desc += "\"serial\": \"" + getSerial() + "\"";
    desc += ", ";
    desc += "\"product\": \"" + getProduct() + "\"";
    desc += ", ";
    desc += "\"manufacturer\": \"" + getManufacturer() + "\"";
#ifdef _WIN32
    desc += ", ";
    desc += "\"deviceName\": \"" + getDeviceName() + "\"";
    desc += ", ";
    desc += "\"deviceDesc\": \"" + getDeviceDesc() + "\"";
    if (isStorage())
    {
        desc += ", ";
        desc += "\"storageType\": \"" + getStorageType() + "\"";
    }
#endif
    if (!m_devRootNode.name.empty())
    {
        desc += ",";
        desc += "\n"; /* 换行 */
        desc += allIntendStr + intendStr;
        desc += "\"devRootNode\": ";
        desc += "{";
        desc += "\"name\": \"" + m_devRootNode.name + "\"";
        std::string temp;
        if (!m_devRootNode.group.empty())
        {
            desc += ", ";
            desc += "\"group\": \"" + m_devRootNode.group + "\"";
        }
        if (!m_devRootNode.fstype.empty())
        {
            desc += ", ";
            desc += "\"fstype\": \"" + m_devRootNode.fstype + "\"";
        }
        if (!m_devRootNode.label.empty())
        {
            desc += ", ";
            desc += "\"label\": \"" + m_devRootNode.label + "\"";
        }
        if (!m_devRootNode.partlabel.empty())
        {
            desc += ", ";
            desc += "\"partlabel\": \"" + m_devRootNode.partlabel + "\"";
        }
        if (!m_devRootNode.model.empty())
        {
            desc += ", ";
            desc += "\"model\": \"" + m_devRootNode.model + "\"";
        }
        if (!m_devRootNode.vendor.empty())
        {
            desc += ", ";
            desc += "\"vendor\": \"" + m_devRootNode.vendor + "\"";
        }
        desc += "}";
    }
    if (m_devNodes.size() > 0)
    {
        desc += ",";
        desc += "\n"; /* 换行 */
        desc += allIntendStr + intendStr;
        desc += "\"devNodes\": ";
        desc += "[";
        for (size_t i = 0; i < m_devNodes.size(); ++i)
        {
            if (i > 0)
            {
                desc += ",";
            }
            if (m_devNodes.size() > 1)
            {
                desc += "\n"; /* 换行 */
                desc += allIntendStr + intendStr + intendStr;
            }
            desc += "{";
            desc += "\"name\": \"" + m_devNodes[i].name + "\"";
            std::string temp;
            if (!m_devNodes[i].group.empty())
            {
                desc += ", ";
                desc += "\"group\": \"" + m_devNodes[i].group + "\"";
            }
            if (!m_devNodes[i].fstype.empty())
            {
                desc += ", ";
                desc += "\"type\": \"" + m_devNodes[i].fstype + "\"";
            }
            if (!m_devNodes[i].label.empty())
            {
                desc += ", ";
                desc += "\"label\": \"" + m_devNodes[i].label + "\"";
            }
            if (!m_devNodes[i].partlabel.empty())
            {
                desc += ", ";
                desc += "\"partlabel\": \"" + m_devNodes[i].partlabel + "\"";
            }
            if (!m_devNodes[i].model.empty())
            {
                desc += ", ";
                desc += "\"model\": \"" + m_devNodes[i].model + "\"";
            }
            if (!m_devNodes[i].vendor.empty())
            {
                desc += ", ";
                desc += "\"vendor\": \"" + m_devNodes[i].vendor + "\"";
            }
            desc += "}";
        }
        if (m_devNodes.size() > 1)
        {
            desc += "\n"; /* 换行 */
            desc += allIntendStr + intendStr;
        }
        desc += "]";
    }
    desc += "\n"; /* 换行 */
    desc += allIntendStr;
    desc += "}";
    return desc;
}

std::vector<UsbInfo> UsbInfo::queryUsbInfos(const std::function<bool(const UsbInfo& info, bool& withDevNode)>& filterFunc, bool mf)
{
    std::vector<UsbInfo> usbInfoList;
    auto usbList = Usb::getAllUsbs(true, true, mf);
    for (size_t i = 0; i < usbList.size(); ++i)
    {
        UsbInfo info(usbList[i]);
        bool withDevNode = false;
        if (!filterFunc || filterFunc(info, withDevNode))
        {
            if (withDevNode && (info.isHid() || info.isStorage())) /* 只需获取HID和存储类型的设备节点 */
            {
                info.m_devNodes = queryUsbDevNodes(info.getBusNum(), info.getPortNum(), info.getAddress(), info.m_devRootNode);
            }
            usbInfoList.emplace_back(info);
        }
    }
    return usbInfoList;
}
} // namespace usb
