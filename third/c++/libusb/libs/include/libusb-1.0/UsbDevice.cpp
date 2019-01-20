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

static UsbDevice parseDevice(libusb_device* dev) {
    uint8_t address = 0, busNumber = 0, portNumber = 0;
    unsigned short idVendor = 0, idProduct = 0;
    char manufacturer[256] = { 0 }, product[256] = { 0 }, serialNumber[256] = { 0 };
    if (dev) {
        address = libusb_get_device_address(dev);
        busNumber = libusb_get_bus_number(dev);
        portNumber = libusb_get_port_number(dev);
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(dev, &desc) >= 0) {
            idVendor = desc.idVendor;
            idProduct = desc.idProduct;
            libusb_device_handle* handle = NULL;
            if (LIBUSB_SUCCESS == libusb_open(dev, &handle)) {
                if (desc.iManufacturer) {
                    libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char*)manufacturer, sizeof(manufacturer));
                }
                if (desc.iProduct) {
                    libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char*)product, sizeof(product));
                }
                if (desc.iSerialNumber) {
                    libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, (unsigned char*)serialNumber, sizeof(serialNumber));
                }
                libusb_close(handle);
            }
        }
    }
    UsbDevice usb;
    usb.address = address;
    usb.busNumber = busNumber;
    usb.portNumber = portNumber;
    usb.vendorId = idVendor;
    usb.productId = idProduct;
    usb.manufacturer = manufacturer;
    usb.product = product;
    usb.serialNumber = serialNumber;
    return usb;
}

static std::vector<UsbDevice> getUsbList(libusb_context* ctx) {
    std::vector<UsbDevice> usbList;
    if (ctx) {
        libusb_device** devList = NULL;
        ssize_t count = libusb_get_device_list(ctx, &devList);
        if (count > 0) {
            libusb_device* dev = NULL;
            int i = 0;
            while ((dev = devList[i++])) {
                usbList.push_back(parseDevice(dev));
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
        std::vector<UsbDevice> usbList = getUsbList(sLibusbContext);
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

bool UsbDevice::open(LIBUSB_DEVICE_HOTPLUG_CALLBACK arrivedCallback, LIBUSB_DEVICE_HOTPLUG_CALLBACK leftCallback) {
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
    sUsbList = getUsbList(sLibusbContext);
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
        usbList = getUsbList(sLibusbContext);
    } else {
        usbList = sUsbList;
    }
    sLibusbContextMutex.unlock();
    return usbList;
}
