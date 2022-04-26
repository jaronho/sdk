#include "usb.h"

#ifdef _WIN32
#include <algorithm>
// Windows.h必须比其他平台文件先包含
#include <Windows.h>
// initguid.h必须在devpkey.h前面包含
#include <initguid.h>
//
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <devpkey.h>
#pragma comment(lib, "setupapi.lib")
#endif

namespace usb
{
#ifdef _WIN32
static std::string wstring2string(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    char* buf = (char*)malloc(sizeof(char) * (len + 1));
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buf, len, NULL, NULL);
    buf[len] = '\0';
    std::string str(buf);
    free(buf);
    return str;
}

/**
 * @brief 解析LocationInfo属性, 属性值格式必须为: "Port_#0014.Hub_#0001"
 *        "Hub_#0001"表示busNum为"0001", "Port_#0014"表示portNum值为"0014"
 */
bool parseLocationInfo(WCHAR propertyBuffer[4096], int& busNum, int& portNum)
{
    busNum = -1;
    portNum = -1;
    std::string buffer = wstring2string(propertyBuffer);
    auto pos = buffer.find("Hub_#");
    if (std::string::npos == pos || pos + 9 > buffer.size())
    {
        return false;
    }
    auto busNumBuf = buffer.substr(pos + 5, 4);
    pos = buffer.find("Port_#");
    if (std::string::npos == pos || pos + 10 > buffer.size())
    {
        return false;
    }
    auto portNumBuf = buffer.substr(pos + 6, 4);
    try
    {
        busNum = std::atoi(busNumBuf.c_str());
        portNum = std::atoi(portNumBuf.c_str());
    }
    catch (...)
    {
        busNum = -1;
        portNum = -1;
        return false;
    }
    return true;
}

/**
 * @brief 解析InstanceId属性, 属性值格式必须为: "USB\VID_0930&PID_140A\D04B61EC2646E25150000113"
 *        "VID_0930"表示VID值为"0930", "PID_140A"表示PID值为"140A"
 *        "D04B61EC2646E25150000113"表示为序列号, 注意只有存储设备才有序列号, 且序列号中不含有"&"
 *        例如, 非存储设备的值格式为: "USB\VID_093A&PID_2510\5&31036E8E&0&3"
 */
bool parseInstanceId(WCHAR propertyBuffer[4096], std::string& instanceId, std::string& vid, std::string& pid, std::string& serial)
{
    instanceId.clear();
    vid.clear();
    pid.clear();
    serial.clear();
    std::string buffer = wstring2string(propertyBuffer);
    auto pos = buffer.find("VID_");
    if (std::string::npos == pos || pos + 8 > buffer.size())
    {
        return false;
    }
    vid = buffer.substr(pos + 4, 4);
    std::transform(vid.begin(), vid.end(), vid.begin(), tolower);
    pos = buffer.find("PID_");
    if (std::string::npos == pos || pos + 8 > buffer.size())
    {
        vid.clear();
        return false;
    }
    pid = buffer.substr(pos + 4, 4);
    std::transform(pid.begin(), pid.end(), pid.begin(), tolower);
    pos = buffer.find_last_of('\\');
    if (std::string::npos != pos)
    {
        serial = buffer.substr(pos + 1);
        if (std::string::npos != serial.find('&'))
        {
            serial.clear();
        }
    }
    instanceId = buffer;
    return true;
}

/**
 * @brief 解析Children属性, 属性值格式, 例如: "USBSTOR\Disk&Ven_TOSHIBA&Prod_TransMemory&Rev_PMAP\D04B61EC2646E25150000113&0"
 *        "Prod_TransMemory"表示产品名称为"TransMemory", "Ven_TOSHIBA"表示厂商名称为"TOSHIBA"
 *        如果无产品名称和厂商名称, 格式可能为: "HID\VID_093A&PID_2510\6&365403a8&0&0000"
 */
void parseChildren(WCHAR propertyBuffer[4096], std::string& product, std::string& manufacturer)
{
    product.clear();
    manufacturer.clear();
    std::string buffer = wstring2string(propertyBuffer);
    auto pos = buffer.find("Prod_");
    if (std::string::npos != pos)
    {
        for (size_t i = pos + 5; i < buffer.size(); ++i)
        {
            if ('&' == buffer[i])
            {
                break;
            }
            product.push_back(buffer[i]);
        }
    }
    pos = buffer.find("Ven_");
    if (std::string::npos != pos)
    {
        for (size_t i = pos + 4; i < buffer.size(); ++i)
        {
            if ('&' == buffer[i])
            {
                break;
            }
            manufacturer.push_back(buffer[i]);
        }
    }
}

