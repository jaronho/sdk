#include "usb.h"

#include <algorithm>
#include <string.h>

#ifdef _WIN32
// Windows.h必须比其他平台文件先包含
#include <Windows.h>
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
#include <usbiodef.h>
#include <usbuser.h>
#pragma comment(lib, "setupapi.lib")
#else
#include <libudev.h>
#endif

namespace usb
{
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
#ifdef _WIN32
    std::string dirverName; /* 设备驱动名称 */
    std::string vendor; /* 设备制造商, 例如: "FNK TECH", "HL-DT-ST", "Samsung " 等 */
    std::string model; /* 设备标识符(型号), 例如: "ELSKY_SSD_256GB", "CDRW_DVD_GCC4244", "DVD_A_DS8A5SH", "USB CARD READER " 等 */
    std::string storageType; /* 存储设备类型, 值: disk-磁盘, cdrom-光驱 */
#else
    DevNode devRootNode; /* 设备根节点 */
    std::vector<DevNode> devNodes; /* 设备节点 */
#endif
};

#ifdef _WIN32
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

void enumerateHub(int rootIndex, std::string hubName, PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo,
                  PUSB_DESCRIPTOR_REQUEST ConfigDesc, PSTRING_DESCRIPTOR_NODE stringDescs, std::vector<UsbImpl>& usbList);

std::string wstring2string(const std::wstring& wstr)
{
    if (!wstr.empty())
    {
        size_t len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
        char* buf = (char*)malloc(sizeof(char) * (len + 1));
        if (buf)
        {
            WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buf, len, NULL, NULL);
            buf[len] = '\0';
            std::string str(buf);
            free(buf);
            return str;
        }
    }
    return std::string();
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
    PUSB_ROOT_HUB_NAME rootHubNameW = (PUSB_ROOT_HUB_NAME)GlobalAlloc(GPTR, nBytes);
    if (NULL == rootHubNameW)
    {
        return "";
    }
    std::string rootHubNameA;
    if (DeviceIoControl(hostController, IOCTL_USB_GET_ROOT_HUB_NAME, NULL, 0, rootHubNameW, nBytes, &nBytes, NULL))
    {
        rootHubNameA = wstring2string(std::wstring(rootHubNameW->RootHubName, nBytes - sizeof(USB_ROOT_HUB_NAME) + sizeof(WCHAR)));
    }
    GlobalFree(rootHubNameW);
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
    PUSB_NODE_CONNECTION_NAME extHubNameW = (PUSB_NODE_CONNECTION_NAME)GlobalAlloc(GPTR, nBytes);
    if (NULL == extHubNameW)
    {
        return "";
    }
    extHubNameW->ConnectionIndex = connectionIndex;
    std::string extHubNameA;
    if (DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_NAME, extHubNameW, nBytes, extHubNameW, nBytes, &nBytes, NULL))
    {
        extHubNameA = wstring2string(std::wstring(extHubNameW->NodeName, nBytes - sizeof(USB_NODE_CONNECTION_NAME) + sizeof(WCHAR)));
    }
    GlobalFree(extHubNameW);
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
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME)GlobalAlloc(GPTR, nBytes);
    if (NULL == driverKeyNameW)
    {
        return "";
    }
    driverKeyNameW->ConnectionIndex = connectionIndex;
    std::string driverKeyNameA;
    if (DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME, driverKeyNameW, nBytes, driverKeyNameW, nBytes, &nBytes, NULL))
    {
        driverKeyNameA = wstring2string(
            std::wstring(driverKeyNameW->DriverKeyName, nBytes - sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME) + sizeof(WCHAR)));
    }
    GlobalFree(driverKeyNameW);
    return driverKeyNameA;
}

BOOL getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devInfoData, DWORD property, LPTSTR* buffer)
{
    if (NULL == buffer)
    {
        return FALSE;
    }
    *buffer = NULL;
    DWORD requiredSize = 0;
    BOOL ret = SetupDiGetDeviceRegistryProperty(devInfo, devInfoData, property, NULL, NULL, 0, &requiredSize);
    if ((0 == requiredSize) || (FALSE != ret && ERROR_INSUFFICIENT_BUFFER != GetLastError()))
    {
        return FALSE;
    }
    *buffer = (LPTSTR)GlobalAlloc(GPTR, requiredSize);
    if (NULL == *buffer)
    {
        return FALSE;
    }
    if (FALSE == SetupDiGetDeviceRegistryProperty(devInfo, devInfoData, property, NULL, (PBYTE)*buffer, requiredSize, &requiredSize))
    {
        GlobalFree(*buffer);
        *buffer = NULL;
        return FALSE;
    }
    return TRUE;
}

