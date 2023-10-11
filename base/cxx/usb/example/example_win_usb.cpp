// Windows.h必须比其他平台文件先包含
#include <Windows.h>
// initguid.h必须在devpkey.h前面包含
#include <initguid.h>
//
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <devpkey.h>
#include <stdio.h>
#include <string>
#include <tchar.h>

#pragma comment(lib, "setupapi.lib")

std::string wstring2string(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    char* buf = (char*)malloc(sizeof(char) * (len + 1));
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buf, len, NULL, NULL);
    buf[len] = '\0';
    std::string str(buf);
    free(buf);
    return str;
}

void printTextInfo(const char* head, WCHAR PropertyBuffer[4096], DWORD RequiredSize)
{
    printf("    %s: ", head);
    std::wstring buf;
    for (auto i = 0; i < (int)RequiredSize; ++i)
    {
        if ('\0' == PropertyBuffer[i])
        {
            break;
        }
        buf.push_back(PropertyBuffer[i]);
    }
    printf("%s", wstring2string(buf).c_str());
    printf("\n");
}

void printNumberInfo(const char* head, WCHAR PropertyBuffer[4096], DWORD RequiredSize)
{
    printf("    %s: ", head);
    for (auto i = 0; i < (int)RequiredSize; ++i)
    {
        if ('\0' == PropertyBuffer[i])
        {
            break;
        }
        printf("%d", PropertyBuffer[i]);
    }
    printf("\n");
}

void printGuidInfo(const char* head, WCHAR PropertyBuffer[4096], DWORD RequiredSize)
{
    WCHAR szBuffer[4096];
    StringFromGUID2((REFGUID)PropertyBuffer, szBuffer, sizeof(szBuffer) / sizeof(WCHAR));
    printTextInfo(head, szBuffer, RequiredSize);
}

