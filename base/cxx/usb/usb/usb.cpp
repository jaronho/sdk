#include "usb.h"

#include <algorithm>
#include <codecvt>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string.h>

#ifdef _WIN32
// initguid.h必须在devpkey.h前面包含
#include <initguid.h>
//
#include <Dbt.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <devpkey.h>
#include <strsafe.h>
#include <usb.h>
#include <usbioctl.h>
#include <winioctl.h>
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")
#else
#include <libudev.h>
#endif

namespace usb
{
#ifdef _WIN32
struct LogicalDrive
{
    std::string driver;
    UINT driveType = 0;
    std::string fstype;
    std::string label;
    DEVICE_TYPE deviceType = 0;
    DWORD deviceNumber = 0;
    std::string serial;
    std::string model;
    std::string vendor;
};
#endif

struct DriverInfo
{
    DriverInfo() = default;
    DriverInfo(const std::string& driver, const std::string& fstype = "", const std::string& label = "")
        : driver(driver), fstype(fstype), label(label)
    {
    }

    std::string driver; /* 设备驱动器, 例如: C:\, D:\ */
    std::string fstype; /* 文件系统类型, 如果是存储设备则值为: FAT32, exfat, NTFS等 */
    std::string label; /* 文件系统标签, 例如: "Jim's U-DISK" */
};

struct ComPortInfo
{
    std::string parentInstanceId; /* 父设备实例ID, 例如: USB\\VID_10C4&PID_EA71\\C264F133A8CECCBF11EA40C3F37ED86 */
    std::string portName; /* 串口名, 如: COM3 */
    std::string friendlyName; /* 友好名, 如: USB Serial Port (COM3) */
};

struct UsbImpl
{
    int busNum = -1; /* 总线编号 */
    int portNum = -1; /* 端口编号 */
    int address = -1; /* 设备地址 */
    std::string vid; /* 厂商ID(小写字母) */
    std::string pid; /* 产品ID(小写字母) */
    std::string serial; /* 序列号 */
    std::string product; /* 产品名称 */
    std::string manufacturer; /* 厂商名称 */
    std::string model; /* 设备标识符(型号), 例如: "ELSKY_SSD_256GB", "CDRW_DVD_GCC4244", "DVD_A_DS8A5SH", "USB CARD READER " 等 */
    std::string vendor; /* 设备制造商, 例如: "FNK TECH", "HL-DT-ST", "Samsung " 等 */
    std::string storageType; /* 存储类型, 值: disk-磁盘(机械盘,SSD,U盘,NVMe,SD卡等), part-分区, cdrom-光驱(CD,DVD,BD,ISO loop设备等) */
#ifdef _WIN32
    std::string driverName; /* 设备驱动名称 */
    std::string deviceId; /* 设备ID */
    std::vector<DriverInfo> driverList; /* 存储设备驱动器列表 */
    std::vector<ComPortInfo> comList; /* 串口列表 */
#else
    DevNode blockDevRootNode; /* 存储设备根节点 */
    std::vector<DevNode> blockDevNodes; /* 存储设备节点 */
    std::vector<DevNode> nonblockDevNodes; /* 非存储设备节点 */
#endif
};

#ifdef _WIN32
std::string SafeArrayToString(SAFEARRAY* psa)
{
    BSTR bstr = NULL;
    if (psa)
    {
        if (psa->fFeatures & FADF_VARIANT) /* 检查SAFEARRAY的类型是否为VT_BSTR */
        {
            VARIANT* pVar;
            if (SUCCEEDED(SafeArrayAccessData(psa, (void HUGEP**)&pVar))) /* 需要处理VARIANT型SAFEARRAY */
            {
                bstr = SysAllocString(pVar->bstrVal); /* 复制BSTR */
                SafeArrayUnaccessData(psa);
            }
        }
        else if (1 == psa->cDims && psa->rgsabound[0].cElements > 0)
        {
            long index = 0; /* 确保SAFEARRAY是一维的并且至少有一个元素, 选择第一个元素 */
            VARIANT element;
            if (SUCCEEDED(SafeArrayGetElement(psa, &index, &element))) /* 锁定SAFEARRAY的数据, 以便复制 */
            {
                bstr = SysAllocString(element.bstrVal); /* 复制BSTR */
                VariantClear(&element); /* 清理VARIANT, SysAllocString会增加引用计数 */
            }
        }
    }
    if (bstr)
    {
        std::wstring wstr = bstr;
        SysFreeString(bstr);
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
    }
    return std::string();
}

/* 参考 UsbView 的实现: 
    https://www.uwe-sieber.de/usbtreeview_e.html
    https://github.com/microsoft/Windows-driver-samples
 */

#define USB_IAD_DESCRIPTOR_TYPE 0x0B
#define NUM_STRING_DESC_TO_GET 32

typedef struct _STRING_DESCRIPTOR_NODE
{
    struct _STRING_DESCRIPTOR_NODE* Next;
    UCHAR DescriptorIndex;
    USHORT LanguageID;
    USB_STRING_DESCRIPTOR StringDescriptor[1];
} STRING_DESCRIPTOR_NODE, *PSTRING_DESCRIPTOR_NODE;

typedef struct _USB_INTERFACE_DESCRIPTOR2
{
    UCHAR bLength; // offset 0, size 1
    UCHAR bDescriptorType; // offset 1, size 1
    UCHAR bInterfaceNumber; // offset 2, size 1
    UCHAR bAlternateSetting; // offset 3, size 1
    UCHAR bNumEndpoints; // offset 4, size 1
    UCHAR bInterfaceClass; // offset 5, size 1
    UCHAR bInterfaceSubClass; // offset 6, size 1
    UCHAR bInterfaceProtocol; // offset 7, size 1
    UCHAR iInterface; // offset 8, size 1
    USHORT wNumClasses; // offset 9, size 2
} USB_INTERFACE_DESCRIPTOR2, *PUSB_INTERFACE_DESCRIPTOR2;

typedef struct _USB_IAD_DESCRIPTOR
{
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bFirstInterface;
    UCHAR bInterfaceCount;
    UCHAR bFunctionClass;
    UCHAR bFunctionSubClass;
    UCHAR bFunctionProtocol;
    UCHAR iFunction;
} USB_IAD_DESCRIPTOR, *PUSB_IAD_DESCRIPTOR;

void enumerateHub(HDEVINFO devInfo, std::map<std::string, SP_DEVINFO_DATA> devInfoDataList, int rootIndex, std::string hubName,
                  PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfo, std::vector<LogicalDrive>& localDriveList,
                  const std::vector<ComPortInfo>& comPortList, std::vector<UsbImpl>& usbList);

std::string guid2string(GUID guid)
{
    std::wostringstream oss;
    oss << std::hex << std::uppercase << std::setfill(L'0'); /* GUID 的数据结构为：Data1, Data2, Data3, Data4[2], Data4[6] */
    oss << L"{";
    /* step1. 将 Data1 转换为字符串 */
    oss << std::setw(8) << guid.Data1;
    /* step2. 将 Data2 和 Data3 转换为字符串 */
    oss << '-' << std::setw(4) << guid.Data2;
    oss << '-' << std::setw(4) << guid.Data3;
    /* step3. 处理 Data4 前2个短整型 */
    oss << '-';
    for (int i = 0; i < 2; ++i)
    {
        oss << std::setw(2) << (unsigned short)guid.Data4[i];
    }
    /* step4. 处理 Data4 的其余6个字节 */
    oss << '-';
    for (int i = 2; i < 8; ++i)
    {
        oss << std::setw(2) << (unsigned char)guid.Data4[i];
    }
    oss << L"}";
    /* step5. 转为std::string */
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(oss.str());
}

std::string getRootHubName(HANDLE hostController)
{
    USB_ROOT_HUB_NAME rootHubName;
    ULONG nBytes = 0;
    if (!DeviceIoControl(hostController, IOCTL_USB_GET_ROOT_HUB_NAME, 0, 0, &rootHubName, sizeof(rootHubName), &nBytes, NULL))
    {
        return "";
    }
    nBytes = rootHubName.ActualLength;
    std::string rootHubNameA;
    PUSB_ROOT_HUB_NAME rootHubNameW = (PUSB_ROOT_HUB_NAME)GlobalAlloc(GPTR, nBytes);
    if (rootHubNameW)
    {
        if (DeviceIoControl(hostController, IOCTL_USB_GET_ROOT_HUB_NAME, NULL, 0, rootHubNameW, nBytes, &nBytes, NULL))
        {
            rootHubNameA = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(
                std::wstring(rootHubNameW->RootHubName, wcslen(rootHubNameW->RootHubName)));
        }
        GlobalFree(rootHubNameW);
    }
    return rootHubNameA;
}

std::string getExternalHubName(HANDLE hub, ULONG connectionIndex)
{
    USB_NODE_CONNECTION_NAME extHubName;
    extHubName.ConnectionIndex = connectionIndex;
    ULONG nBytes = 0;
    if (!DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_NAME, &extHubName, sizeof(extHubName), &extHubName, sizeof(extHubName), &nBytes,
                         NULL))
    {
        return "";
    }
    nBytes = extHubName.ActualLength;
    if (nBytes <= sizeof(extHubName))
    {
        return "";
    }
    std::string extHubNameA;
    PUSB_NODE_CONNECTION_NAME extHubNameW = (PUSB_NODE_CONNECTION_NAME)GlobalAlloc(GPTR, nBytes);
    if (extHubNameW)
    {
        extHubNameW->ConnectionIndex = connectionIndex;
        if (DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_NAME, extHubNameW, nBytes, extHubNameW, nBytes, &nBytes, NULL))
        {
            extHubNameA = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(
                std::wstring(extHubNameW->NodeName, wcslen(extHubNameW->NodeName)));
        }
        GlobalFree(extHubNameW);
    }
    return extHubNameA;
}

std::string getDriverKeyName(HANDLE hub, ULONG connectionIndex)
{
    USB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyName;
    driverKeyName.ConnectionIndex = connectionIndex;
    ULONG nBytes = 0;
    if (!DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME, &driverKeyName, sizeof(driverKeyName), &driverKeyName,
                         sizeof(driverKeyName), &nBytes, NULL))
    {
        return "";
    }
    nBytes = driverKeyName.ActualLength;
    if (nBytes <= sizeof(driverKeyName))
    {
        return "";
    }
    std::string driverKeyNameA;
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME)GlobalAlloc(GPTR, nBytes);
    if (driverKeyNameW)
    {
        driverKeyNameW->ConnectionIndex = connectionIndex;
        if (DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME, driverKeyNameW, nBytes, driverKeyNameW, nBytes, &nBytes,
                            NULL))
        {
            driverKeyNameA = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(
                std::wstring(driverKeyNameW->DriverKeyName, wcslen(driverKeyNameW->DriverKeyName)));
        }
        GlobalFree(driverKeyNameW);
    }
    return driverKeyNameA;
}

