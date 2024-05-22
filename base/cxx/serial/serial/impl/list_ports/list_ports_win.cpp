#ifdef _WIN32

#include <Windows.h>
#include <tchar.h>
/* SetupAPI.h需要在Windows.h后包含 */
#include <SetupAPI.h>
#include <cstring>
#include <devguid.h>
#include <initguid.h>

#include "../../serial.h"

#pragma comment(lib, "setupapi.lib")

namespace serial
{
static const DWORD max_length = 256;

/* Convert a wide Unicode string to an UTF8 string */
std::string utf8Encode(const std::wstring& wstr)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], len, NULL, NULL);
    return str;
}

std::vector<PortInfo> Serial::getAllPorts()
{
    std::vector<PortInfo> portList;
    HDEVINFO info = SetupDiGetClassDevs((const GUID*)&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
    unsigned int index = 0;
    SP_DEVINFO_DATA data;
    data.cbSize = sizeof(SP_DEVINFO_DATA);
    while (SetupDiEnumDeviceInfo(info, index, &data))
    {
        ++index;
        /* 获取端口 */
        HKEY hkey = SetupDiOpenDevRegKey(info, &data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        TCHAR portNamePtr[max_length];
        DWORD portNameLen = max_length;
        LONG ret = RegQueryValueExA(hkey, _T("PortName"), NULL, NULL, (LPBYTE)portNamePtr, &portNameLen);
        RegCloseKey(hkey);
        if (EXIT_SUCCESS != ret)
        {
            continue;
        }
        if (portNameLen > 0 && portNameLen <= max_length)
        {
            portNamePtr[portNameLen - 1] = '\0';
        }
        else
        {
            portNamePtr[0] = '\0';
        }
        /* 忽略并口 */
        if (_tcsstr(portNamePtr, _T("LPT")))
        {
            continue;
        }
        /* 获取硬件ID */
        TCHAR hardwareIdPtr[max_length];
        DWORD hardwareIdActualLen = 0;
        BOOL gotHardwareId =
            SetupDiGetDeviceRegistryPropertyA(info, &data, SPDRP_HARDWAREID, NULL, (PBYTE)hardwareIdPtr, max_length, &hardwareIdActualLen);
        if (gotHardwareId && hardwareIdActualLen > 0)
        {
            hardwareIdPtr[hardwareIdActualLen - 1] = '\0';
        }
        else
        {
            hardwareIdPtr[0] = '\0';
        }
        /* 获取端口友好名称 */
        TCHAR friendlyNamePtr[max_length];
        DWORD friendlyNameActualLen = 0;
        BOOL got_friendly_name = SetupDiGetDeviceRegistryPropertyA(info, &data, SPDRP_FRIENDLYNAME, NULL, (PBYTE)friendlyNamePtr,
                                                                   max_length, &friendlyNameActualLen);
        if (got_friendly_name && friendlyNameActualLen > 0)
        {
            friendlyNamePtr[friendlyNameActualLen - 1] = '\0';
        }
        else
        {
            friendlyNamePtr[0] = '\0';
        }
        /* 获取本地环境属性 */
        TCHAR locationPtr[max_length];
        DWORD locationActualLen = 0;
        BOOL got_location = SetupDiGetDeviceRegistryPropertyA(info, &data, SPDRP_LOCATION_INFORMATION, NULL, (PBYTE)locationPtr, max_length,
                                                              &locationActualLen);
        if (got_location && locationActualLen > 0)
        {
            locationPtr[locationActualLen - 1] = '\0';
        }
        else
        {
            locationPtr[0] = '\0';
        }
        std::string portName = portNamePtr;
        std::string hardwareId = hardwareIdPtr;
        std::string friendlyName = friendlyNamePtr;
        std::string location = locationPtr;
        PortInfo entry;
        entry.port = portName;
        entry.hardwareId = hardwareId;
        entry.description = friendlyName;
        entry.location = location;
        /* 插入列表 */
        portList.emplace_back(entry);
    }
    SetupDiDestroyDeviceInfoList(info);
    return portList;
}
} // namespace serial

#endif