void driverNameToDeviceInst(const std::string driveName, HDEVINFO* pDevInfo, PSP_DEVINFO_DATA pDevInfoData)
{
    if (driveName.empty() || NULL == pDevInfo || NULL == pDevInfoData)
    {
        return;
    }
    *pDevInfo = INVALID_HANDLE_VALUE;
    memset(pDevInfoData, 0, sizeof(SP_DEVINFO_DATA));
    HDEVINFO deviceInfo = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (INVALID_HANDLE_VALUE != deviceInfo)
    {
        bool matchFlag = false;
        SP_DEVINFO_DATA deviceInfoData;
        deviceInfoData.cbSize = sizeof(deviceInfoData);
        for (ULONG index = 0; SetupDiEnumDeviceInfo(deviceInfo, index, &deviceInfoData); ++index)
        {
            PSTR buf = NULL;
            if (getDeviceProperty(deviceInfo, &deviceInfoData, SPDRP_DRIVER, &buf))
            {
                if (0 == _stricmp(driveName.c_str(), buf))
                {
                    *pDevInfo = deviceInfo;
                    CopyMemory(pDevInfoData, &deviceInfoData, sizeof(deviceInfoData));
                    matchFlag = true;
                }
                GlobalFree(buf);
                if (matchFlag)
                {
                    break;
                }
            }
        }
        if (!matchFlag)
        {
            SetupDiDestroyDeviceInfoList(deviceInfo);
        }
    }
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
                ULONG nBytes = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, stringDescs->StringDescriptor->bString,
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

void parseBusRelations(const std::string& buffer, std::string& vendor, std::string& model, std::string& storageType)
{
    vendor.clear();
    model.clear();
    storageType.clear();
    auto pos = buffer.find("Ven_");
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
    pos = buffer.find("Prod_");
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
    pos = buffer.find("USBSTOR\\");
    if (std::string::npos != pos)
    {
        for (size_t i = pos + 8; i < buffer.size(); ++i)
        {
            if ('&' == buffer[i])
            {
                break;
            }
            storageType.push_back(buffer[i]);
        }
        std::transform(storageType.begin(), storageType.end(), storageType.begin(), tolower);
    }
}

void enumerateHubPorts(int rootIndex, HANDLE hHubDevice, ULONG numPorts, std::vector<UsbImpl>& usbList)
{
    for (ULONG index = 1; index <= numPorts; ++index)
    {
        ULONG nBytesEx = sizeof(USB_NODE_CONNECTION_INFORMATION_EX) + (sizeof(USB_PIPE_INFO) * 30);
        PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)GlobalAlloc(GPTR, nBytesEx);
        if (NULL == connectionInfoEx)
        {
            break;
        }
        USB_PORT_CONNECTOR_PROPERTIES portConnectorProps;
        ZeroMemory(&portConnectorProps, sizeof(portConnectorProps));
        portConnectorProps.ConnectionIndex = index;
        ULONG nBytes = 0;
        DeviceIoControl(hHubDevice, IOCTL_USB_GET_PORT_CONNECTOR_PROPERTIES, &portConnectorProps, sizeof(USB_PORT_CONNECTOR_PROPERTIES),
                        &portConnectorProps, sizeof(USB_PORT_CONNECTOR_PROPERTIES), &nBytes, NULL);
        connectionInfoEx->ConnectionIndex = index;
        if (!DeviceIoControl(hHubDevice, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX, connectionInfoEx, nBytesEx, connectionInfoEx,
                             nBytesEx, &nBytesEx, NULL))
        {
            nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) + sizeof(USB_PIPE_INFO) * 30;
            PUSB_NODE_CONNECTION_INFORMATION connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)GlobalAlloc(GPTR, nBytes);
            if (NULL == connectionInfo)
            {
                GlobalFree(connectionInfoEx);
                continue;
            }
            connectionInfo->ConnectionIndex = index;
            if (!DeviceIoControl(hHubDevice, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION, connectionInfo, nBytes, connectionInfo, nBytes,
                                 &nBytes, NULL))
            {
                GlobalFree(connectionInfo);
                GlobalFree(connectionInfoEx);
                continue;
            }
            connectionInfoEx->ConnectionIndex = connectionInfo->ConnectionIndex;
            connectionInfoEx->DeviceDescriptor = connectionInfo->DeviceDescriptor;
            connectionInfoEx->CurrentConfigurationValue = connectionInfo->CurrentConfigurationValue;
            connectionInfoEx->Speed = connectionInfo->LowSpeed ? UsbLowSpeed : UsbFullSpeed;
            connectionInfoEx->DeviceIsHub = connectionInfo->DeviceIsHub;
            connectionInfoEx->DeviceAddress = connectionInfo->DeviceAddress;
            connectionInfoEx->NumberOfOpenPipes = connectionInfo->NumberOfOpenPipes;
            connectionInfoEx->ConnectionStatus = connectionInfo->ConnectionStatus;
            memcpy(&connectionInfoEx->PipeList[0], &connectionInfo->PipeList[0], sizeof(USB_PIPE_INFO) * 30);
            GlobalFree(connectionInfo);
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
            std::string dirverName = getDriverKeyName(hHubDevice, index);
            std::string vendor, model, storageType;
            HDEVINFO devInfo = INVALID_HANDLE_VALUE;
            SP_DEVINFO_DATA devInfoData = {0};
            driverNameToDeviceInst(dirverName, &devInfo, &devInfoData);
            if (INVALID_HANDLE_VALUE != devInfo)
            {
                DEVPROPTYPE propertyType = 0;
                WCHAR propertyBuffer[1024] = {0};
                DWORD requiredSize = 0;
                if (SetupDiGetDevicePropertyW(devInfo, &devInfoData, &DEVPKEY_Device_BusRelations, &propertyType,
                                              reinterpret_cast<PBYTE>(propertyBuffer), sizeof(propertyBuffer), &requiredSize, 0))
                {
                    parseBusRelations(wstring2string(propertyBuffer), vendor, model, storageType);
                }
                SetupDiDestroyDeviceInfoList(devInfo);
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
            info.dirverName = dirverName;
            info.vendor = vendor;
            info.model = model;
            info.storageType = storageType;
            usbList.emplace_back(info);
        }
        if (connectionInfoEx->DeviceIsHub) /* Hub Device */
        {
            enumerateHub(rootIndex, getExternalHubName(hHubDevice, index), connectionInfoEx, configDesc, stringDescs, usbList);
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
}

void enumerateHub(int rootIndex, std::string hubName, PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfo,
                  PUSB_DESCRIPTOR_REQUEST configDesc, PSTRING_DESCRIPTOR_NODE stringDescs, std::vector<UsbImpl>& usbList)
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
    do
    {
        std::string deviceName = "\\\\.\\" + hubName;
        HANDLE hHubDevice = CreateFile(deviceName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE == hHubDevice)
        {
            break;
        }
        ULONG nBytes = 0;
        if (!DeviceIoControl(hHubDevice, IOCTL_USB_GET_NODE_INFORMATION, hubInfo, sizeof(USB_NODE_INFORMATION), hubInfo,
                             sizeof(USB_NODE_INFORMATION), &nBytes, NULL))
        {
            CloseHandle(hHubDevice);
            break;
        }
        if (NULL == connectionInfo) /* Root Hub */
        {
        }
        else /* Extend Hub */
        {
        }
        enumerateHubPorts(rootIndex, hHubDevice, hubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts, usbList);
        CloseHandle(hHubDevice);
    } while (0);
    GlobalFree(hubInfo);
}