struct WinUsb
{
public:
    std::string parentInstanceId;
    std::string instanceId;
    int busNum;
    int portNum;
    std::string vid;
    std::string pid;
    std::string serial;
    std::string product;
    std::string manufacturer;
};

int confirmBusNum(const WinUsb& info, const std::vector<WinUsb>& winUsbList)
{
    auto parentInstanceId = info.parentInstanceId;
    std::transform(parentInstanceId.begin(), parentInstanceId.end(), parentInstanceId.begin(), tolower);
    for (const auto& item : winUsbList)
    {
        auto instanceId = item.instanceId;
        std::transform(instanceId.begin(), instanceId.end(), instanceId.begin(), tolower);
        if (parentInstanceId == instanceId)
        {
            return confirmBusNum(item, winUsbList);
        }
    }
    return info.busNum;
}
#endif

Usb::Usb() : m_parent(nullptr), m_busNum(-1), m_portNum(-1), m_address(-1), m_classCode(-1), m_speedLevel(0) {}

Usb::Usb(const Usb& src)
{
    m_parent = src.m_parent;
    m_busNum = src.m_busNum;
    m_portNum = src.m_portNum;
    m_address = src.m_address;
    m_classCode = src.m_classCode;
    m_speedLevel = src.m_speedLevel;
    m_vid = src.m_vid;
    m_pid = src.m_pid;
    m_serial = src.m_serial;
    m_product = src.m_product;
    m_manufacturer = src.m_manufacturer;
}

std::shared_ptr<Usb> Usb::getParent() const
{
    return m_parent;
}

int Usb::getBusNum() const
{
    return m_busNum;
}

int Usb::getPortNum() const
{
    return m_portNum;
}

int Usb::getAddress() const
{
    return m_address;
}

int Usb::getClassCode() const
{
    return m_classCode;
}

std::string Usb::getClassDesc() const
{
    switch (m_classCode)
    {
    case LIBUSB_CLASS_PER_INTERFACE:
        return "Per Interface";
    case LIBUSB_CLASS_AUDIO:
        return "Audio";
    case LIBUSB_CLASS_COMM:
        return "Communications";
    case LIBUSB_CLASS_HID:
        return "Human Interface Device";
    case LIBUSB_CLASS_PHYSICAL:
        return "Physical";
    case LIBUSB_CLASS_IMAGE:
        return "Image";
    case LIBUSB_CLASS_PRINTER:
        return "Printer";
    case LIBUSB_CLASS_MASS_STORAGE:
        return "Mass Storage";
    case LIBUSB_CLASS_HUB:
        return "Hub";
    case LIBUSB_CLASS_DATA:
        return "Data";
    case LIBUSB_CLASS_SMART_CARD:
        return "Smart Card";
    case LIBUSB_CLASS_CONTENT_SECURITY:
        return "Content Security";
    case LIBUSB_CLASS_VIDEO:
        return "Video";
    case LIBUSB_CLASS_PERSONAL_HEALTHCARE:
        return "Personal Healthcare";
    case LIBUSB_CLASS_DIAGNOSTIC_DEVICE:
        return "Diagnostic Device";
    case LIBUSB_CLASS_WIRELESS:
        return "Wireless";
    case LIBUSB_CLASS_MISCELLANEOUS:
        return "Miscellaneous";
    case LIBUSB_CLASS_APPLICATION:
        return "Application";
    case LIBUSB_CLASS_VENDOR_SPEC:
        return "Vendor Specific";
    }
    return std::to_string(m_classCode);
}

int Usb::getSpeedLevel() const
{
    return m_speedLevel;
}

std::string Usb::getSpeedDesc() const
{
    switch (m_speedLevel)
    {
    case LIBUSB_SPEED_UNKNOWN:
        return "unknown";
    case LIBUSB_SPEED_LOW:
        return "1.5MBit/s";
    case LIBUSB_SPEED_FULL:
        return "12MBit/s";
    case LIBUSB_SPEED_HIGH:
        return "480MBit/s";
    case LIBUSB_SPEED_SUPER:
        return "5000MBit/s";
    case LIBUSB_SPEED_SUPER_PLUS:
        return "10000MBit/s";
    }
    return std::to_string(m_speedLevel);
}

std::string Usb::getVid() const
{
    return m_vid;
}

std::string Usb::getPid() const
{
    return m_pid;
}

std::string Usb::getSerial() const
{
    return m_serial;
}

std::string Usb::getProduct() const
{
    return m_product;
}

