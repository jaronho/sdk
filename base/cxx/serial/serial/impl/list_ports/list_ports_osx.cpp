#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <iostream>
#include <stdint.h>
#include <string>
#include <sys/param.h>
#include <vector>

#include "../../serial.h"

#define HARDWARE_ID_STRING_LENGTH 128

namespace serial
{
std::string cfstring_to_string(CFStringRef cfstring);
std::string get_device_path(io_object_t& serial_port);
std::string get_class_name(io_object_t& obj);
io_registry_entry_t get_parent_iousb_device(io_object_t& serial_port);
std::string get_string_property(io_object_t& device, const char* property);
unsigned short get_int_property(io_object_t& device, const char* property);
std::string rtrim(const std::string& str);

std::string cfstring_to_string(CFStringRef cfstring)
{
    char cstring[MAXPATHLEN];
    std::string result;
    if (cfstring)
    {
        Boolean success = CFStringGetCString(cfstring, cstring, sizeof(cstring), kCFStringEncodingASCII);
        if (success)
        {
            result = cstring;
        }
    }
    return result;
}

std::string get_device_path(io_object_t& serial_port)
{
    CFTypeRef callout_path;
    std::string device_path;
    callout_path = IORegistryEntryCreateCFProperty(serial_port, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
    if (callout_path)
    {
        if (CFGetTypeID(callout_path) == CFStringGetTypeID())
        {
            device_path = cfstring_to_string(static_cast<CFStringRef>(callout_path));
        }
        CFRelease(callout_path);
    }
    return device_path;
}

std::string get_class_name(io_object_t& obj)
{
    std::string result;
    io_name_t class_name;
    kern_return_t kern_result;
    kern_result = IOObjectGetClass(obj, class_name);
    if (KERN_SUCCESS == kern_result)
    {
        result = class_name;
    }
    return result;
}

io_registry_entry_t get_parent_iousb_device(io_object_t& serial_port)
{
    io_object_t device = serial_port;
    io_registry_entry_t parent = 0;
    io_registry_entry_t result = 0;
    kern_return_t kern_result = KERN_FAILURE;
    std::string name = get_class_name(device);
    // Walk the IO Registry tree looking for this devices parent IOUSBDevice.
    while ("IOUSBDevice" != name)
    {
        kern_result = IORegistryEntryGetParentEntry(device, kIOServicePlane, &parent);
        if (KERN_SUCCESS != kern_result)
        {
            result = 0;
            break;
        }
        device = parent;
        name = get_class_name(device);
    }
    if (KERN_SUCCESS == kern_result)
    {
        result = device;
    }
    return result;
}

std::string get_string_property(io_object_t& device, const char* property)
{
    std::string property_name;
    if (device)
    {
        CFStringRef property_as_cfstring = CFStringCreateWithCString(kCFAllocatorDefault, property, kCFStringEncodingASCII);
        CFTypeRef name_as_cfstring = IORegistryEntryCreateCFProperty(device, property_as_cfstring, kCFAllocatorDefault, 0);
        if (name_as_cfstring)
        {
            if (CFGetTypeID(name_as_cfstring) == CFStringGetTypeID())
            {
                property_name = cfstring_to_string(static_cast<CFStringRef>(name_as_cfstring));
            }
            CFRelease(name_as_cfstring);
        }
        if (property_as_cfstring)
        {
            CFRelease(property_as_cfstring);
        }
    }
    return property_name;
}

unsigned short get_int_property(io_object_t& device, const char* property)
{
    unsigned short result = 0;
    if (device)
    {
        CFStringRef property_as_cfstring = CFStringCreateWithCString(kCFAllocatorDefault, property, kCFStringEncodingASCII);
        CFTypeRef number = IORegistryEntryCreateCFProperty(device, property_as_cfstring, kCFAllocatorDefault, 0);
        if (property_as_cfstring)
        {
            CFRelease(property_as_cfstring);
        }
        if (number)
        {
            if (CFGetTypeID(number) == CFNumberGetTypeID())
            {
                bool success = CFNumberGetValue(static_cast<CFNumberRef>(number), kCFNumberSInt16Type, &result);
                if (!success)
                {
                    result = 0;
                }
            }
            CFRelease(number);
        }
    }
    return result;
}

std::string rtrim(const std::string& str)
{
    std::string result = str;
    std::string whitespace = " \t\f\v\n\r";
    std::size_t found = result.find_last_not_of(whitespace);
    if (std::std::string::npos == found)
    {
        result.clear();
    }
    else
    {
        result.erase(found + 1);
    }
    return result;
}

std::vector<PortInfo> getAllPorts()
{
    std::vector<PortInfo> devices_found;
    CFMutableDictionaryRef classes_to_match;
    io_iterator_t serial_port_iterator;
    io_object_t serial_port;
    mach_port_t master_port;
    kern_return_t kern_result;
    kern_result = IOMasterPort(MACH_PORT_NULL, &master_port);
    if (KERN_SUCCESS != kern_result)
    {
        return devices_found;
    }
    classes_to_match = IOServiceMatching(kIOSerialBSDServiceValue);
    if (!classes_to_match)
    {
        return devices_found;
    }
    CFDictionarySetValue(classes_to_match, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
    kern_result = IOServiceGetMatchingServices(master_port, classes_to_match, &serial_port_iterator);
    if (KERN_SUCCESS != kern_result)
    {
        return devices_found;
    }
    while ((serial_port = IOIteratorNext(serial_port_iterator)))
    {
        std::string device_path = get_device_path(serial_port);
        io_registry_entry_t parent = get_parent_iousb_device(serial_port);
        IOObjectRelease(serial_port);
        if (device_path.empty())
        {
            continue;
        }
        PortInfo port_info;
        port_info.port = device_path;
        port_info.description = "n/a";
        port_info.hardware_id = "n/a";
        std::string device_name = rtrim(get_string_property(parent, "USB Product Name"));
        std::string vendor_name = rtrim(get_string_property(parent, "USB Vendor Name"));
        std::string description = rtrim(vendor_name + " " + device_name);
        if (!description.empty())
        {
            port_info.description = description;
        }
        std::string serial_number = rtrim(get_string_property(parent, "USB Serial Number"));
        unsigned short vendor_id = get_int_property(parent, "idVendor");
        unsigned short product_id = get_int_property(parent, "idProduct");
        if (vendor_id && product_id)
        {
            char cstring[HARDWARE_ID_STRING_LENGTH];
            if (serial_number.empty())
            {
                serial_number = "None";
            }
            int ret =
                snprintf(cstring, HARDWARE_ID_STRING_LENGTH, "USB VID:PID=%04x:%04x SNR=%s", vendor_id, product_id, serial_number.c_str());
            if ((ret >= 0) && (ret < HARDWARE_ID_STRING_LENGTH))
            {
                port_info.hardware_id = cstring;
            }
        }
        devices_found.push_back(port_info);
    }
    IOObjectRelease(serial_port_iterator);
    return devices_found;
}
} // namespace serial

#endif
