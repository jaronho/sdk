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

    std::string className(void) {
        switch (classCode) {
        case LIBUSB_CLASS_PER_INTERFACE:
            return "CLASS_PER_INTERFACE";
        case LIBUSB_CLASS_AUDIO:
            return "Audio class";
        case LIBUSB_CLASS_COMM:
            return "Communications class";
        case LIBUSB_CLASS_HID:
            return "Human Interface Device class";
        case LIBUSB_CLASS_PHYSICAL:
            return "Physical";
        case LIBUSB_CLASS_PRINTER:
            return "Printer class";
        case LIBUSB_CLASS_IMAGE:
            return "Image class";
        case LIBUSB_CLASS_MASS_STORAGE:
            return "Mass storage class";
        case LIBUSB_CLASS_HUB:
            return "Hub class";
        case LIBUSB_CLASS_DATA:
            return "Data class";
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
            return "Wireless class";
        case LIBUSB_CLASS_APPLICATION:
            return "Application class";
        case LIBUSB_CLASS_VENDOR_SPEC:
            return "Class is vendor-specific";
        }
        return std::to_string(classCode);
    }

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