std::string Usb::getManufacturer() const
{
    return m_manufacturer;
}

bool Usb::isHid() const
{
    return (LIBUSB_CLASS_HID == m_classCode);
}

bool Usb::isStorage() const
{
    return (LIBUSB_CLASS_MASS_STORAGE == m_classCode);
}

bool Usb::isHub() const
{
    return (LIBUSB_CLASS_HUB == m_classCode);
}

std::vector<Usb> Usb::getAllUsbs(bool sf, bool pf, bool mf)
{
    std::vector<Usb> usbList;
    if (LIBUSB_SUCCESS != libusb_init(NULL))
    {
        return usbList;
    }
    libusb_device** devList;
    ssize_t count = libusb_get_device_list(NULL, &devList);
    if (count > 0 && devList)
    {
#ifdef _WIN32
        std::vector<WinUsb> winUsbList;
        if (sf || pf || mf)
        {
            getWinUsbList(winUsbList);
        }
#endif
        for (ssize_t i = 0; i < count; ++i) /* 遍历设备列表 */
        {
            Usb info;
#ifdef _WIN32
            if (parseUsb(devList[i], sf, pf, mf, winUsbList, info))
#else
            if (parseUsb(devList[i], sf, pf, mf, info))
#endif
            {
                usbList.emplace_back(info);
            }
        }
        libusb_free_device_list(devList, 1);
    }
    libusb_exit(NULL);
    return usbList;
}