std::string getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devInfoData, DWORD property)
{
    CHAR buffer[1024] = {0};
    DWORD requiredSize;
    if (FALSE == SetupDiGetDeviceRegistryPropertyA(devInfo, devInfoData, property, NULL, (PBYTE)buffer, sizeof(buffer), &requiredSize))
    {
        return std::string();
    }
    return buffer;
}

PUSB_DESCRIPTOR_REQUEST getConfigDescriptor(HANDLE hHubDevice, ULONG connectionIndex, UCHAR descriptorIndex)
{
    UCHAR configDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) + sizeof(USB_CONFIGURATION_DESCRIPTOR)];
    PUSB_DESCRIPTOR_REQUEST configDescReq = (PUSB_DESCRIPTOR_REQUEST)configDescReqBuf;
    ULONG nBytes = sizeof(configDescReqBuf);
    memset(configDescReq, 0, nBytes);
    configDescReq->ConnectionIndex = connectionIndex;
    configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8) | descriptorIndex;
    configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));
    ULONG nBytesReturned = 0;
    if (!DeviceIoControl(hHubDevice, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, configDescReq, nBytes, configDescReq, nBytes,
                         &nBytesReturned, NULL))
    {
        return NULL;
    }
    if (nBytes != nBytesReturned)
    {
        return NULL;
    }
    PUSB_CONFIGURATION_DESCRIPTOR configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq + 1);
    if (configDesc->wTotalLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        return NULL;
    }
    nBytes = sizeof(USB_DESCRIPTOR_REQUEST) + configDesc->wTotalLength;
    configDescReq = (PUSB_DESCRIPTOR_REQUEST)GlobalAlloc(GPTR, nBytes);
    if (NULL == configDescReq)
    {
        return NULL;
    }
    configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq + 1);
    configDescReq->ConnectionIndex = connectionIndex;
    configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8) | descriptorIndex;
    configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));
    if (!DeviceIoControl(hHubDevice, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, configDescReq, nBytes, configDescReq, nBytes,
                         &nBytesReturned, NULL))
    {
        GlobalFree(configDescReq);
        return NULL;
    }
    if (nBytes != nBytesReturned)
    {
        GlobalFree(configDescReq);
        return NULL;
    }
    if (configDesc->wTotalLength != (nBytes - sizeof(USB_DESCRIPTOR_REQUEST)))
    {
        GlobalFree(configDescReq);
        return NULL;
    }
    return configDescReq;
}

BOOL areThereStringDescriptors(PUSB_DEVICE_DESCRIPTOR deviceDesc, PUSB_CONFIGURATION_DESCRIPTOR configDesc)
{
    if (deviceDesc->iManufacturer || deviceDesc->iProduct || deviceDesc->iSerialNumber)
    {
        return TRUE;
    }
    PUCHAR descEnd = (PUCHAR)configDesc + configDesc->wTotalLength;
    PUSB_COMMON_DESCRIPTOR commonDesc = (PUSB_COMMON_DESCRIPTOR)configDesc;
    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd && (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        switch (commonDesc->bDescriptorType)
        {
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
        case USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR_TYPE:
            if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
            {
                break;
            }
            if (((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
            {
                return TRUE;
            }
            commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
            continue;
        case USB_INTERFACE_DESCRIPTOR_TYPE:
            if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) && commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
            {
                break;
            }
            if (((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
            {
                return TRUE;
            }
            commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
            continue;
        default:
            commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
            continue;
        }
        break;
    }
    return FALSE;
}

PSTRING_DESCRIPTOR_NODE getStringDescriptor(HANDLE hHubDevice, ULONG connectionIndex, UCHAR descriptorIndex, USHORT languageID)
{
    UCHAR stringDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) + MAXIMUM_USB_STRING_LENGTH];
    PUSB_DESCRIPTOR_REQUEST stringDescReq = (PUSB_DESCRIPTOR_REQUEST)stringDescReqBuf;
    ULONG nBytes = sizeof(stringDescReqBuf);
    memset(stringDescReq, 0, nBytes);
    stringDescReq->ConnectionIndex = connectionIndex;
    stringDescReq->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8) | descriptorIndex;
    stringDescReq->SetupPacket.wIndex = languageID;
    stringDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));
    ULONG nBytesReturned = 0;
    if (!DeviceIoControl(hHubDevice, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, stringDescReq, nBytes, stringDescReq, nBytes,
                         &nBytesReturned, NULL))
    {
        return NULL;
    }
    if (nBytesReturned < 2)
    {
        return NULL;
    }
    PUSB_STRING_DESCRIPTOR stringDesc = (PUSB_STRING_DESCRIPTOR)(stringDescReq + 1);
    if (USB_STRING_DESCRIPTOR_TYPE != stringDesc->bDescriptorType)
    {
        return NULL;
    }
    if (stringDesc->bLength != nBytesReturned - sizeof(USB_DESCRIPTOR_REQUEST))
    {
        return NULL;
    }
    if (0 != stringDesc->bLength % 2)
    {
        return NULL;
    }
    PSTRING_DESCRIPTOR_NODE stringDescNode =
        (PSTRING_DESCRIPTOR_NODE)GlobalAlloc(GPTR, sizeof(STRING_DESCRIPTOR_NODE) + stringDesc->bLength);
    if (NULL == stringDescNode)
    {
        return NULL;
    }
    stringDescNode->DescriptorIndex = descriptorIndex;
    stringDescNode->LanguageID = languageID;
    memcpy(stringDescNode->StringDescriptor, stringDesc, stringDesc->bLength);
    return stringDescNode;
}

HRESULT getStringDescriptors(HANDLE hHubDevice, ULONG connectionIndex, UCHAR descriptorIndex, ULONG numLanguageIDs, USHORT* languageIDs,
                             PSTRING_DESCRIPTOR_NODE StringDescNodeHead)
{
    PSTRING_DESCRIPTOR_NODE tail = NULL;
    PSTRING_DESCRIPTOR_NODE trailing = NULL;
    for (tail = StringDescNodeHead; tail != NULL; tail = tail->Next)
    {
        if (tail->DescriptorIndex == descriptorIndex)
        {
            return S_OK;
        }
        trailing = tail;
    }
    tail = trailing;
    for (ULONG i = 0; (tail != NULL) && (i < numLanguageIDs); i++)
    {
        if (tail->Next)
        {
            GlobalFree(tail->Next);
        }
        tail->Next = getStringDescriptor(hHubDevice, connectionIndex, descriptorIndex, languageIDs[i]);
        tail = tail->Next;
    }
    if (tail == NULL)
    {
        return E_FAIL;
    }
    return S_OK;
}