int main()
{
    HDEVINFO DeviceInfoSet = SetupDiGetClassDevsW(NULL, L"USB", NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (DeviceInfoSet == INVALID_HANDLE_VALUE)
        return 0;
    int i = 0;
    for (i = 0;; i++)
    {
        SP_DEVINFO_DATA DeviceInfoData;
        DeviceInfoData.cbSize = sizeof(DeviceInfoData);
        if (!SetupDiEnumDeviceInfo(DeviceInfoSet, i, &DeviceInfoData))
        {
            break;
        }
        TCHAR szDeviceInstanceID[MAX_DEVICE_ID_LEN];
        CONFIGRET status = CM_Get_Device_ID(DeviceInfoData.DevInst, szDeviceInstanceID, MAX_PATH, 0);
        if (CR_SUCCESS != status)
        {
            continue;
        }
        printf("=============================\n");
        _tprintf(TEXT("%s\n"), szDeviceInstanceID);
        DWORD RequiredSize = 0;
        DEVPROPTYPE PropertyType = 0;
        WCHAR PropertyBuffer[4096] = {0};
        /* NAME */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_NAME, &PropertyType, reinterpret_cast<PBYTE>(PropertyBuffer),
                                      sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("NAME", PropertyBuffer, RequiredSize);
        }
        /* DeviceDesc */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DeviceDesc, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DeviceDesc", PropertyBuffer, RequiredSize);
        }
        /* HardwareIds */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_HardwareIds, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("HardwareIds", PropertyBuffer, RequiredSize);
        }
        /* CompatibleIds */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_CompatibleIds, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("CompatibleIds", PropertyBuffer, RequiredSize);
        }
        /* Service */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Service, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("Service", PropertyBuffer, RequiredSize);
        }
        /* Class */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Class, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("Class", PropertyBuffer, RequiredSize);
        }
        /* ClassGuid */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ClassGuid, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printGuidInfo("ClassGuid", PropertyBuffer, RequiredSize);
        }
        /* Driver */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Driver, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("Driver", PropertyBuffer, RequiredSize);
        }
        /* ConfigFlags */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ConfigFlags, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("ConfigFlags", PropertyBuffer, RequiredSize);
        }
        /* Manufacturer */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Manufacturer, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("Manufacturer", PropertyBuffer, RequiredSize);
        }
        /* FriendlyName */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_FriendlyName, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("FriendlyName", PropertyBuffer, RequiredSize);
        }
        /* LocationInfo */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_LocationInfo, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("LocationInfo", PropertyBuffer, RequiredSize);
        }
        /* PDOName */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_PDOName, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("PDOName", PropertyBuffer, RequiredSize);
        }
        /* Capabilities */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Capabilities, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("Capabilities", PropertyBuffer, RequiredSize);
        }
        /* UINumber */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_UINumber, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("UINumber", PropertyBuffer, RequiredSize);
        }
        /* UpperFilters */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_UpperFilters, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("UpperFilters", PropertyBuffer, RequiredSize);
        }
        /* LowerFilters */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_LowerFilters, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("LowerFilters", PropertyBuffer, RequiredSize);
        }
        /* BusTypeGuid */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_BusTypeGuid, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printGuidInfo("BusTypeGuid", PropertyBuffer, RequiredSize);
        }
        /* LegacyBusType */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_LegacyBusType, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("LegacyBusType", PropertyBuffer, RequiredSize);
        }
        /* BusNumber */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_BusNumber, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("BusNumber", PropertyBuffer, RequiredSize);
        }
        /* EnumeratorName */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_EnumeratorName, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("EnumeratorName", PropertyBuffer, RequiredSize);
        }
        /* DevType */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DevType, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("DevType", PropertyBuffer, RequiredSize);
        }
        /* Characteristics */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Characteristics, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("Characteristics", PropertyBuffer, RequiredSize);
        }
        /* Address */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Address, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("Address", PropertyBuffer, RequiredSize);
        }
        /* UINumberDescFormat */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_UINumberDescFormat, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("UINumberDescFormat", PropertyBuffer, RequiredSize);
        }
        /* RemovalPolicy */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_RemovalPolicy, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("RemovalPolicy", PropertyBuffer, RequiredSize);
        }
        /* RemovalPolicyDefault */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_RemovalPolicyDefault, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("RemovalPolicyDefault", PropertyBuffer, RequiredSize);
        }
        /* RemovalPolicyOverride */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_RemovalPolicyOverride, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("RemovalPolicyOverride", PropertyBuffer, RequiredSize);
        }
        /* InstallState */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_InstallState, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("InstallState", PropertyBuffer, RequiredSize);
        }
        /* LocationPaths */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_LocationPaths, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("LocationPaths", PropertyBuffer, RequiredSize);
        }
        /* BaseContainerId */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_BaseContainerId, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printGuidInfo("BaseContainerId", PropertyBuffer, RequiredSize);
        }
        /* InstanceId */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_InstanceId, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("InstanceId", PropertyBuffer, RequiredSize);
        }
        /* DevNodeStatus */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DevNodeStatus, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("DevNodeStatus", PropertyBuffer, RequiredSize);
        }
        /* ProblemCode */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ProblemCode, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("ProblemCode", PropertyBuffer, RequiredSize);
        }
        /* EjectionRelations */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_EjectionRelations, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("EjectionRelations", PropertyBuffer, RequiredSize);
        }
        /* RemovalRelations */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_RemovalRelations, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("RemovalRelations", PropertyBuffer, RequiredSize);
        }
        /* PowerRelations */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_PowerRelations, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("PowerRelations", PropertyBuffer, RequiredSize);
        }
        /* BusRelations */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_BusRelations, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("BusRelations", PropertyBuffer, RequiredSize);
        }
        /* Parent */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Parent, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("Parent", PropertyBuffer, RequiredSize);
        }
        /* Children */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Children, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("Children", PropertyBuffer, RequiredSize);
        }
        /* Siblings */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Siblings, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("Siblings", PropertyBuffer, RequiredSize);
        }
        /* TransportRelations */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_TransportRelations, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("TransportRelations", PropertyBuffer, RequiredSize);
        }
        /* ContainerId */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ContainerId, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printGuidInfo("ContainerId", PropertyBuffer, RequiredSize);
        }
        /* Model */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Model, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("Model", PropertyBuffer, RequiredSize);
        }
        /* ModelId */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ModelId, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printGuidInfo("ModelId", PropertyBuffer, RequiredSize);
        }
        /* FriendlyNameAttribute */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_FriendlyNameAttributes, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("FriendlyNameAttribute", PropertyBuffer, RequiredSize);
        }
        /* ManufacturerAttributes */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ManufacturerAttributes, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("ManufacturerAttributes", PropertyBuffer, RequiredSize);
        }
        /* SignalStrength */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_SignalStrength, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("SignalStrength", PropertyBuffer, RequiredSize);
        }
        /* Numa_Proximity_Domain */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Numa_Proximity_Domain, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("Numa_Proximity_Domain", PropertyBuffer, RequiredSize);
        }
        /* DHP_Rebalance_Policy */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DHP_Rebalance_Policy, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("DHP_Rebalance_Policy", PropertyBuffer, RequiredSize);
        }
        /* Numa_Node */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Numa_Node, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("Numa_Node", PropertyBuffer, RequiredSize);
        }
        /* BusReportedDeviceDesc */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_BusReportedDeviceDesc, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("BusReportedDeviceDesc", PropertyBuffer, RequiredSize);
        }
        /* ConfigurationId */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ConfigurationId, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("ConfigurationId", PropertyBuffer, RequiredSize);
        }
        /* ReportedDeviceIdsHash */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ReportedDeviceIdsHash, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("ReportedDeviceIdsHash", PropertyBuffer, RequiredSize);
        }
        /* BiosDeviceName */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_BiosDeviceName, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("BiosDeviceName", PropertyBuffer, RequiredSize);
        }
        /* DriverProblemDesc */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverProblemDesc, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DriverProblemDesc", PropertyBuffer, RequiredSize);
        }
        /* DebuggerSafe */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DebuggerSafe, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("DebuggerSafe", PropertyBuffer, RequiredSize);
        }
        /* Stack */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_Stack, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("Stack", PropertyBuffer, RequiredSize);
        }
        /* ExtendedConfigurationIds */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ExtendedConfigurationIds, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("ExtendedConfigurationIds", PropertyBuffer, RequiredSize);
        }
        /* FirmwareVersion */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_FirmwareVersion, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("FirmwareVersion", PropertyBuffer, RequiredSize);
        }
        /* FirmwareRevision */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_FirmwareRevision, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("FirmwareRevision", PropertyBuffer, RequiredSize);
        }
        /* DependencyProviders */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DependencyProviders, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DependencyProviders", PropertyBuffer, RequiredSize);
        }
        /* DependencyDependents */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DependencyDependents, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DependencyDependents", PropertyBuffer, RequiredSize);
        }
        /* ExtendedAddress */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ExtendedAddress, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("ExtendedAddress", PropertyBuffer, RequiredSize);
        }
        /* SessionId */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_SessionId, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("SessionId", PropertyBuffer, RequiredSize);
        }
        /* DriverVersion */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverVersion, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DriverVersion", PropertyBuffer, RequiredSize);
        }
        /* DriverDesc */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverDesc, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DriverDesc", PropertyBuffer, RequiredSize);
        }
        /* DriverInfPath */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverInfPath, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DriverInfPath", PropertyBuffer, RequiredSize);
        }
        /* DriverInfSection */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverInfSection, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DriverInfSection", PropertyBuffer, RequiredSize);
        }
        /* DriverInfSectionExt */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverInfSectionExt, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DriverInfSectionExt", PropertyBuffer, RequiredSize);
        }
        /* MatchingDeviceId */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_MatchingDeviceId, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("MatchingDeviceId", PropertyBuffer, RequiredSize);
        }
        /* DriverProvider */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverProvider, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DriverProvider", PropertyBuffer, RequiredSize);
        }
        /* DriverPropPageProvider */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverPropPageProvider, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DriverPropPageProvider", PropertyBuffer, RequiredSize);
        }
        /* DriverCoInstallers */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverCoInstallers, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("DriverCoInstallers", PropertyBuffer, RequiredSize);
        }
        /* ResourcePickerTags */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ResourcePickerTags, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("ResourcePickerTags", PropertyBuffer, RequiredSize);
        }
        /* ResourcePickerExceptions */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_ResourcePickerExceptions, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printTextInfo("ResourcePickerExceptions", PropertyBuffer, RequiredSize);
        }
        /* DriverRank */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverRank, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("DriverRank", PropertyBuffer, RequiredSize);
        }
        /* DriverLogoLevel */
        memset(PropertyBuffer, 0, sizeof(PropertyBuffer));
        if (SetupDiGetDevicePropertyW(DeviceInfoSet, &DeviceInfoData, &DEVPKEY_Device_DriverLogoLevel, &PropertyType,
                                      reinterpret_cast<PBYTE>(PropertyBuffer), sizeof(PropertyBuffer), &RequiredSize, 0))
        {
            printNumberInfo("DriverLogoLevel", PropertyBuffer, RequiredSize);
        }
    }
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    return 0;
}