std::vector<UsbImpl> enumerateHostControllers()
{
    std::vector<UsbImpl> usbList;
    HDEVINFO devInfo = SetupDiGetClassDevs((LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
    if (NULL == devInfo)
    {
        return usbList;
    }
    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (ULONG index = 0; SetupDiEnumDeviceInfo(devInfo, index, &devInfoData); ++index)
    {
        SP_DEVICE_INTERFACE_DATA devInterfaceData;
        devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        if (!SetupDiEnumDeviceInterfaces(devInfo, 0, (LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER, index, &devInterfaceData))
        {
            break;
        }
        ULONG requiredSize = 0;
        BOOL ret = SetupDiGetDeviceInterfaceDetail(devInfo, &devInterfaceData, NULL, 0, &requiredSize, NULL);
        if (FALSE == ret && ERROR_INSUFFICIENT_BUFFER != GetLastError())
        {
            break;
        }
        PSP_DEVICE_INTERFACE_DETAIL_DATA devDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, requiredSize);
        if (NULL == devDetailData)
        {
            break;
        }
        devDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if (SetupDiGetDeviceInterfaceDetail(devInfo, &devInterfaceData, devDetailData, requiredSize, &requiredSize, NULL))
        {
            HANDLE hHCDev = CreateFile(devDetailData->DevicePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (INVALID_HANDLE_VALUE != hHCDev)
            {
                enumerateHub(index + 1, getRootHubName(hHCDev), NULL, NULL, NULL, usbList);
                CloseHandle(hHCDev);
            }
        }
        GlobalFree(devDetailData);
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

void enumerateUsbDevNodes(std::vector<UsbImpl>& usbList)
{
    struct udev* udev = udev_new();
    if (!udev)
    {
        return;
    }
    udev_enumerate* enumerate = udev_enumerate_new(udev);
    if (!enumerate)
    {
        udev_unref(udev);
        return;
    }
    udev_enumerate_add_match_is_initialized(enumerate); /* 只查找已经初始化的设备 */
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
        return;
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
        if (!dev)
        {
            continue;
        }
        const char* devNode = udev_device_get_devnode(dev);
        do
        {
            if (!devNode)
            {
                break;
            }
            struct udev_device* pDev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
            if (!pDev)
            {
                break;
            }
            const char* busNumPtr = udev_device_get_property_value(pDev, "BUSNUM");
            const char* portNumPtr = udev_device_get_sysnum(pDev);
            const char* devNumPtr = udev_device_get_property_value(pDev, "DEVNUM");
            if (!busNumPtr || !portNumPtr || !devNumPtr)
            {
                break;
            }
            int busNum = -1, portNum = -1, address = -1;
            try
            {
                busNum = std::atoi(busNumPtr);
                portNum = std::atoi(portNumPtr);
                address = std::atoi(devNumPtr);
            }
            catch (...)
            {
            }
            auto iter = std::find_if(usbList.begin(), usbList.end(), [&](UsbImpl info) {
                return (busNum == info.busNum && portNum == info.portNum && address == info.address);
            });
            if (usbList.end() == iter)
            {
                break;
            }
            const char* subSystemPtr = udev_device_get_subsystem(dev);
            if (subSystemPtr && 0 == strcmp(subSystemPtr, "block")) /* 存储设备 */
            {
                auto pos = std::string(devNode).rfind('/');
                auto devName = std::string::npos == pos ? std::string() : std::string(devNode).substr(pos + 1);
                auto command = std::string("lsblk -aOP | grep -E 'NAME=\"") + devName + "\" KNAME=\"" + devName + "\" '";
                std::string outStr;
                runCommand(command, &outStr, nullptr);
                static const std::string GROUP_FLAG = " GROUP=\""; /* 组名 */
                static const std::string FSTYPE_FLAG = " FSTYPE=\""; /* 文件系统类型 */
                static const std::string LABEL_FLAG = " LABEL=\""; /* 文件系统标签 */
                static const std::string PARTLABEL_FLAG = " PARTLABEL=\""; /* 分区标签 */
                static const std::string TYPE_FLAG = " TYPE=\""; /* 设备类型 */
                static const std::string MODEL_FLAG = " MODEL=\""; /* 设备标识符 */
                static const std::string VENDOR_FLAG = " VENDOR=\""; /* 设备制造商 */
                std::string group;
                auto groupPos = outStr.find(GROUP_FLAG);
                if (std::string::npos != groupPos)
                {
                    auto ep = outStr.find('"', groupPos + GROUP_FLAG.size());
                    if (std::string::npos != ep)
                    {
                        group = outStr.substr(groupPos + GROUP_FLAG.size(), ep - groupPos - GROUP_FLAG.size());
                    }
                }
                std::string fstype;
                auto fstypePos = outStr.find(FSTYPE_FLAG);
                if (std::string::npos != fstypePos)
                {
                    auto ep = outStr.find('"', fstypePos + FSTYPE_FLAG.size());
                    if (std::string::npos != ep)
                    {
                        fstype = outStr.substr(fstypePos + FSTYPE_FLAG.size(), ep - fstypePos - FSTYPE_FLAG.size());
                    }
                }
                std::string label;
                auto labelPos = outStr.find(LABEL_FLAG);
                if (std::string::npos != labelPos)
                {
                    auto ep = outStr.find('"', labelPos + LABEL_FLAG.size());
                    if (std::string::npos != ep)
                    {
                        label = outStr.substr(labelPos + LABEL_FLAG.size(), ep - labelPos - LABEL_FLAG.size());
                    }
                }
                std::string partlabel;
                auto partlabelPos = outStr.find(PARTLABEL_FLAG);
                if (std::string::npos != partlabelPos)
                {
                    auto ep = outStr.find('"', partlabelPos + PARTLABEL_FLAG.size());
                    if (std::string::npos != ep)
                    {
                        partlabel = outStr.substr(partlabelPos + PARTLABEL_FLAG.size(), ep - partlabelPos - PARTLABEL_FLAG.size());
                    }
                }
                std::string type;
                auto typePos = outStr.find(TYPE_FLAG);
                if (std::string::npos != typePos)
                {
                    auto ep = outStr.find('"', typePos + TYPE_FLAG.size());
                    if (std::string::npos != ep)
                    {
                        type = outStr.substr(typePos + TYPE_FLAG.size(), ep - typePos - TYPE_FLAG.size());
                    }
                }
                std::string model;
                auto modelPos = outStr.find(MODEL_FLAG);
                if (std::string::npos != modelPos)
                {
                    auto ep = outStr.find('"', modelPos + MODEL_FLAG.size());
                    if (std::string::npos != ep)
                    {
                        model = outStr.substr(modelPos + MODEL_FLAG.size(), ep - modelPos - MODEL_FLAG.size());
                    }
                }
                std::string vendor;
                auto vendorPos = outStr.find(VENDOR_FLAG);
                if (std::string::npos != vendorPos)
                {
                    auto ep = outStr.find('"', vendorPos + VENDOR_FLAG.size());
                    if (std::string::npos != ep)
                    {
                        vendor = outStr.substr(vendorPos + VENDOR_FLAG.size(), ep - vendorPos - VENDOR_FLAG.size());
                    }
                }
                if ("disk" == group) /* 磁盘 */
                {
                    if ("disk" == type) /* 超块(不可挂载) */
                    {
                        iter->devRootNode = DevNode(devNode, group, fstype, label, partlabel, model, vendor);
                    }
                    else if ("part" == type) /* 分区 */
                    {
                        iter->devNodes.emplace_back(DevNode(devNode, group, fstype, label, partlabel, model, vendor));
                    }
                }
                else if ("cdrom" == group) /* 光驱 */
                {
                    iter->devNodes.emplace_back(DevNode(devNode, group, fstype, label, partlabel, model, vendor));
                }
            }
            else /* 非存储设备 */
            {
                iter->devNodes.emplace_back(DevNode(devNode));
            }
        } while (0);
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}
#endif

Usb::Usb(const Usb& src)
{
    m_busNum = src.m_busNum;
    m_portNum = src.m_portNum;
    m_address = src.m_address;
    m_classCode = src.m_classCode;
    m_subClassCode = src.m_subClassCode;
    m_protocolCode = src.m_protocolCode;
    m_speedLevel = src.m_speedLevel;
    m_vid = src.m_vid;
    m_pid = src.m_pid;
    m_serial = src.m_serial;
    m_product = src.m_product;
    m_manufacturer = src.m_manufacturer;
    m_devRootNode = src.m_devRootNode;
    m_devNodes = src.m_devNodes;
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

#ifdef _WIN32
std::string Usb::getVendor() const
{
    return m_devNodes.empty() ? "" : m_devNodes[0].vendor;
}

std::string Usb::getModel() const
{
    return m_devNodes.empty() ? "" : m_devNodes[0].model;
}

std::string Usb::getStorageType() const
{
    return m_devNodes.empty() ? "" : m_devNodes[0].group;
}
#else
DevNode Usb::getDevRootNode() const
{
    return m_devRootNode;
}

std::vector<DevNode> Usb::getDevNodes() const
{
    return m_devNodes;
}
#endif

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

std::string Usb::describe(int allIntend, int intend) const
{
    std::string allIntendStr(allIntend, ' '), intendStr(intend, ' ');
    std::string desc;
    desc += allIntendStr + "{";
    desc += "\n"; /* 换行 */
    desc += allIntendStr + intendStr;
    desc += "\"busNum\": " + std::to_string(getBusNum());
    desc += ", ";
    desc += "\"portNum\": " + std::to_string(getPortNum());
    desc += ", ";
    desc += "\"address\": " + std::to_string(getAddress());
    desc += ", ";
    desc += "\"classCode\": " + std::to_string(getClassCode());
    desc += ", ";
    desc += "\"classHex\": \"" + getClassHex() + "\"";
    desc += ", ";
    desc += "\"classDesc\": \"" + getClassDesc() + "\"";
    desc += ", ";
    desc += "\"subClass\": " + std::to_string(getSubClassCode());
    desc += ", ";
    desc += "\"protocol\": " + std::to_string(getProtocolCode());
    desc += ", ";
    desc += "\"speed\": " + std::to_string(getSpeedLevel());
    desc += ", ";
    desc += "\"speedDesc\": \"" + getSpeedDesc() + "\"";
    desc += ",";
    desc += "\n"; /* 换行 */
    desc += allIntendStr + intendStr;
    desc += "\"vid\": \"" + getVid() + "\"";
    desc += ", ";
    desc += "\"pid\": \"" + getPid() + "\"";
    desc += ", ";
    desc += "\"serial\": \"" + getSerial() + "\"";
    desc += ", ";
    desc += "\"product\": \"" + getProduct() + "\"";
    desc += ", ";
    desc += "\"manufacturer\": \"" + getManufacturer() + "\"";
#ifdef _WIN32
    desc += ", ";
    desc += "\"vendor\": \"" + getVendor() + "\"";
    desc += ", ";
    desc += "\"model\": \"" + getModel() + "\"";
    if (isStorage())
    {
        desc += ", ";
        desc += "\"storageType\": \"" + getStorageType() + "\"";
    }
#else
    if (!m_devRootNode.name.empty())
    {
        desc += ",";
        desc += "\n"; /* 换行 */
        desc += allIntendStr + intendStr;
        desc += "\"devRootNode\": ";
        desc += "{";
        desc += "\"name\": \"" + m_devRootNode.name + "\"";
        std::string temp;
        if (!m_devRootNode.group.empty())
        {
            desc += ", ";
            desc += "\"group\": \"" + m_devRootNode.group + "\"";
        }
        if (!m_devRootNode.fstype.empty())
        {
            desc += ", ";
            desc += "\"fstype\": \"" + m_devRootNode.fstype + "\"";
        }
        if (!m_devRootNode.label.empty())
        {
            desc += ", ";
            desc += "\"label\": \"" + m_devRootNode.label + "\"";
        }
        if (!m_devRootNode.partlabel.empty())
        {
            desc += ", ";
            desc += "\"partlabel\": \"" + m_devRootNode.partlabel + "\"";
        }
        if (!m_devRootNode.vendor.empty())
        {
            desc += ", ";
            desc += "\"vendor\": \"" + m_devRootNode.vendor + "\"";
        }
        if (!m_devRootNode.model.empty())
        {
            desc += ", ";
            desc += "\"model\": \"" + m_devRootNode.model + "\"";
        }
        desc += "}";
    }
    if (m_devNodes.size() > 0)
    {
        desc += ",";
        desc += "\n"; /* 换行 */
        desc += allIntendStr + intendStr;
        desc += "\"devNodes\": ";
        desc += "[";
        for (size_t i = 0; i < m_devNodes.size(); ++i)
        {
            if (i > 0)
            {
                desc += ",";
            }
            if (m_devNodes.size() > 1)
            {
                desc += "\n"; /* 换行 */
                desc += allIntendStr + intendStr + intendStr;
            }
            desc += "{";
            desc += "\"name\": \"" + m_devNodes[i].name + "\"";
            std::string temp;
            if (!m_devNodes[i].group.empty())
            {
                desc += ", ";
                desc += "\"group\": \"" + m_devNodes[i].group + "\"";
            }
            if (!m_devNodes[i].fstype.empty())
            {
                desc += ", ";
                desc += "\"type\": \"" + m_devNodes[i].fstype + "\"";
            }
            if (!m_devNodes[i].label.empty())
            {
                desc += ", ";
                desc += "\"label\": \"" + m_devNodes[i].label + "\"";
            }
            if (!m_devNodes[i].partlabel.empty())
            {
                desc += ", ";
                desc += "\"partlabel\": \"" + m_devNodes[i].partlabel + "\"";
            }
            if (!m_devNodes[i].vendor.empty())
            {
                desc += ", ";
                desc += "\"vendor\": \"" + m_devNodes[i].vendor + "\"";
            }
            if (!m_devNodes[i].model.empty())
            {
                desc += ", ";
                desc += "\"model\": \"" + m_devNodes[i].model + "\"";
            }
            desc += "}";
        }
        if (m_devNodes.size() > 1)
        {
            desc += "\n"; /* 换行 */
            desc += allIntendStr + intendStr;
        }
        desc += "]";
    }
#endif
    desc += "\n"; /* 换行 */
    desc += allIntendStr;
    desc += "}";
    return desc;
}

std::vector<Usb> Usb::getAllUsbs(bool detailFlag)
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
        std::vector<UsbImpl> implList;
        if (detailFlag)
        {
#ifdef _WIN32
            /* Windows平台下libusb无法打开设备, 需要通过系统API获取详细信息 */
            implList = enumerateHostControllers();
#else
            /* Linux平台下libusb打开设备可能会卡住, 因此通过读取系统内核文件获取详细信息 */
            implList = enumerateUsbDevices();
            enumerateUsbDevNodes(implList);
#endif
        }
        for (ssize_t i = 0; i < count; ++i) /* 遍历设备列表 */
        {
            Usb info;
            if (parseUsb(devList[i], detailFlag, implList, info))
            {
                usbList.emplace_back(info);
            }
        }
        libusb_free_device_list(devList, 1);
    }
    libusb_exit(NULL);
    /* 排序 */
    std::sort(usbList.begin(), usbList.end(), [](const usb::Usb& a, const usb::Usb& b) {
        if (a.getBusNum() < b.getBusNum())
        {
            return true;
        }
        else if (a.getBusNum() == b.getBusNum())
        {
            if (a.getPortNum() < b.getPortNum())
            {
                return true;
            }
            else if (a.getPortNum() == b.getPortNum())
            {
                return a.getAddress() < b.getAddress();
            }
        }
        return false;
    });
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

bool Usb::parseUsb(libusb_device* dev, bool detailFlag, const std::vector<UsbImpl>& implList, Usb& info)
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
    info.m_subClassCode = desc.bDeviceSubClass;
    info.m_protocolCode = desc.bDeviceProtocol;
    info.m_speedLevel = libusb_get_device_speed(dev); /* 速度等级 */
    char vid[6] = {0}; /* 厂商ID */
    sprintf(vid, "%04x", desc.idVendor);
    info.m_vid = vid;
    char pid[6] = {0}; /* 产品ID */
    sprintf(pid, "%04x", desc.idProduct);
    info.m_pid = pid;
    if (detailFlag) /* 获取详细信息 */
    {
#if 0 /* Windows平台下打不开, Linux平台下有时会卡住, 因此弃而不用 */
        libusb_device_handle* handle;
        if (LIBUSB_SUCCESS == libusb_open(dev, &handle) && handle)
        {
            char serial[256] = {0}; /* 序列号 */
            libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, (unsigned char*)serial, sizeof(serial));
            info.m_serial = serial;
            char product[256] = {0}; /* 产品名称 */
            libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char*)product, sizeof(product));
            info.m_product = product;
            char manufacturer[256] = {0}; /* 厂商名称 */
            libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char*)manufacturer, sizeof(manufacturer));
            info.m_manufacturer = manufacturer;
            libusb_close(handle);
        }
#endif
        /* 获取详细信息 */
        for (const auto& item : implList)
        {
            if (item.busNum == info.m_busNum && item.portNum == info.m_portNum && item.address == info.m_address && item.vid == info.m_vid
                && item.pid == info.m_pid)
            {
                if (info.m_serial.empty())
                {
                    info.m_serial = item.serial;
                }
                if (info.m_product.empty())
                {
                    info.m_product = item.product;
                }
                if (info.m_manufacturer.empty())
                {
                    info.m_manufacturer = item.manufacturer;
                }
#ifdef _WIN32
                info.m_devNodes.emplace_back(DevNode(item.dirverName, item.storageType, "", "", "", item.vendor, item.model));
#else
                info.m_devRootNode = item.devRootNode;
                info.m_devNodes = item.devNodes;
#endif
                break;
            }
        }
    }
    return true;
}
} // namespace usb