PSTRING_DESCRIPTOR_NODE getAllStringDescriptors(HANDLE hHubDevice, ULONG connectionIndex, PUSB_DEVICE_DESCRIPTOR deviceDesc,
                                                PUSB_CONFIGURATION_DESCRIPTOR configDesc)
{
    PSTRING_DESCRIPTOR_NODE supportedLanguagesString = getStringDescriptor(hHubDevice, connectionIndex, 0, 0);
    if (supportedLanguagesString == NULL)
    {
        return NULL;
    }
    ULONG numLanguageIDs = (supportedLanguagesString->StringDescriptor->bLength - 2) / 2;
    USHORT* languageIDs = (USHORT*)(&supportedLanguagesString->StringDescriptor->bString[0]);
    if (deviceDesc->iManufacturer)
    {
        getStringDescriptors(hHubDevice, connectionIndex, deviceDesc->iManufacturer, numLanguageIDs, languageIDs, supportedLanguagesString);
    }
    if (deviceDesc->iProduct)
    {
        getStringDescriptors(hHubDevice, connectionIndex, deviceDesc->iProduct, numLanguageIDs, languageIDs, supportedLanguagesString);
    }
    if (deviceDesc->iSerialNumber)
    {
        getStringDescriptors(hHubDevice, connectionIndex, deviceDesc->iSerialNumber, numLanguageIDs, languageIDs, supportedLanguagesString);
    }
    PUCHAR descEnd = (PUCHAR)configDesc + configDesc->wTotalLength;
    PUSB_COMMON_DESCRIPTOR commonDesc = (PUSB_COMMON_DESCRIPTOR)configDesc;
    BOOL getMoreStrings = FALSE;
    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd && (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        switch (commonDesc->bDescriptorType)
        {
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
            {
                break;
            }
            if (((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
            {
                getStringDescriptors(hHubDevice, connectionIndex, ((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration,
                                     numLanguageIDs, languageIDs, supportedLanguagesString);
            }
            commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
            continue;
        case USB_IAD_DESCRIPTOR_TYPE:
            if (commonDesc->bLength < sizeof(USB_IAD_DESCRIPTOR))
            {
                break;
            }
            if (((PUSB_IAD_DESCRIPTOR)commonDesc)->iFunction)
            {
                getStringDescriptors(hHubDevice, connectionIndex, ((PUSB_IAD_DESCRIPTOR)commonDesc)->iFunction, numLanguageIDs, languageIDs,
                                     supportedLanguagesString);
            }
            commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
            continue;
        case USB_INTERFACE_DESCRIPTOR_TYPE:
            if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) && commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
            {
                break;
            }
            if (((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
            {
                getStringDescriptors(hHubDevice, connectionIndex, ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface, numLanguageIDs,
                                     languageIDs, supportedLanguagesString);
            }
            if (USB_DEVICE_CLASS_VIDEO == ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->bInterfaceClass)
            {
                getMoreStrings = TRUE;
            }
            commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
            continue;
        default:
            commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
            continue;
        }
        break;
    }
    if (getMoreStrings)
    {
        for (UCHAR uIndex = 1; (uIndex < NUM_STRING_DESC_TO_GET); uIndex++)
        {
            HRESULT hr = getStringDescriptors(hHubDevice, connectionIndex, uIndex, numLanguageIDs, languageIDs, supportedLanguagesString);
            if (FAILED(hr))
            {
                break;
            }
        }
    }
    return supportedLanguagesString;
}

std::string getDisplayString(UCHAR index, PSTRING_DESCRIPTOR_NODE stringDescs)
{
    std::string desc;
    char pString[512];
    while (stringDescs)
    {
        if (stringDescs->DescriptorIndex == index)
        {
            memset(pString, 0, 512);
            if (stringDescs->StringDescriptor->bLength > sizeof(USHORT))
            {
                ULONG nBytes = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, stringDescs->StringDescriptor->bString,
                                                   (stringDescs->StringDescriptor->bLength - 2) / 2, pString, 512, NULL, NULL);
                if (nBytes)
                {
                    desc += pString;
                }
            }
        }
        stringDescs = stringDescs->Next;
    }
    return desc;
}

void parseBusRelations(const std::string& buffer, std::string& model, std::string& vendor, std::string& storageType,
                       std::string& devicePath)
{
    model.clear();
    vendor.clear();
    storageType.clear();
    devicePath.clear();
    auto pos = buffer.find("Prod_");
    if (std::string::npos != pos)
    {
        for (size_t i = pos + 5; i < buffer.size(); ++i)
        {
            if ('&' == buffer[i])
            {
                break;
            }
            model.push_back('_' == buffer[i] ? ' ' : buffer[i]);
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
            vendor.push_back('_' == buffer[i] ? ' ' : buffer[i]);
        }
    }
    pos = buffer.find("USBSTOR\\");
    size_t offset = 8;
    if (std::string::npos == pos)
    {
        pos = buffer.find("SCSI\\");
        offset = 5;
    }
    if (std::string::npos != pos)
    {
        for (size_t i = pos + offset; i < buffer.size(); ++i)
        {
            if ('&' == buffer[i])
            {
                break;
            }
            storageType.push_back(buffer[i]);
        }
        std::transform(storageType.begin(), storageType.end(), storageType.begin(), tolower);
    }
    std::string interfaceClassGUID;
    if ("disk" == storageType) /* 磁盘 */
    {
        interfaceClassGUID = guid2string(GUID_DEVINTERFACE_DISK);
    }
    else if ("cdrom" == storageType) /* 光驱 */
    {
        interfaceClassGUID = guid2string(GUID_DEVINTERFACE_CDROM);
    }
    if (interfaceClassGUID.empty())
    {
        pos = buffer.find("USB\\");
        auto miPos = buffer.find("MI_");
        if (std::string::npos != pos && std::string::npos != miPos) /* 检查是否是CDC设备(通常包含MI_接口号) */
        {
            devicePath = buffer;
            for (size_t i = 0; i < devicePath.size(); ++i)
            {
                if ('\\' == devicePath[i])
                {
                    devicePath[i] = '#';
                }
            }
            devicePath = "\\\\?\\" + devicePath + "#" + guid2string(GUID_DEVINTERFACE_COMPORT);
            std::transform(devicePath.begin(), devicePath.end(), devicePath.begin(), tolower);
        }
    }
    else
    {
        devicePath = buffer;
        for (size_t i = 0; i < devicePath.size(); ++i)
        {
            if ('\\' == devicePath[i])
            {
                devicePath[i] = '#';
            }
        }
        devicePath = "\\\\?\\" + devicePath + "#" + interfaceClassGUID;
        std::transform(devicePath.begin(), devicePath.end(), devicePath.begin(), tolower);
    }
}

void parseStorageDriver(const std::string& devicePath, std::vector<LogicalDrive>& localDriveList, std::vector<DriverInfo>& driverList)
{
    driverList.clear();
    int deviceType = -1, deviceNumber = -1;
    if (!devicePath.empty())
    {
        auto handle = CreateFileA(devicePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                                  FILE_FLAG_OVERLAPPED, NULL);
        if (INVALID_HANDLE_VALUE != handle)
        {
            DWORD nBytes = 0;
            STORAGE_DEVICE_NUMBER deviceNum;
            if (DeviceIoControl(handle, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &deviceNum, sizeof(deviceNum), &nBytes, NULL))
            {
                deviceType = deviceNum.DeviceType;
                deviceNumber = deviceNum.DeviceNumber;
            }
            CloseHandle(handle);
        }
    }
    auto iter = std::find_if(localDriveList.begin(), localDriveList.end(), [&](const LogicalDrive& item) {
        return (item.deviceType == deviceType && item.deviceNumber == deviceNumber);
    });
    if (localDriveList.end() != iter)
    {
        driverList.emplace_back(DriverInfo(iter->driver, iter->fstype, iter->label));
        localDriveList.erase(iter);
    }
}

std::vector<LogicalDrive> getLogicalDriveList()
{
    std::vector<LogicalDrive> localDriveList;
    DWORD drives = GetLogicalDrives();
    int index = 0;
    while (drives)
    {
        if (1 == (drives & 0x1))
        {
            char driverChar = 'A' + index;
            char drivePath[10] = {0};
            sprintf_s(drivePath, "\\\\.\\%c:", driverChar);
            auto handle =
                CreateFileA(drivePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
            if (INVALID_HANDLE_VALUE != handle)
            {
                std::string driver = std::string(1, driverChar) + ":\\";
                const int bufferSize = 256;
                CHAR volumeName[bufferSize] = {0};
                DWORD volumeSerialNumber = 0;
                DWORD maximumComponentLength = 0;
                DWORD fileSystemFlags = 0;
                CHAR fileSystemName[bufferSize] = {0};
                GetVolumeInformationA(driver.c_str(), volumeName, bufferSize, &volumeSerialNumber, &maximumComponentLength,
                                      &fileSystemFlags, fileSystemName, bufferSize);
                LogicalDrive info;
                info.driver = driver;
                info.driveType = GetDriveTypeA(driver.c_str());
                info.fstype = fileSystemName;
                info.label = volumeName;
                DWORD nBytes = 0;
                STORAGE_DEVICE_NUMBER devNum;
                if (DeviceIoControl(handle, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &devNum, sizeof(devNum), &nBytes, NULL))
                {
                    info.deviceType = devNum.DeviceType;
                    info.deviceNumber = devNum.DeviceNumber;
                }
                STORAGE_PROPERTY_QUERY query;
                ZeroMemory(&query, sizeof(query));
                query.PropertyId = StorageDeviceProperty;
                query.QueryType = PropertyStandardQuery;
                CHAR buffer[sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1] = {0};
                STORAGE_DEVICE_DESCRIPTOR* pDevDesc = (STORAGE_DEVICE_DESCRIPTOR*)buffer;
                pDevDesc->Size = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;
                if (DeviceIoControl(handle, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), pDevDesc, pDevDesc->Size, &nBytes, NULL))
                {
                    info.serial = &buffer[pDevDesc->SerialNumberOffset];
                    if (std::all_of(info.serial.begin(), info.serial.end(), isspace)) /* 全部是空格 */
                    {
                        info.serial.clear();
                    }
                    info.model = &buffer[pDevDesc->ProductIdOffset];
                    info.vendor = &buffer[pDevDesc->VendorIdOffset];
                }
                localDriveList.emplace_back(info);
                CloseHandle(handle);
            }
        }
        drives = drives >> 1;
        ++index;
    }
    return localDriveList;
}

void enumerateHubPorts(HDEVINFO devInfo, std::map<std::string, SP_DEVINFO_DATA> devInfoDataList, int rootIndex, HANDLE hHubDevice,
                       ULONG numPorts, std::vector<LogicalDrive>& localDriveList, const std::vector<ComPortInfo>& comPortList,
                       std::vector<UsbImpl>& usbList)
{
    bool recheckDriverFlag = false;
    for (ULONG index = 1; index <= numPorts; ++index)
    {
        ULONG nBytesEx = sizeof(USB_NODE_CONNECTION_INFORMATION_EX) + (sizeof(USB_PIPE_INFO) * 30);
        PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)GlobalAlloc(GPTR, nBytesEx);
        if (NULL == connectionInfoEx)
        {
            break;
        }
        connectionInfoEx->ConnectionIndex = index;
        if (!DeviceIoControl(hHubDevice, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX, connectionInfoEx, nBytesEx, connectionInfoEx,
                             nBytesEx, &nBytesEx, NULL))
        {
            ULONG nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) + sizeof(USB_PIPE_INFO) * 30;
            PUSB_NODE_CONNECTION_INFORMATION connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)GlobalAlloc(GPTR, nBytes);
            if (connectionInfo)
            {
                connectionInfo->ConnectionIndex = index;
                if (DeviceIoControl(hHubDevice, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION, connectionInfo, nBytes, connectionInfo, nBytes,
                                    &nBytes, NULL))
                {
                    memcpy(connectionInfoEx, connectionInfo, sizeof(USB_NODE_CONNECTION_INFORMATION));
                }
                GlobalFree(connectionInfo);
            }
        }
        PUSB_DESCRIPTOR_REQUEST configDesc = NULL;
        if (DeviceConnected == connectionInfoEx->ConnectionStatus)
        {
            configDesc = getConfigDescriptor(hHubDevice, index, 0);
        }
        PSTRING_DESCRIPTOR_NODE stringDescs = NULL;
        if (configDesc && areThereStringDescriptors(&connectionInfoEx->DeviceDescriptor, (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc + 1)))
        {
            stringDescs = getAllStringDescriptors(hHubDevice, index, &connectionInfoEx->DeviceDescriptor,
                                                  (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc + 1));
        }
        if (DeviceConnected == connectionInfoEx->ConnectionStatus) /* Port Plugin Device */
        {
            char vid[16] = {0};
            sprintf(vid, "%04x", connectionInfoEx->DeviceDescriptor.idVendor);
            char pid[16] = {0};
            sprintf(pid, "%04x", connectionInfoEx->DeviceDescriptor.idProduct);
            std::string serial;
            if (connectionInfoEx->DeviceDescriptor.iSerialNumber)
            {
                serial = getDisplayString(connectionInfoEx->DeviceDescriptor.iSerialNumber, stringDescs);
            }
            std::string product;
            if (connectionInfoEx->DeviceDescriptor.iProduct)
            {
                product = getDisplayString(connectionInfoEx->DeviceDescriptor.iProduct, stringDescs);
            }
            std::string manufacturer;
            if (connectionInfoEx->DeviceDescriptor.iManufacturer)
            {
                manufacturer = getDisplayString(connectionInfoEx->DeviceDescriptor.iManufacturer, stringDescs);
            }
            std::string driverName = getDriverKeyName(hHubDevice, index);
            std::string deviceId, model, vendor, storageType, devicePath;
            std::vector<DriverInfo> driverList;
            std::vector<ComPortInfo> comList;
            auto iter = devInfoDataList.find(driverName);
            if (devInfoDataList.end() != iter)
            {
                CHAR instanceId[MAX_DEVICE_ID_LEN];
                if (CR_SUCCESS == CM_Get_Device_ID(iter->second.DevInst, instanceId, MAX_PATH, 0))
                {
                    if (serial.empty())
                    {
                        std::string str(instanceId);
                        auto slashPos = str.rfind('\\');
                        if (std::string::npos != slashPos)
                        {
                            str = str.substr(slashPos + 1);
                            if (std::string::npos == str.find('&'))
                            {
                                serial = str;
                            }
                        }
                    }
                }
                DEVPROPTYPE propertyType = 0;
                WCHAR propertyBuffer[1024] = {0};
                DWORD requiredSize = 0;
                if (SetupDiGetDevicePropertyW(devInfo, &(iter->second), &DEVPKEY_Device_BusRelations, &propertyType,
                                              reinterpret_cast<PBYTE>(propertyBuffer), sizeof(propertyBuffer), &requiredSize, 0))
                {
                    deviceId = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(propertyBuffer);
                    parseBusRelations(deviceId, model, vendor, storageType, devicePath);
                    if (!devicePath.empty())
                    {
                        if (storageType.empty()) /* 非存储设备, 尝试查找是否串口 */
                        {
                            std::string key = instanceId;
                            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                            for (const auto& info : comPortList)
                            {
                                if (info.parentInstanceId == key)
                                {
                                    comList.emplace_back(info);
                                }
                            }
                        }
                        else /* 存储设备 */
                        {
                            parseStorageDriver(devicePath, localDriveList, driverList);
                            if (driverList.empty())
                            {
                                recheckDriverFlag = true;
                            }
                        }
                    }
                }
            }
            UsbImpl info;
            info.busNum = rootIndex;
            info.portNum = index;
            info.address = connectionInfoEx->DeviceAddress;
            info.vid = vid;
            info.pid = pid;
            info.serial = serial;
            info.product = product;
            info.manufacturer = manufacturer;
            info.driverName = driverName;
            info.deviceId = deviceId;
            info.model = model;
            info.vendor = vendor;
            info.storageType = storageType;
            info.driverList = driverList;
            info.comList = comList;
            usbList.emplace_back(info);
        }
        if (connectionInfoEx->DeviceIsHub) /* Hub Device */
        {
            enumerateHub(devInfo, devInfoDataList, rootIndex, getExternalHubName(hHubDevice, index), connectionInfoEx, localDriveList,
                         comPortList, usbList);
        }
        if (configDesc)
        {
            GlobalFree(configDesc);
        }
        while (stringDescs)
        {
            PSTRING_DESCRIPTOR_NODE Next = stringDescs->Next;
            GlobalFree(stringDescs);
            stringDescs = Next;
        }
        GlobalFree(connectionInfoEx);
    }
    if (recheckDriverFlag) /* 再次检测驱动器 */
    {
        for (size_t i = 0; i < usbList.size(); ++i)
        {
            if (("disk" == usbList[i].storageType || "cdrom" == usbList[i].storageType) && usbList[i].driverList.empty())
            {
                auto iter = std::find_if(localDriveList.begin(), localDriveList.end(), [&](const LogicalDrive& item) {
                    return ((DRIVE_REMOVABLE == item.driveType || DRIVE_CDROM == item.driveType) && item.serial == usbList[i].serial
                            && item.model == usbList[i].model && item.vendor == usbList[i].vendor);
                });
                if (localDriveList.end() != iter)
                {
                    usbList[i].driverList.emplace_back(DriverInfo(iter->driver, iter->fstype, iter->label));
                    localDriveList.erase(iter);
                }
            }
        }
    }
}

void enumerateHub(HDEVINFO devInfo, std::map<std::string, SP_DEVINFO_DATA> devInfoDataList, int rootIndex, std::string hubName,
                  PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfo, std::vector<LogicalDrive>& localDriveList,
                  const std::vector<ComPortInfo>& comPortList, std::vector<UsbImpl>& usbList)
{
    if (hubName.empty())
    {
        return;
    }
    PUSB_NODE_INFORMATION hubInfo = (PUSB_NODE_INFORMATION)GlobalAlloc(GPTR, sizeof(USB_NODE_INFORMATION));
    if (NULL == hubInfo)
    {
        return;
    }
    std::string deviceName = "\\\\.\\" + hubName;
    HANDLE hHubDevice =
        CreateFileA(deviceName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (INVALID_HANDLE_VALUE == hHubDevice)
    {
        GlobalFree(hubInfo);
        return;
    }
    ULONG nBytes = 0;
    if (!DeviceIoControl(hHubDevice, IOCTL_USB_GET_NODE_INFORMATION, hubInfo, sizeof(USB_NODE_INFORMATION), hubInfo,
                         sizeof(USB_NODE_INFORMATION), &nBytes, NULL))
    {
        CloseHandle(hHubDevice);
        GlobalFree(hubInfo);
        return;
    }
    if (NULL == connectionInfo) /* Root Hub */
    {
    }
    else /* Extend Hub */
    {
    }
    enumerateHubPorts(devInfo, devInfoDataList, rootIndex, hHubDevice, hubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts,
                      localDriveList, comPortList, usbList);
    CloseHandle(hHubDevice);
    GlobalFree(hubInfo);
}

std::vector<ComPortInfo> getComPortList(HDEVINFO hDevInfo)
{
    std::vector<ComPortInfo> comPortList;
    if (INVALID_HANDLE_VALUE == hDevInfo)
    {
        return comPortList;
    }
    SP_DEVICE_INTERFACE_DATA interfaceData = {sizeof(SP_DEVICE_INTERFACE_DATA)};
    DWORD index = 0;
    while (SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_COMPORT, index++, &interfaceData))
    {
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetailA(hDevInfo, &interfaceData, NULL, 0, &requiredSize, NULL);
        PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, requiredSize);
        if (!detailData)
        {
            continue;
        }
        detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        SP_DEVINFO_DATA devInfoData = {sizeof(SP_DEVINFO_DATA)};
        if (SetupDiGetDeviceInterfaceDetailA(hDevInfo, &interfaceData, detailData, requiredSize, &requiredSize, &devInfoData))
        {
            DEVINST parentDevInst;
            CHAR parentInstanceId[MAX_DEVICE_ID_LEN];
            if (CR_SUCCESS == CM_Get_Parent(&parentDevInst, devInfoData.DevInst, 0)
                && CR_SUCCESS == CM_Get_Device_ID(parentDevInst, parentInstanceId, MAX_PATH, 0))
            {
                HKEY hKey = SetupDiOpenDevRegKey(hDevInfo, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
                if (hKey != INVALID_HANDLE_VALUE)
                {
                    char portName[64] = {0};
                    DWORD size = sizeof(portName);
                    if (ERROR_SUCCESS == RegQueryValueExA(hKey, "PortName", NULL, NULL, (LPBYTE)portName, &size))
                    {
                        std::string key = parentInstanceId;
                        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                        ComPortInfo info;
                        info.parentInstanceId = key;
                        info.portName = portName;
                        char friendlyName[256] = {0};
                        if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)friendlyName,
                                                              sizeof(friendlyName), &requiredSize))
                        {
                            info.friendlyName = friendlyName;
                        }
                        comPortList.emplace_back(info);
                    }
                    RegCloseKey(hKey);
                }
            }
        }
        GlobalFree(detailData);
    }
    return comPortList;
}

