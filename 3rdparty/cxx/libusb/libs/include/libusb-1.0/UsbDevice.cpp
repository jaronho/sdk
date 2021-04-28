/**********************************************************************
* Author:	jaron.ho
* Date:		2019-01-07
* Brief:	USB设备
**********************************************************************/
#include "UsbDevice.h"
#include <unistd.h>
#include <mutex>
#include <thread>

static libusb_context* sLibusbContext = NULL;
static std::mutex sLibusbContextMutex;
static std::vector<UsbDevice> sUsbList;
static LIBUSB_DEVICE_HOTPLUG_CALLBACK sLibusbDeviceArrivedCallback;
static LIBUSB_DEVICE_HOTPLUG_CALLBACK sLibusbDeviceLeftCallback;
static std::vector<libusb_class_code> sValidClassCodes;

static UsbDevice parseDevice(libusb_device* dev, const std::vector<libusb_class_code>& codes) {
    struct libusb_device_descriptor desc;
    if (!dev || libusb_get_device_descriptor(dev, &desc) < 0) {
        return UsbDevice();
    }
    unsigned char classCode = desc.bDeviceClass;
    if (LIBUSB_CLASS_PER_INTERFACE == classCode && desc.bNumConfigurations > 0) {
        struct libusb_config_descriptor* config;
        if (LIBUSB_SUCCESS == libusb_get_config_descriptor(dev, 0, &config)) {
            if (config->bNumInterfaces > 0) {
                struct libusb_interface interface = config->interface[0];
                if (interface.num_altsetting > 0) {
                    struct libusb_interface_descriptor altsetting = interface.altsetting[0];
                    classCode = altsetting.bInterfaceClass;
                }
            }
            libusb_free_config_descriptor(config);
        }
    }
    bool isMatched = false;
    if (codes.empty()) {
        isMatched = true;
    } else {
        for (size_t i = 0, len = codes.size(); i < len; ++i) {
            if (classCode == codes[i]) {
                isMatched = true;
                break;
            }
        }
    }
    if (!isMatched) {
        return UsbDevice();
    }
    UsbDevice usb;
    usb.classCode = classCode;
    usb.vendorId = desc.idVendor;
    usb.productId = desc.idProduct;
    usb.busNumber = libusb_get_bus_number(dev);
    usb.portNumber = libusb_get_port_number(dev);
    usb.address = libusb_get_device_address(dev);
    libusb_device_handle* handle = NULL;
    if (LIBUSB_SUCCESS == libusb_open(dev, &handle)) {
        if (desc.iManufacturer) {
            char manufacturer[256] = { 0 };
            libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char*)manufacturer, sizeof(manufacturer));
            usb.manufacturer = manufacturer;
        }
        if (desc.iProduct) {
            char product[256] = { 0 };
            libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char*)product, sizeof(product));
            usb.product = product;
        }
        if (desc.iSerialNumber) {
            char serialNumber[256] = { 0 };
            libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, (unsigned char*)serialNumber, sizeof(serialNumber));
            usb.serialNumber = serialNumber;
        }
        libusb_close(handle);
    }
    return usb;
}

static std::vector<UsbDevice> getUsbList(libusb_context* ctx, const std::vector<libusb_class_code>& codes) {
    std::vector<UsbDevice> usbList;
    if (ctx) {
        libusb_device** devList = NULL;
        ssize_t count = libusb_get_device_list(ctx, &devList);
        if (count > 0) {
            libusb_device* dev = NULL;
            int i = 0;
            while ((dev = devList[i++])) {
                UsbDevice usb = parseDevice(dev, codes);
                if (usb.vendorId > 0 && usb.productId > 0) {
                    usbList.push_back(usb);
                }
            }
            libusb_free_device_list(devList, 1);
        }
    }
    return usbList;
}

static std::vector<UsbDevice>::iterator findInUsbVector(std::vector<UsbDevice>& usbList, UsbDevice usb) {
    std::vector<UsbDevice>::iterator iter = usbList.begin();
    for (; usbList.end() != iter; ++iter) {
        if (usb.address == (*iter).address && usb.busNumber == (*iter).busNumber && usb.portNumber == (*iter).portNumber && usb.vendorId == (*iter).vendorId && usb.productId == (*iter).productId) {
            break;
        }
    }
    return iter;
}

