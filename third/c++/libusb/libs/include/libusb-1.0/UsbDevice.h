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
    int address;                    /* the usb address for the device */
    int busNumber;                  /* the usb bus number of the device */
    int portNumber;                 /* the usb port number */
    int vendorId;                   /* the device vendor id */
    int productId;                  /* the device product id */
    std::string manufacturer;       /* the manufacturer of the device */
    std::string product;            /* the device product name */
    std::string serialNumber;       /* the device serial number */

public:
    static bool open(LIBUSB_DEVICE_HOTPLUG_CALLBACK arrivedCallback, LIBUSB_DEVICE_HOTPLUG_CALLBACK leftCallback);

    static void close(void);

    static void listen(void);

    static std::vector<UsbDevice> getList(void);
};

#endif // _USB_DEVICE_H_