#ifdef _WIN32
void Usb::getWinUsbList(std::vector<WinUsb>& winUsbList)
{
    winUsbList.clear();
    HDEVINFO deviceInfo = SetupDiGetClassDevsW(NULL, L"USB", NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (!deviceInfo || INVALID_HANDLE_VALUE == deviceInfo)
    {
        return;
    }
    int index = 0;
    while (1)
    {
        SP_DEVINFO_DATA deviceData;
        deviceData.cbSize = sizeof(deviceData);
        if (!SetupDiEnumDeviceInfo(deviceInfo, index++, &deviceData))
        {
            break;
        }
        DEVPROPTYPE propertyType = 0;
        WCHAR propertyBuffer[4096] = {0};
        DWORD requiredSize = 0;
        WinUsb info;
        /* 解析父节点 */
        memset(propertyBuffer, 0, sizeof(propertyBuffer));
        if (!SetupDiGetDevicePropertyW(deviceInfo, &deviceData, &DEVPKEY_Device_Parent, &propertyType,
                                       reinterpret_cast<PBYTE>(propertyBuffer), sizeof(propertyBuffer), &requiredSize, 0))
        {
            return;
        }
        info.parentInstanceId = wstring2string(propertyBuffer);
        /* 解析BusNum, PortNum */
        memset(propertyBuffer, 0, sizeof(propertyBuffer));
        if (!SetupDiGetDevicePropertyW(deviceInfo, &deviceData, &DEVPKEY_Device_LocationInfo, &propertyType,
                                       reinterpret_cast<PBYTE>(propertyBuffer), sizeof(propertyBuffer), &requiredSize, 0))
        {
            continue;
        }
        if (!parseLocationInfo(propertyBuffer, info.busNum, info.portNum))
        {
            continue;
        }
        /* 解析VID, PID, 序列号 */
        memset(propertyBuffer, 0, sizeof(propertyBuffer));
        if (!SetupDiGetDevicePropertyW(deviceInfo, &deviceData, &DEVPKEY_Device_InstanceId, &propertyType,
                                       reinterpret_cast<PBYTE>(propertyBuffer), sizeof(propertyBuffer), &requiredSize, 0))
        {
            continue;
        }
        if (!parseInstanceId(propertyBuffer, info.instanceId, info.vid, info.pid, info.serial))
        {
            continue;
        }
        /* 解析产品名称, 厂商名称 */
        memset(propertyBuffer, 0, sizeof(propertyBuffer));
        if (SetupDiGetDevicePropertyW(deviceInfo, &deviceData, &DEVPKEY_Device_Children, &propertyType,
                                      reinterpret_cast<PBYTE>(propertyBuffer), sizeof(propertyBuffer), &requiredSize, 0))
        {
            parseChildren(propertyBuffer, info.product, info.manufacturer);
        }
        winUsbList.emplace_back(info);
    }
    for (auto& info : winUsbList)
    {
        info.busNum = confirmBusNum(info, winUsbList);
    }
    SetupDiDestroyDeviceInfoList(deviceInfo);
}

bool Usb::matchWinUsbParent(const std::vector<WinUsb>& winUsbList, std::string parentInstanceId, const std::shared_ptr<Usb>& parent)
{
    if (!parent || !parent->m_parent)
    {
        return true;
    }
    std::transform(parentInstanceId.begin(), parentInstanceId.end(), parentInstanceId.begin(), tolower);
    for (const auto& item : winUsbList)
    {
        auto instanceId = item.instanceId;
        std::transform(instanceId.begin(), instanceId.end(), instanceId.begin(), tolower);
        if (instanceId == parentInstanceId && item.busNum == parent->m_busNum && item.portNum == parent->m_portNum
            && item.vid == parent->m_vid && item.pid == parent->m_pid)
        {
            if (matchWinUsbParent(winUsbList, item.parentInstanceId, parent->m_parent))
            {
                return true;
            }
        }
    }
    return false;
}
#endif

#ifdef _WIN32
bool Usb::parseUsb(libusb_device* dev, bool sf, bool pf, bool mf, const std::vector<WinUsb>& winUsbList, Usb& info)
#else
bool Usb::parseUsb(libusb_device* dev, bool sf, bool pf, bool mf, Usb& info)
#endif
{
    if (!dev)
    {
        return false;
    }
    struct libusb_device_descriptor desc;
    if (LIBUSB_SUCCESS != libusb_get_device_descriptor(dev, &desc))
    {
        return false;
    }
    libusb_device* parent = libusb_get_parent(dev);
    if (parent)
    {
        info.m_parent = std::make_shared<Usb>();
#ifdef _WIN32
        if (!parseUsb(parent, sf, pf, mf, winUsbList, *info.m_parent))
#else
        if (!parseUsb(parent, sf, pf, mf, *info.m_parent))
#endif
        {
            info.m_parent = nullptr;
        }
    }
    info.m_busNum = libusb_get_bus_number(dev); /* 总线编号 */
    info.m_portNum = libusb_get_port_number(dev); /* 端口编号(Linux中也叫系统编号sysNum) */
    info.m_address = libusb_get_device_address(dev); /* 地址(每次拔插都会变) */
    int classCode = desc.bDeviceClass; /* 设备类型编码(用于判断鼠标,键盘,Hub等) */
    if (LIBUSB_CLASS_PER_INTERFACE == desc.bDeviceClass && desc.bNumConfigurations > 0)
    {
        struct libusb_config_descriptor* config;
        if (LIBUSB_SUCCESS == libusb_get_config_descriptor(dev, 0, &config) && config)
        {
            if (config->bNumInterfaces > 0 && config->interface[0].altsetting > 0)
            {
                classCode = config->interface[0].altsetting->bInterfaceClass;
            }
            libusb_free_config_descriptor(config);
        }
    }
    info.m_classCode = classCode;
    info.m_speedLevel = libusb_get_device_speed(dev); /* 速度等级 */
    char vid[6] = {0}; /* 厂商ID */
    sprintf(vid, "%04x", desc.idVendor);
    info.m_vid = vid;
    char pid[6] = {0}; /* 产品ID */
    sprintf(pid, "%04x", desc.idProduct);
    info.m_pid = pid;
    if (sf || pf || mf) /* 获取详细信息 */
    {
        libusb_device_handle* handle;
        if (LIBUSB_SUCCESS == libusb_open(dev, &handle) && handle)
        {
            if (sf)
            {
                char serial[256] = {0}; /* 序列号 */
                libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, (unsigned char*)serial, sizeof(serial));
                info.m_serial = serial;
            }
            if (pf)
            {
                char product[256] = {0}; /* 产品名称 */
                libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char*)product, sizeof(product));
                info.m_product = product;
            }
            if (mf)
            {
                char manufacturer[256] = {0}; /* 产品名称 */
                libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char*)manufacturer, sizeof(manufacturer));
                info.m_manufacturer = manufacturer;
            }
            libusb_close(handle);
        }
        else
        {
#ifdef _WIN32
            for (auto item : winUsbList)
            {
                if (item.busNum == info.m_busNum && item.portNum == info.m_portNum && item.vid == info.m_vid && item.pid == info.m_pid)
                {
                    if (matchWinUsbParent(winUsbList, item.parentInstanceId, info.m_parent))
                    {
                        if (sf)
                        {
                            info.m_serial = item.serial;
                        }
                        if (pf)
                        {
                            info.m_product = item.product;
                        }
                        if (mf)
                        {
                            info.m_manufacturer = item.manufacturer;
                        }
                        break;
                    }
                }
            }
#endif
        }
    }
    return true;
}
} // namespace usb