static void detectThreadHandler(void) {
    while (true) {
        sLibusbContextMutex.lock();
        if (!sLibusbContext) {
            sLibusbContextMutex.unlock();
            break;
        }
        std::vector<UsbDevice> usbList = getUsbList(sLibusbContext, sValidClassCodes);
        /* step1: check usb arrived */
        std::vector<UsbDevice> usbArrivedList;
        for (size_t i = 0; i < usbList.size(); ++i) {
            UsbDevice usb = usbList[i];
            std::vector<UsbDevice>::iterator iter = findInUsbVector(sUsbList, usb);
            if (sUsbList.end() == iter) {
                usbArrivedList.push_back(usb);
            } else {
                if (usb.manufacturer.empty()) {
                    usbList[i].manufacturer = (*iter).manufacturer;
                }
                if (usb.product.empty()) {
                    usbList[i].product = (*iter).product;
                }
                if (usb.serialNumber.empty()) {
                    usbList[i].serialNumber = (*iter).serialNumber;
                }
            }
        }
        /* step2: check usb left */
        std::vector<UsbDevice> usbLeftList;
        for (size_t i = 0; i < sUsbList.size(); ++i) {
            UsbDevice usb = sUsbList[i];
            std::vector<UsbDevice>::iterator iter = findInUsbVector(usbList, usb);
            if (usbList.end() == iter) {
                usbLeftList.push_back(usb);
            }
        }
        /* step3: update usb list */
        if (usbArrivedList.size() > 0 || usbLeftList.size() > 0) {
            sUsbList = usbList;
        }
        /* step4: notify detect result */
        if (sLibusbDeviceArrivedCallback) {
            for (size_t i = 0; i < usbArrivedList.size(); ++i) {
                sLibusbDeviceArrivedCallback(usbArrivedList[i]);
            }
        }
        if (sLibusbDeviceLeftCallback) {
            for (size_t i = 0; i < usbLeftList.size(); ++i) {
                sLibusbDeviceLeftCallback(usbLeftList[i]);
            }
        }
        sLibusbContextMutex.unlock();
        usleep(1000);
    }
}

bool UsbDevice::isOpen(void) {
    bool flag = false;
    sLibusbContextMutex.lock();
    if (sLibusbContext) {
        flag = true;
    }
    sLibusbContextMutex.unlock();
    return flag;
}

bool UsbDevice::open(LIBUSB_DEVICE_HOTPLUG_CALLBACK arrivedCallback, LIBUSB_DEVICE_HOTPLUG_CALLBACK leftCallback, const std::vector<libusb_class_code>& validClassCodes) {
    sLibusbContextMutex.lock();
    if (sLibusbContext) {
        sLibusbContextMutex.unlock();
        return true;
    }
    if (0 < libusb_init(&sLibusbContext)) {
        sLibusbContextMutex.unlock();
        return false;
    }
    sLibusbDeviceArrivedCallback = arrivedCallback;
    sLibusbDeviceLeftCallback = leftCallback;
    sValidClassCodes = validClassCodes;
    sUsbList = getUsbList(sLibusbContext, validClassCodes);
    std::thread detectThread(detectThreadHandler);
    detectThread.detach();
    sLibusbContextMutex.unlock();
    return true;
}

void UsbDevice::close(void) {
    sLibusbContextMutex.lock();
    if (!sLibusbContext) {
        sLibusbContextMutex.unlock();
        return;
    }
    libusb_exit(sLibusbContext);
    sLibusbContext = NULL;
    sUsbList.clear();
    sLibusbContextMutex.unlock();
}

std::vector<UsbDevice> UsbDevice::getList(bool sync) {
    std::vector<UsbDevice> usbList;
    sLibusbContextMutex.lock();
    if (sync) {
        usbList = getUsbList(sLibusbContext, sValidClassCodes);
    } else {
        usbList = sUsbList;
    }
    sLibusbContextMutex.unlock();
    return usbList;
}
