/**********************************************************************
* Author:	jaron.ho
* Date:		2019-01-07
* Brief:	USB设备
**********************************************************************/
#ifndef _USB_DEVICE_H_
#define _USB_DEVICE_H_

#include <functional>
#include <string>
#include <vector>
#include "libusb.h"

#define LIBUSB_DEVICE_HOTPLUG_CALLBACK std::function<void(UsbDevice usb)>

class UsbDevice {
public:
    UsbDevice(void) : classCode(0), vendorId(0), productId(0), busNumber(0), portNumber(0), address(0) {}

public:
    int classCode;                  /* class code for the device, see libusb_class_code */
    int vendorId;                   /* the device vendor id */
    int productId;                  /* the device product id */
    int busNumber;                  /* the usb bus number of the device */
    int portNumber;                 /* the usb port number */
    int address;                    /* the usb address for the device */
    std::string manufacturer;       /* the manufacturer of the device */
    std::string product;            /* the device product name */
    std::string serialNumber;       /* the device serial number */

public:
    /* USB设备检测是否已打开 */
    static bool isOpen(void);

    /* 打开USB设备检测,注意:回调函数将在线程中被调用(回调函数可设置为空) */
    static bool open(LIBUSB_DEVICE_HOTPLUG_CALLBACK arrivedCallback, LIBUSB_DEVICE_HOTPLUG_CALLBACK leftCallback, const std::vector<libusb_class_code>& validClassCodes);

    /* 关闭USB设备检测 */
    static void close(void);

    /* 获取当前USB设备列表 */
    static std::vector<UsbDevice> getList(bool sync = false);
};

#endif // _USB_DEVICE_H_
