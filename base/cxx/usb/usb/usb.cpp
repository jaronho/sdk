#include "usb.h"

#include <libusb.h>

namespace usb
{
Usb::Usb() : m_busNum(-1), m_portNum(-1), m_address(-1), m_classCode(-1) {}

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
        for (ssize_t i = 0; i < count; ++i) /* 遍历设备列表 */
        {
            libusb_device* dev = devList[i];
            if (!dev)
            {
                continue;
            }
            struct libusb_device_descriptor desc;
            if (LIBUSB_SUCCESS != libusb_get_device_descriptor(dev, &desc))
            {
                continue;
            }
            Usb info;
            info.m_busNum = libusb_get_bus_number(dev); /* 总线编号 */
            info.m_portNum = libusb_get_port_number(dev); /* 端口编号(Linux中也叫系统编号) */
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
            }
            usbList.emplace_back(info);
        }
        libusb_free_device_list(devList, 1);
    }
    libusb_exit(NULL);
    return usbList;
}
} // namespace usb