std::vector<UsbImpl> enumerateHostControllers()
{
    std::vector<UsbImpl> usbList;
    HDEVINFO devInfo = SetupDiGetClassDevsA(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (NULL == devInfo)
    {
        return usbList;
    }
    /* 收集所有控制器信息 */
    struct ControllerInfo
    {
        std::string deviceInstanceId; /* 设备实例ID, 用于排序 */
        std::string rootHubName; /* Root Hub名称 */
        SP_DEVINFO_DATA devInfoData; /* 设备信息数据 */
    };
    std::vector<ControllerInfo> controllers;
    /* 枚举所有USB主机控制器 */
    SP_DEVICE_INTERFACE_DATA devInterfaceData;
    devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    for (ULONG index = 0; SetupDiEnumDeviceInterfaces(devInfo, 0, (LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER, index, &devInterfaceData);
         ++index)
    {
        ULONG requiredSize = 0;
        SetupDiGetDeviceInterfaceDetailA(devInfo, &devInterfaceData, NULL, 0, &requiredSize, NULL);
        PSP_DEVICE_INTERFACE_DETAIL_DATA devDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, requiredSize);
        if (devDetailData)
        {
            devDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            SP_DEVINFO_DATA devInfoData = {sizeof(SP_DEVINFO_DATA)};
            if (SetupDiGetDeviceInterfaceDetailA(devInfo, &devInterfaceData, devDetailData, requiredSize, &requiredSize, &devInfoData))
            {
                HANDLE hHCDev = CreateFileA(devDetailData->DevicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
                if (INVALID_HANDLE_VALUE != hHCDev)
                {
                    auto rootHubName = getRootHubName(hHCDev);
                    /* 获取设备实例ID（这是关键排序依据）*/
                    CHAR instanceId[MAX_DEVICE_ID_LEN];
                    if (SetupDiGetDeviceInstanceIdA(devInfo, &devInfoData, instanceId, sizeof(instanceId), NULL))
                    {
                        ControllerInfo ci;
                        ci.deviceInstanceId = instanceId;
                        ci.rootHubName = rootHubName;
                        ci.devInfoData = devInfoData;
                        controllers.emplace_back(ci);
                    }
                    CloseHandle(hHCDev);
                }
            }
            GlobalFree(devDetailData);
        }
    }
    /* 按设备实例ID排序, libusb在Windows上的枚举顺序是按设备路径/实例ID字母序 */
    std::sort(controllers.begin(), controllers.end(),
              [](const ControllerInfo& a, const ControllerInfo& b) { return a.deviceInstanceId < b.deviceInstanceId; });
    /* 遍历获取所有 SP_DEVINFO_DATA 信息 */
    std::map<std::string, SP_DEVINFO_DATA> devInfoDataList; /* key: driverName, value: 信息 */
    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(devInfoData);
    ULONG totalCount = 0;
    for (; SetupDiEnumDeviceInfo(devInfo, totalCount, &devInfoData); ++totalCount)
    {
        auto driverName = getDeviceProperty(devInfo, &devInfoData, SPDRP_DRIVER);
        if (!driverName.empty())
        {
            devInfoDataList[driverName] = devInfoData;
        }
    }
    /* 遍历获取USB设备详情 */
    auto localDriveList = getLogicalDriveList();
    auto comPortList = getComPortList(devInfo);
    /* 按校准后的顺序枚举, 使用排序后的索引+1作为busNum, 与libusb保持一致 */
    for (size_t i = 0; i < controllers.size(); ++i)
    {
        int busNum = static_cast<int>(i + 1); /* 校准后的busNum */
        enumerateHub(devInfo, devInfoDataList, busNum, controllers[i].rootHubName, NULL, localDriveList, comPortList, usbList);
    }
    SetupDiDestroyDeviceInfoList(devInfo);
    return usbList;
}
#else
static void runCommand(const std::string& cmd, std::string* outStr, std::vector<std::string>* outVec)
{
    if (outStr)
    {
        outStr->clear();
    }
    if (outVec)
    {
        outVec->clear();
    }
    FILE* stream = popen(cmd.c_str(), "r");
    if (stream)
    {
        if (outStr || outVec)
        {
            const size_t bufferSize = 1025;
            char buffer[bufferSize] = {0};
            std::string line;
            while (fread(buffer, 1, bufferSize - 1, stream) > 0)
            {
                if (outStr)
                {
                    outStr->append(buffer);
                }
                if (outVec)
                {
                    line += buffer;
                    while (1)
                    {
                        size_t pos = line.find("\r\n"), offset = 2;
                        if (std::string::npos == pos)
                        {
                            pos = line.find("\n"), offset = 1;
                        }
                        if (std::string::npos == pos)
                        {
                            break;
                        }
                        outVec->emplace_back(line.substr(0, pos));
                        line = line.substr(pos + offset, line.size() - pos - offset);
                    }
                }
                memset(buffer, 0, bufferSize);
            }
            if (outVec && !line.empty())
            {
                outVec->emplace_back(line);
            }
        }
        pclose(stream);
    }
}

std::vector<UsbImpl> enumerateUsbDevices()
{
    std::vector<UsbImpl> usbList;
    std::vector<std::string> outVec;
    runCommand("cat /sys/kernel/debug/usb/devices", nullptr, &outVec);
    UsbImpl info;
    for (const auto& line : outVec)
    {
        static const std::string BUS_FLAG = "Bus="; /* 总线号 */
        static const std::string PRNT_FLAG = "Prnt="; /* 父设备数量, 如: XHCI控制器是root, 位于最顶层, 其Prnt=0 */
        static const std::string PORT_FLAG = "Port="; /* 端口 */
        static const std::string ADDRESS_FLAG = "Dev#="; /* 设备编号 */
        static const std::string VID_FLAG = "Vendor="; /* VID */
        static const std::string PID_FLAG = "ProdID="; /* PID */
        static const std::string SERIAL_FLAG = "SerialNumber="; /* 序列号 */
        static const std::string PRODUCT_FLAG = "Product="; /* 产品名称 */
        static const std::string MANUFACTURER_FLAG = "Manufacturer="; /* 厂商名称 */
        auto busPos = line.find(BUS_FLAG);
        auto prntPos = line.find(PRNT_FLAG);
        auto portPos = line.find(PORT_FLAG);
        auto addressPos = line.find(ADDRESS_FLAG);
        if (std::string::npos != busPos && std::string::npos != prntPos && std::string::npos != portPos && std::string::npos != addressPos)
        {
            if (info.busNum >= 0 && info.portNum >= 0 && info.address >= 0)
            {
                usbList.emplace_back(info);
                info = UsbImpl();
            }
            std::string busTmp;
            for (size_t i = busPos + BUS_FLAG.size(); i < line.size(); ++i)
            {
                if (' ' != line[i])
                {
                    busTmp.push_back(line[i]);
                }
                else if (!busTmp.empty())
                {
                    break;
                }
            }
            std::string prntTmp;
            for (size_t i = prntPos + PRNT_FLAG.size(); i < line.size(); ++i)
            {
                if (' ' != line[i])
                {
                    prntTmp.push_back(line[i]);
                }
                else if (!prntTmp.empty())
                {
                    break;
                }
            }
            std::string portTmp;
            for (size_t i = portPos + PORT_FLAG.size(); i < line.size(); ++i)
            {
                if (' ' != line[i])
                {
                    portTmp.push_back(line[i]);
                }
                else if (!portTmp.empty())
                {
                    break;
                }
            }
            std::string addressTmp;
            for (size_t i = addressPos + ADDRESS_FLAG.size(); i < line.size(); ++i)
            {
                if (' ' != line[i])
                {
                    addressTmp.push_back(line[i]);
                }
                else if (!addressTmp.empty())
                {
                    break;
                }
            }
            try
            {
                info.busNum = std::atoi(busTmp.c_str());
                info.portNum = std::atoi(portTmp.c_str());
                if (std::atoi(prntTmp.c_str()) > 0) /* 非根HUB, 端口号需要加1 */
                {
                    info.portNum += 1;
                }
                info.address = std::atoi(addressTmp.c_str());
            }
            catch (...)
            {
            }
            continue;
        }
        auto vidPos = line.find(VID_FLAG);
        auto pidPos = line.find(PID_FLAG);
        if (std::string::npos != vidPos && std::string::npos != pidPos)
        {
            std::string vidTmp;
            for (size_t i = vidPos + VID_FLAG.size(); i < line.size(); ++i)
            {
                if (' ' != line[i])
                {
                    vidTmp.push_back(line[i]);
                }
                else if (!vidTmp.empty())
                {
                    break;
                }
            }
            std::transform(vidTmp.begin(), vidTmp.end(), vidTmp.begin(), tolower);
            std::string pidTmp;
            for (size_t i = pidPos + PID_FLAG.size(); i < line.size(); ++i)
            {
                if (' ' != line[i])
                {
                    pidTmp.push_back(line[i]);
                }
                else if (!pidTmp.empty())
                {
                    break;
                }
            }
            std::transform(pidTmp.begin(), pidTmp.end(), pidTmp.begin(), tolower);
            info.vid = vidTmp;
            info.pid = pidTmp;
            continue;
        }
        auto serialPos = line.find(SERIAL_FLAG);
        if (std::string::npos != serialPos)
        {
            info.serial = line.substr(serialPos + SERIAL_FLAG.size());
            continue;
        }
        auto productPos = line.find(PRODUCT_FLAG);
        if (std::string::npos != productPos)
        {
            info.product = line.substr(productPos + PRODUCT_FLAG.size());
            continue;
        }
        auto manufacturerPos = line.find(MANUFACTURER_FLAG);
        if (std::string::npos != manufacturerPos)
        {
            info.manufacturer = line.substr(manufacturerPos + MANUFACTURER_FLAG.size());
            continue;
        }
    }
    if (info.busNum >= 0 && info.portNum >= 0 && info.address >= 0)
    {
        usbList.emplace_back(info);
    }
    return usbList;
}

struct UsbUdevImpl
{
    int busNum = -1;
    int portNum = -1;
    int address = -1;
    std::string devNodeName;
    bool isBlock = false;
};

std::vector<UsbUdevImpl> enumerateUsbUdevs()
{
    std::vector<UsbUdevImpl> udevList;
    struct udev* udev = udev_new();
    if (!udev)
    {
        return udevList;
    }
    udev_enumerate* enumerate = udev_enumerate_new(udev);
    if (!enumerate)
    {
        udev_unref(udev);
        return udevList;
    }
    udev_enumerate_add_match_is_initialized(enumerate);
#if 0 /* 暂时屏蔽设备过滤类型 */
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_add_match_subsystem(enumerate, "scsi");
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
#endif
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry* devEntryList = udev_enumerate_get_list_entry(enumerate);
    if (!devEntryList)
    {
        udev_enumerate_unref(enumerate);
        udev_unref(udev);
        return udevList;
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
        if (dev)
        {
            const char* devNode = udev_device_get_devnode(dev);
            if (devNode)
            {
                struct udev_device* pDev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
                if (pDev)
                {
                    const char* busNumPtr = udev_device_get_property_value(pDev, "BUSNUM");
                    const char* portNumPtr = udev_device_get_sysnum(pDev);
                    const char* devNumPtr = udev_device_get_property_value(pDev, "DEVNUM");
                    if (busNumPtr && portNumPtr && devNumPtr)
                    {
                        const char* subSystemPtr = udev_device_get_subsystem(dev);
                        UsbUdevImpl info;
                        try
                        {
                            info.busNum = std::atoi(busNumPtr);
                            info.portNum = std::atoi(portNumPtr);
                            info.address = std::atoi(devNumPtr);
                        }
                        catch (...)
                        {
                        }
                        info.devNodeName = devNode;
                        info.isBlock = (subSystemPtr && 0 == strcmp(subSystemPtr, "block"));
                        udevList.emplace_back(info);
                    }
                }
            }
            udev_device_unref(dev);
        }
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return udevList;
}

void enumerateUsbDevNodes(std::vector<UsbImpl>& usbList, int timeoutS = 5)
{
    if (usbList.empty())
    {
        return;
    }
    std::vector<std::string> outVec;
    if (timeoutS > 0)
    {
        runCommand("timeout " + std::to_string(timeoutS) + " lsblk -abOP", nullptr, &outVec);
    }
    else
    {
        runCommand("lsblk -abOP", nullptr, &outVec);
    }
    auto udevList = enumerateUsbUdevs();
    for (const auto& info : udevList)
    {
        auto iter = std::find_if(usbList.begin(), usbList.end(), [&](const UsbImpl& item) {
            return (item.busNum == info.busNum && item.portNum == info.portNum && item.address == info.address);
        });
        if (usbList.end() == iter)
        {
            continue;
        }
        if (info.isBlock) /* 存储设备 */
        {
            auto it = std::find_if(outVec.begin(), outVec.end(), [&](const std::string& line) {
                return (std::string::npos != line.find("PATH=\"" + info.devNodeName + "\"")); /* 找到设备节点对应信息 */
            });
            if (outVec.end() == it)
            {
                continue;
            }
            const auto& infoStr = (*it);
            static const std::string TYPE_FLAG = " TYPE=\""; /* 设备类型 */
            static const std::string FSTYPE_FLAG = " FSTYPE=\""; /* 文件系统类型 */
            static const std::string LABEL_FLAG = " LABEL=\""; /* 文件系统标签 */
            static const std::string PARTLABEL_FLAG = " PARTLABEL=\""; /* 分区标签 */
            static const std::string MODEL_FLAG = " MODEL=\""; /* 设备标识符 */
            static const std::string VENDOR_FLAG = " VENDOR=\""; /* 设备制造商 */
            static const std::string SIZE_FLAG = " SIZE=\""; /* 大小 */
            std::string type;
            auto typePos = infoStr.find(TYPE_FLAG);
            if (std::string::npos != typePos)
            {
                auto ep = infoStr.find('"', typePos + TYPE_FLAG.size());
                if (std::string::npos != ep)
                {
                    type = infoStr.substr(typePos + TYPE_FLAG.size(), ep - typePos - TYPE_FLAG.size());
                }
            }
            std::string fstype;
            auto fstypePos = infoStr.find(FSTYPE_FLAG);
            if (std::string::npos != fstypePos)
            {
                auto ep = infoStr.find('"', fstypePos + FSTYPE_FLAG.size());
                if (std::string::npos != ep)
                {
                    fstype = infoStr.substr(fstypePos + FSTYPE_FLAG.size(), ep - fstypePos - FSTYPE_FLAG.size());
                }
            }
            std::string label;
            auto labelPos = infoStr.find(LABEL_FLAG);
            if (std::string::npos != labelPos)
            {
                auto ep = infoStr.find('"', labelPos + LABEL_FLAG.size());
                if (std::string::npos != ep)
                {
                    label = infoStr.substr(labelPos + LABEL_FLAG.size(), ep - labelPos - LABEL_FLAG.size());
                }
            }
            std::string partlabel;
            auto partlabelPos = infoStr.find(PARTLABEL_FLAG);
            if (std::string::npos != partlabelPos)
            {
                auto ep = infoStr.find('"', partlabelPos + PARTLABEL_FLAG.size());
                if (std::string::npos != ep)
                {
                    partlabel = infoStr.substr(partlabelPos + PARTLABEL_FLAG.size(), ep - partlabelPos - PARTLABEL_FLAG.size());
                }
            }
            std::string model;
            auto modelPos = infoStr.find(MODEL_FLAG);
            if (std::string::npos != modelPos)
            {
                auto ep = infoStr.find('"', modelPos + MODEL_FLAG.size());
                if (std::string::npos != ep)
                {
                    model = infoStr.substr(modelPos + MODEL_FLAG.size(), ep - modelPos - MODEL_FLAG.size());
                }
            }
            std::string vendor;
            auto vendorPos = infoStr.find(VENDOR_FLAG);
            if (std::string::npos != vendorPos)
            {
                auto ep = infoStr.find('"', vendorPos + VENDOR_FLAG.size());
                if (std::string::npos != ep)
                {
                    vendor = infoStr.substr(vendorPos + VENDOR_FLAG.size(), ep - vendorPos - VENDOR_FLAG.size());
                }
            }
            size_t capacity = 0;
            auto sizePos = infoStr.find(SIZE_FLAG);
            if (std::string::npos != sizePos)
            {
                auto ep = infoStr.find('"', sizePos + SIZE_FLAG.size());
                if (std::string::npos != ep)
                {
                    capacity = std::atoll(infoStr.substr(sizePos + SIZE_FLAG.size(), ep - sizePos - SIZE_FLAG.size()).c_str());
                }
            }
            if (iter->model.empty())
            {
                iter->model = model;
            }
            if (iter->vendor.empty())
            {
                iter->vendor = vendor;
            }
            if ("disk" == type) /* 磁盘 */
            {
                iter->storageType = type;
                iter->blockDevRootNode = DevNode(info.devNodeName, "", fstype, label, partlabel, capacity);
            }
            else if ("part" == type) /* 分区 */
            {
                iter->storageType = type;
                iter->blockDevNodes.emplace_back(DevNode(info.devNodeName, "", fstype, label, partlabel, capacity));
            }
            else if ("rom" == type) /* 光驱 */
            {
                iter->storageType = "cdrom";
                iter->blockDevNodes.emplace_back(DevNode(info.devNodeName, "", fstype, label, partlabel, capacity));
            }
            else /* 其他 */
            {
                iter->storageType = type;
                iter->blockDevNodes.emplace_back(DevNode(info.devNodeName, "", fstype, label, partlabel, capacity));
            }
        }
        else /* 非存储设备 */
        {
            iter->nonblockDevNodes.emplace_back(DevNode(info.devNodeName));
        }
    }
    for (auto& item : usbList)
    {
        if (!item.blockDevRootNode.name.empty())
        {
            if (item.blockDevNodes.empty())
            {
                item.blockDevNodes.emplace_back(item.blockDevRootNode);
            }
            else
            {
                for (auto& devNode : item.blockDevNodes)
                {
                    devNode.pkname = item.blockDevRootNode.name;
                }
            }
        }
    }
}
#endif

bool isMountpoint(std::string path)
{
    path.erase(0, path.find_first_not_of(' '));
    path.erase(path.find_last_not_of(' ') + 1);
    if (path.empty())
    {
        return false;
    }
    std::string::size_type pos = 0;
    while (std::string::npos != (pos = path.find("\\", pos)))
    {
        path.replace(pos, 1, "/");
        pos += 1;
    }
    if (path.size() > 1 && '/' == path[path.size() - 1])
    {
        path.erase(path.size() - 1, 1);
    }
#ifdef _WIN32
    if (1 == path.size() || (2 == path.size() && ':' == path[path.size() - 1]))
    {
        char driverChar = toupper(path[0]);
        DWORD drives = GetLogicalDrives();
        int index = 0;
        while (drives)
        {
            if (1 == (drives & 0x1))
            {
                if ('A' + index == driverChar)
                {
                    return true;
                }
            }
            drives = drives >> 1;
            ++index;
        }
    }
#else
    path.insert(0, " ");
    std::vector<std::string> outVec;
    runCommand("df | grep \"" + path + "\"", nullptr, &outVec);
    if (1 == outVec.size() && (outVec[0].rfind(path) + path.size()) == outVec[0].size())
    {
        return true;
    }
#endif
    return false;
}

#ifdef _WIN32
std::vector<CdromInfo> getCdromInfoList()
{
    std::vector<CdromInfo> infoList;
    if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        IDiscMaster2* pDiscMaster;
        if (SUCCEEDED(CoCreateInstance(CLSID_MsftDiscMaster2, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDiscMaster))))
        {
            LONG totalDevices = 0;
            if (SUCCEEDED(pDiscMaster->get_Count(&totalDevices)))
            {
                for (LONG index = 0; index < totalDevices; ++index)
                {
                    BSTR recorderID;
                    if (FAILED(pDiscMaster->get_Item(index, &recorderID)))
                    {
                        continue;
                    }
                    IDiscRecorder2* pDiscRecorder;
                    if (SUCCEEDED(CoCreateInstance(CLSID_MsftDiscRecorder2, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDiscRecorder))))
                    {
                        if (SUCCEEDED(pDiscRecorder->InitializeDiscRecorder(recorderID)))
                        {
                            SAFEARRAY* volumePathNames = NULL;
                            if (SUCCEEDED(pDiscRecorder->get_VolumePathNames(&volumePathNames)))
                            {
                                std::string driver = SafeArrayToString(volumePathNames);
                                SafeArrayDestroy(volumePathNames);
                                CdromInfo info;
                                info.name = driver;
                                SAFEARRAY* ppProfiles = NULL;
                                if (SUCCEEDED(pDiscRecorder->get_SupportedProfiles(&ppProfiles)) && ppProfiles)
                                {
                                    LONG lBound, uBound;
                                    if (SUCCEEDED(::SafeArrayGetLBound(ppProfiles, 1, &lBound))
                                        && SUCCEEDED(::SafeArrayGetUBound(ppProfiles, 1, &uBound)))
                                    {
                                        for (LONG i = lBound; i <= uBound; ++i)
                                        {
                                            VARIANT element;
                                            if (FAILED(::SafeArrayGetElement(ppProfiles, &i, &element)))
                                            {
                                                continue;
                                            }
                                            IMAPI_PROFILE_TYPE profileType = (IMAPI_PROFILE_TYPE)element.lVal;
                                            if (IMAPI_PROFILE_TYPE_CD_RECORDABLE == profileType)
                                            {
                                                info.can_write_CD_R = 1;
                                            }
                                            else if (IMAPI_PROFILE_TYPE_CD_REWRITABLE == profileType)
                                            {
                                                info.can_write_CD_RW = 1;
                                            }
                                            else if (IMAPI_PROFILE_TYPE_DVDROM == profileType)
                                            {
                                                info.can_read_DVD = 1;
                                            }
                                            else if (IMAPI_PROFILE_TYPE_DVD_DASH_RECORDABLE == profileType)
                                            {
                                                info.can_write_DVD_R = 1;
                                            }
                                            else if (IMAPI_PROFILE_TYPE_DVD_RAM == profileType)
                                            {
                                                info.can_write_DVD_RAM = 1;
                                            }
                                        }
                                    }
                                    SafeArrayDestroy(ppProfiles);
                                }
                                infoList.emplace_back(info);
                            }
                        }
                        pDiscRecorder->Release();
                    }
                    SysFreeString(recorderID);
                }
            }
            pDiscMaster->Release();
        }
        CoUninitialize();
    }
    return infoList;
}
#else
std::vector<CdromInfo> getCdromInfoList(std::string& outStr)
{
    std::vector<CdromInfo> infoList;
    std::vector<std::string> outVec;
    runCommand("cat /proc/sys/dev/cdrom/info", &outStr, &outVec);
    for (const auto& line : outVec)
    {
        std::vector<std::string> vec;
        for (size_t i = 0; i <= line.size();)
        {
            auto pos = line.find(":", i);
            if (std::string::npos == pos)
            {
                pos = line.size();
            }
            vec.emplace_back(line.substr(i, pos - i));
            i = pos + 1;
        }
        if (2 == vec.size())
        {
            const auto& title = vec[0];
            std::vector<std::string> valueList;
            std::string tmp;
            for (auto ch : vec[1])
            {
                if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
                {
                    tmp.push_back(ch);
                }
                else if (!tmp.empty())
                {
                    valueList.emplace_back(tmp);
                    tmp.clear();
                }
            }
            if (!tmp.empty())
            {
                valueList.emplace_back(tmp);
            }
            for (size_t i = 0; i < valueList.size(); ++i)
            {
                if ("drive name" == title)
                {
                    infoList.emplace_back(CdromInfo("/dev/" + valueList[i]));
                }
                else if (i < infoList.size())
                {
                    if ("drive speed" == title)
                    {
                        infoList[i].speed = std::atoi(valueList[i].c_str());
                    }
                    else if ("drive # of slots" == title)
                    {
                        infoList[i].slotNum = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can close tray" == title)
                    {
                        infoList[i].can_close_tray = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can open tray" == title)
                    {
                        infoList[i].can_open_tray = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can lock tray" == title)
                    {
                        infoList[i].can_lock_tray = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can change speed" == title)
                    {
                        infoList[i].can_change_speed = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can select disk" == title)
                    {
                        infoList[i].can_select_disk = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can read multisession" == title)
                    {
                        infoList[i].can_read_multisession = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can read MCN" == title)
                    {
                        infoList[i].can_read_MCN = std::atoi(valueList[i].c_str());
                    }
                    else if ("Reports media changed" == title)
                    {
                        infoList[i].reports_media_changed = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can play audio" == title)
                    {
                        infoList[i].can_play_audio = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can write CD-R" == title)
                    {
                        infoList[i].can_write_CD_R = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can write CD-RW" == title)
                    {
                        infoList[i].can_write_CD_RW = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can read DVD" == title)
                    {
                        infoList[i].can_read_DVD = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can write DVD-R" == title)
                    {
                        infoList[i].can_write_DVD_R = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can write DVD-RAM" == title)
                    {
                        infoList[i].can_write_DVD_RAM = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can read MRW" == title)
                    {
                        infoList[i].can_read_MRW = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can write MRW" == title)
                    {
                        infoList[i].can_write_MRW = std::atoi(valueList[i].c_str());
                    }
                    else if ("Can write RAM" == title)
                    {
                        infoList[i].can_write_RAM = std::atoi(valueList[i].c_str());
                    }
                }
            }
        }
    }
    return infoList;
}
#endif

DevNode::DevNode(const std::string& name, const std::string& pkname, const std::string& fstype, const std::string& label,
                 const std::string& partlabel, size_t capacity, const std::string& winDriver)
    : name(name), pkname(pkname), fstype(fstype), label(label), partlabel(partlabel), capacity(capacity), m_winDriver(winDriver)
{
}

std::string DevNode::getMountpoint() const
{
    std::string mountpoint;
#if _WIN32
    mountpoint = m_winDriver;
#else
    if (0 == name.find("/dev/"))
    {
        auto pos = name.find_last_of("/");
        auto basename = (pos < name.size()) ? name.substr(pos + 1, name.size() - 1) : name;
        std::vector<std::string> outVec;
        runCommand("lsblk | grep \"" + basename + " \"", nullptr, &outVec);
        if (1 == outVec.size() && !outVec[0].empty())
        {
            auto line = outVec[0];
            line.erase(0, line.find_first_not_of(' '));
            line.erase(line.find_last_not_of(' ') + 1);
            pos = line.find_last_of(' ');
            if (std::string::npos != pos)
            {
                mountpoint = (pos < line.size()) ? line.substr(pos + 1, line.size() - 1) : line;
                mountpoint = (mountpoint.empty() || '/' != mountpoint[0]) ? std::string() : mountpoint;
            }
        }
    }
#endif
    return mountpoint;
}

Usb::Usb(const Usb& src)
{
    m_dev = src.m_dev;
    m_parent = src.m_parent;
    m_children = src.m_children;
    m_parentPath = src.m_parentPath;
    m_path = src.m_path;
    m_busNum = src.m_busNum;
    m_portNum = src.m_portNum;
    m_address = src.m_address;
    m_classCode = src.m_classCode;
    m_subClassCode = src.m_subClassCode;
    m_protocolCode = src.m_protocolCode;
    m_subProtocolCode = src.m_subProtocolCode;
    m_speedLevel = src.m_speedLevel;
    m_vid = src.m_vid;
    m_pid = src.m_pid;
    m_serial = src.m_serial;
    m_product = src.m_product;
    m_model = src.m_model;
    m_vendor = src.m_vendor;
    m_storageType = src.m_storageType;
    m_manufacturer = src.m_manufacturer;
    m_devNodes = src.m_devNodes;
    m_cdromInfo = src.m_cdromInfo;
}

std::shared_ptr<Usb> Usb::getParent() const
{
    return m_parent;
}

std::vector<std::weak_ptr<usb::Usb>> Usb::getChildren() const
{
    return m_children;
}

std::string Usb::getParentPath() const
{
    return m_parentPath;
}

std::string Usb::getPath() const
{
    return m_path;
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

std::string Usb::getClassHex() const
{
    char buf[4] = {0};
    sprintf(buf, "%02Xh", m_classCode);
    return buf;
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

int Usb::getSubClassCode() const
{
    return m_subClassCode;
}

int Usb::getProtocolCode() const
{
    return m_protocolCode;
}

std::vector<int> Usb::getSubProtocolCode() const
{
    return m_subProtocolCode;
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

std::string Usb::getModel() const
{
    return m_model;
}

std::string Usb::getVendor() const
{
    return m_vendor;
}

std::string Usb::getStorageType() const
{
    return m_storageType;
}

std::vector<DevNode> Usb::getDevNodes() const
{
    return m_devNodes;
}

CdromInfo Usb::getCdromInfo() const
{
    return m_cdromInfo;
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

bool Usb::isDisk() const
{
    return ("disk" == m_storageType || "part" == m_storageType);
}

bool Usb::isCdrom() const
{
    return ("cdrom" == m_storageType);
}

std::string Usb::describe(bool showChildren, int allIntend, int intend) const
{
    std::string allIntendStr(allIntend, ' '), intendStr(intend, ' '), crlfStr(intend > 0 ? "\n" : "");
    std::string desc;
    desc += allIntendStr; /* 缩进 */
    desc += "{";
    desc += crlfStr + allIntendStr + intendStr; /* 换行/缩进 */
    desc += "\"parentPath\": \"" + m_parentPath + "\", ";
    desc += "\"path\": \"" + m_path + "\", ";
    desc += "\"busNum\": " + std::to_string(m_busNum) + ", ";
    desc += "\"portNum\": " + std::to_string(m_portNum) + ", ";
    desc += "\"address\": " + std::to_string(m_address) + ", ";
    desc += crlfStr + allIntendStr + intendStr; /* 换行/缩进 */
    desc += "\"classCode\": " + std::to_string(m_classCode) + ", ";
    desc += "\"classHex\": \"" + getClassHex() + "\", ";
    desc += "\"classDesc\": \"" + getClassDesc() + "\", ";
    desc += "\"subClass\": " + std::to_string(m_subClassCode) + ", ";
    desc += "\"protocol\": " + std::to_string(m_protocolCode) + ", ";
    desc += "\"subProtocol\": [";
    for (size_t i = 0; i < m_subProtocolCode.size(); ++i)
    {
        desc += (i > 0) ? ", " : "";
        desc += std::to_string(m_subProtocolCode[i]);
    }
    desc += "], ";
    desc += "\"speed\": " + std::to_string(m_speedLevel) + ", ";
    desc += "\"speedDesc\": \"" + getSpeedDesc() + "\", ";
    desc += crlfStr + allIntendStr + intendStr; /* 换行/缩进 */
    desc += "\"vid\": \"" + m_vid + "\", ";
    desc += "\"pid\": \"" + m_pid + "\", ";
    desc += "\"serial\": \"" + m_serial + "\", ";
    desc += "\"product\": \"" + m_product + "\", ";
    desc += "\"manufacturer\": \"" + m_manufacturer + "\", ";
    desc += "\"model\": \"" + m_model + "\", ";
    desc += "\"vendor\": \"" + m_vendor + "\", ";
    desc += "\"storageType\": \"" + m_storageType + "\"";
    if (m_devNodes.size() > 0)
    {
        desc += ",";
        desc += crlfStr + allIntendStr + intendStr; /* 换行/缩进 */
        desc += "\"devNodes\": ";
        desc += "[";
        for (size_t i = 0; i < m_devNodes.size(); ++i)
        {
            desc += (i > 0) ? "," : "";
            if (m_devNodes.size() > 1)
            {
                desc += crlfStr + allIntendStr + intendStr + intendStr; /* 换行/缩进 */
            }
            desc += "{";
            size_t fieldCount = 0;
            if (!m_devNodes[i].name.empty())
            {
                ++fieldCount;
                desc += "\"name\": \"" + m_devNodes[i].name + "\"";
            }
            if (!m_devNodes[i].pkname.empty())
            {
                desc += (++fieldCount > 1) ? ", " : "";
                desc += "\"pkname\": \"" + m_devNodes[i].pkname + "\"";
            }
            if (!m_devNodes[i].fstype.empty())
            {
                desc += (++fieldCount > 1) ? ", " : "";
                desc += "\"fstype\": \"" + m_devNodes[i].fstype + "\"";
            }
            if (!m_devNodes[i].label.empty())
            {
                desc += (++fieldCount > 1) ? ", " : "";
                desc += "\"label\": \"" + m_devNodes[i].label + "\"";
            }
            if (!m_devNodes[i].partlabel.empty())
            {
                desc += (++fieldCount > 1) ? ", " : "";
                desc += "\"partlabel\": \"" + m_devNodes[i].partlabel + "\"";
            }
            auto mountpoint = m_devNodes[i].getMountpoint();
            if (!mountpoint.empty())
            {
                desc += (++fieldCount > 1) ? ", " : "";
                desc += "\"mountpoint\": \"" + mountpoint;
#ifdef _WIN32
                if (!mountpoint.empty())
                {
                    desc += "\\"; /* JSON字符串要再增加1个反斜杠"\"进行转义 */
                }
                desc += "\"";
#endif
            }
#ifdef _WIN32
            if (!m_cdromInfo.name.empty() && mountpoint == m_cdromInfo.name)
#else
            if (!m_cdromInfo.name.empty() && m_devNodes[i].name == m_cdromInfo.name)
#endif
            {
                desc += ", ";
                desc += "\"CD-R\": " + std::to_string(m_cdromInfo.can_write_CD_R) + ", ";
                desc += "\"CD-RW\": " + std::to_string(m_cdromInfo.can_write_CD_RW) + ", ";
                desc += "\"DVD-ROM\": " + std::to_string(m_cdromInfo.can_read_DVD) + ", ";
                desc += "\"DVD-R\": " + std::to_string(m_cdromInfo.can_write_DVD_R) + ", ";
                desc += "\"DVD-RAW\": " + std::to_string(m_cdromInfo.can_write_DVD_RAM);
            }
            desc += "}";
        }
        if (m_devNodes.size() > 1)
        {
            desc += crlfStr + allIntendStr + intendStr; /* 换行/缩进 */
        }
        desc += "]";
    }
    if (showChildren && !m_children.empty())
    {
        desc += ",";
        desc += crlfStr + allIntendStr + intendStr; /* 换行/缩进 */
        desc += "\"children\": [";
        for (size_t i = 0; i < m_children.size(); ++i)
        {
            const auto child = m_children[i].lock();
            if (child)
            {
                desc += (i > 0) ? "," : "";
                desc += crlfStr; /* 换行 */
                desc += child->describe(showChildren, allIntend + intend + intend, intend);
            }
        }
        desc += crlfStr + allIntendStr + intendStr; /* 换行/缩进 */
        desc += "]";
    }
    desc += crlfStr + allIntendStr; /* 换行/缩进 */
    desc += "}";
    return desc;
}

std::vector<std::shared_ptr<usb::Usb>> Usb::getAllUsbs(bool detailFlag)
{
    std::vector<std::shared_ptr<usb::Usb>> usbList;
    libusb_context* ctx = NULL;
    if (LIBUSB_SUCCESS == libusb_init_context(&ctx, NULL, 0))
    {
        libusb_device** devList = NULL;
        ssize_t count = libusb_get_device_list(ctx, &devList);
        if (count > 0 && devList)
        {
            std::vector<UsbImpl> implList;
            std::vector<CdromInfo> cdromList;
            if (detailFlag)
            {
#ifdef _WIN32
                /* Windows平台下libusb无法打开设备, 需要通过系统API获取详细信息 */
                implList = enumerateHostControllers();
                cdromList = getCdromInfoList();
#else
                /* Linux平台下libusb打开设备可能会卡住, 因此通过读取系统内核文件获取详细信息 */
                implList = enumerateUsbDevices();
                enumerateUsbDevNodes(implList, 5);
                std::string outStr;
                cdromList = getCdromInfoList(outStr);
#endif
            }
            /* 遍历设备列表 */
            for (ssize_t i = 0; i < count; ++i)
            {
                auto info = parseUsb(devList[i], detailFlag, implList, cdromList);
                if (info)
                {
                    usbList.emplace_back(info);
                }
            }
            /* 确认父子节点关系 */
            for (size_t m = 0; m < usbList.size(); ++m)
            {
                if (usbList[m]->m_parent)
                {
                    for (size_t n = 0; n < usbList.size(); ++n)
                    {
                        if (usbList[m]->m_parent->m_dev == usbList[n]->m_dev)
                        {
                            usbList[m]->m_parent = usbList[n];
                            usbList[n]->m_children.emplace_back(usbList[m]);
                            break;
                        }
                    }
                }
            }
            for (size_t k = 0; k < usbList.size(); ++k)
            {
                usbList[k]->m_dev = NULL;
                if (usbList[k]->m_parent)
                {
                    usbList[k]->m_parentPath = usbList[k]->m_parent->calculatePath();
                }
                usbList[k]->m_path = usbList[k]->calculatePath();
            }
            libusb_free_device_list(devList, 1);
        }
        libusb_exit(ctx);
    }
    return usbList;
}

#ifdef _WIN32
bool Usb::registerDeviceNotify(HANDLE handle)
{
    if (!handle)
    {
        return false;
    }
    static const GUID GUID_DEVINTERFACE_LIST[] = {{0xA5DCBF10, 0x6530, 0x11D2, {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED}},
                                                  {0x53f56307, 0xb6bf, 0x11d0, {0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b}}};
    HDEVNOTIFY devNotify;
    for (int i = 0; i < sizeof(GUID_DEVINTERFACE_LIST) / sizeof(GUID); i++)
    {
        DEV_BROADCAST_DEVICEINTERFACE notifyFilter;
        ZeroMemory(&notifyFilter, sizeof(notifyFilter));
        notifyFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        notifyFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        notifyFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
        devNotify = RegisterDeviceNotification(handle, &notifyFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
        if (!devNotify)
        {
            return false;
        }
    }
    return true;
}
#endif

std::string Usb::calculatePath() const
{
    std::string path = std::to_string(m_portNum);
    auto parent = m_parent;
    while (parent && parent->m_parent)
    {
        path = std::to_string(parent->m_portNum) + "." + path;
        parent = parent->m_parent;
    }
    path = std::to_string(m_busNum) + "-" + path;
    return path;
}

std::shared_ptr<usb::Usb> Usb::parseUsb(libusb_device* dev, bool detailFlag, const std::vector<UsbImpl>& implList,
                                        const std::vector<CdromInfo>& cdromList)
{
    if (!dev)
    {
        return nullptr;
    }
    struct libusb_device_descriptor desc;
    if (LIBUSB_SUCCESS != libusb_get_device_descriptor(dev, &desc))
    {
        return nullptr;
    }
    auto info = std::make_shared<Usb>();
    info->m_dev = dev;
    libusb_device* parent = libusb_get_parent(dev);
    if (parent) /* 解析父节点 */
    {
        info->m_parent = std::make_shared<Usb>();
        info->m_parent->m_dev = parent;
    }
    info->m_busNum = libusb_get_bus_number(dev); /* 总线编号 */
    info->m_portNum = libusb_get_port_number(dev); /* 端口编号(Linux中也叫系统编号sysNum) */
    info->m_address = libusb_get_device_address(dev); /* 地址(每次拔插都会变) */
    int classCode = desc.bDeviceClass; /* 设备类型编码(用于判断鼠标,键盘,Hub等) */
    if (LIBUSB_CLASS_PER_INTERFACE == desc.bDeviceClass && desc.bNumConfigurations > 0)
    {
        struct libusb_config_descriptor* config = NULL;
        if (LIBUSB_SUCCESS == libusb_get_config_descriptor(dev, 0, &config) && config)
        {
            for (uint8_t i = 0; i < config->bNumInterfaces; ++i)
            {
                struct libusb_interface inf = config->interface[i];
                if (inf.num_altsetting > 0 && inf.altsetting)
                {
                    if (0 == classCode)
                    {
                        classCode = inf.altsetting->bInterfaceClass;
                    }
                    info->m_subProtocolCode.emplace_back(inf.altsetting->bInterfaceProtocol);
                }
            }
            libusb_free_config_descriptor(config);
        }
    }
    info->m_classCode = classCode;
    info->m_subClassCode = desc.bDeviceSubClass;
    info->m_protocolCode = desc.bDeviceProtocol;
    info->m_speedLevel = libusb_get_device_speed(dev); /* 速度等级 */
    char vid[6] = {0}; /* 厂商ID */
    sprintf(vid, "%04x", desc.idVendor);
    info->m_vid = vid;
    char pid[6] = {0}; /* 产品ID */
    sprintf(pid, "%04x", desc.idProduct);
    info->m_pid = pid;
    if (detailFlag) /* 获取详细信息 */
    {
        /* 获取详细信息 */
        for (const auto& item : implList)
        {
            if (item.busNum == info->m_busNum && item.portNum == info->m_portNum && item.address == info->m_address
                && item.vid == info->m_vid && item.pid == info->m_pid)
            {
                if (info->m_serial.empty())
                {
                    info->m_serial = item.serial;
                }
                if (info->m_product.empty())
                {
                    info->m_product = item.product;
                }
                if (info->m_manufacturer.empty())
                {
                    info->m_manufacturer = item.manufacturer;
                }
                if (info->m_model.empty())
                {
                    info->m_model = item.model;
                }
                if (info->m_vendor.empty())
                {
                    info->m_vendor = item.vendor;
                }
                if (info->m_storageType.empty())
                {
                    info->m_storageType = item.storageType;
                }
#ifdef _WIN32
                for (const auto& di : item.driverList)
                {
                    info->m_devNodes.emplace_back(DevNode("", "", di.fstype, di.label, "", 0, di.driver));
                }
                for (const auto& ci : item.comList)
                {
                    info->m_devNodes.emplace_back(DevNode(ci.portName));
                }
#else
                info->m_devNodes = item.blockDevNodes;
                for (const auto& devNode : item.nonblockDevNodes)
                {
                    info->m_devNodes.emplace_back(devNode);
                }
#endif
                if ("cdrom" == info->m_storageType && !info->m_devNodes.empty())
                {
                    auto iter = std::find_if(cdromList.begin(), cdromList.end(), [&](const CdromInfo& item) {
#ifdef _WIN32
                        return (item.name == info->m_devNodes[0].getMountpoint());
#else
                        for (const auto& devNode : info->m_devNodes)
                        {
                            if (devNode.name == item.name)
                            {
                                return true;
                            }
                        }
                        return false;
#endif
                    });
                    if (cdromList.end() != iter)
                    {
                        info->m_cdromInfo = (*iter);
                    }
                }
                break;
            }
        }
#if 0 /* Windows平台下打不开, Linux平台下有时会卡住, 因此弃而不用 */
        if (info->m_serial.empty() || info->m_product.empty() || info->m_manufacturer.empty())
        {
            libusb_device_handle* handle = NULL;
            if (LIBUSB_SUCCESS == libusb_open(dev, &handle) && handle)
            {
                unsigned char buf[256];
                int len;
                if (desc.iSerialNumber && info->m_serial.empty())
                {
                    len = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, buf, sizeof(buf));
                    if (len > 0)
                    {
                        info->m_serial = std::string((char*)buf, len);
                    }
                }
                if (desc.iProduct && info->m_product.empty())
                {
                    len = libusb_get_string_descriptor_ascii(handle, desc.iProduct, buf, sizeof(buf));
                    if (len > 0)
                    {
                        info->m_product = std::string((char*)buf, len);
                    }
                }
                if (desc.iManufacturer && info->m_manufacturer.empty())
                {
                    len = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, buf, sizeof(buf));
                    if (len > 0)
                    {
                        info->m_manufacturer = std::string((char*)buf, len);
                    }
                }
                libusb_close(handle);
            }
        }
#endif
    }
    return info;
}
} // namespace usb
