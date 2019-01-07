/**********************************************************************
* Author:	jaron.ho
* Date:		2019-01-07
* Brief:	USB设备
**********************************************************************/
#include "UsbDevice.h"
#include <unistd.h>
#include <list>
#include <mutex>
#include <thread>

static libusb_context* sLibusbContext = NULL;
static std::mutex sLibusbContextMutex;
static std::vector<UsbDevice> sUsbList;
static std::list<UsbDevice> sUsbArrivedList;
static std::list<UsbDevice> sUsbLeftList;
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

static int LIBUSB_CALL deviceArrivedCallback(struct libusb_context* ctx, struct libusb_device* dev, libusb_hotplug_event event, void* userdata) {
    if (sLibusbDeviceArrivedCallback) {
        sLibusbDeviceArrivedCallback(parseDevice(dev));
    }
}

static int LIBUSB_CALL deviceLeftCallback(struct libusb_context* ctx, struct libusb_device* dev, libusb_hotplug_event event, void* userdata) {
    if (sLibusbDeviceLeftCallback) {
        sLibusbDeviceLeftCallback(parseDevice(dev));
    }
}

static void detectThreadHandler(void) {
    while (true) {
        sLibusbContextMutex.lock();
        if (!sLibusbContext) {
            sUsbList.clear();
            sUsbArrivedList.clear();
            sUsbLeftList.clear();
            break;
        }
        std::vector<UsbDevice> usbList = getUsbList(sLibusbContext);
        for (size_t i = 0; i < usbList.size(); ++i) {
            UsbDevice usb = usbList[i];
            bool isArrived = true;
            for (size_t j = 0; j < sUsbList.size(); ++j) {
                UsbDevice usbTmp = sUsbList[j];
                if (usb.address == usbTmp.address && usb.busNumber == usbTmp.busNumber && usb.portNumber == usbTmp.portNumber && usb.vendorId == usbTmp.vendorId && usb.productId == usbTmp.productId) {
                    isArrived = false;
                    break;
                }
            }
            if (isArrived) {
                sUsbArrivedList.push_back(usb);
            }
        }
        for (size_t i = 0; i < sUsbList.size(); ++i) {
            UsbDevice usb = sUsbList[i];
            bool isLeft = true;
            for (size_t j = 0; j < usbList.size(); ++j) {
                UsbDevice usbTmp = usbList[j];
                if (usb.address == usbTmp.address && usb.busNumber == usbTmp.busNumber && usb.portNumber == usbTmp.portNumber && usb.vendorId == usbTmp.vendorId && usb.productId == usbTmp.productId) {
                    isLeft = false;
                    break;
                }
            }
            if (isLeft) {
                sUsbLeftList.push_back(usb);
            }
        }
        sUsbList = usbList;
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
    sUsbList = getUsbList(sLibusbContext);
    sUsbArrivedList.clear();
    sUsbLeftList.clear();
    sLibusbDeviceArrivedCallback = arrivedCallback;
    sLibusbDeviceLeftCallback = leftCallback;
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
    sLibusbContextMutex.unlock();
}

void UsbDevice::listen(void) {
    sLibusbContextMutex.lock();
    while (sUsbArrivedList.size() > 0) {
        UsbDevice usb = *sUsbArrivedList.begin();
        sUsbArrivedList.pop_front();
        if (sLibusbDeviceArrivedCallback) {
            sLibusbDeviceArrivedCallback(usb);
        }
    }
    while (sUsbLeftList.size() > 0) {
        UsbDevice usb = *sUsbLeftList.begin();
        sUsbLeftList.pop_front();
        if (sLibusbDeviceLeftCallback) {
            sLibusbDeviceLeftCallback(usb);
        }
    }
    sLibusbContextMutex.unlock();
}

std::vector<UsbDevice> UsbDevice::getList(void) {
    std::vector<UsbDevice> usbList;
    sLibusbContextMutex.lock();
    usbList = getUsbList(sLibusbContext);
    sLibusbContextMutex.unlock();
    return usbList;
}
