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
        TCHAR portNameW[max_length];
        DWORD portNameLen = max_length;
        LONG ret = RegQueryValueEx(hkey, _T("PortName"), NULL, NULL, (LPBYTE)portNameW, &portNameLen);
        RegCloseKey(hkey);
        if (EXIT_SUCCESS != ret)
        {
            continue;
        }
        if (portNameLen > 0 && portNameLen <= max_length)
        {
            portNameW[portNameLen - 1] = '\0';
        }
        else
        {
            portNameW[0] = '\0';
        }
        /* 忽略并口 */
        if (_tcsstr(portNameW, _T("LPT")))
        {
            continue;
        }
        /* 获取硬件ID */
        TCHAR hardwareIdW[max_length];
        DWORD hardwareIdActualLen = 0;
        BOOL gotHardwareId =
            SetupDiGetDeviceRegistryProperty(info, &data, SPDRP_HARDWAREID, NULL, (PBYTE)hardwareIdW, max_length, &hardwareIdActualLen);
        if (gotHardwareId && hardwareIdActualLen > 0)
        {
            hardwareIdW[hardwareIdActualLen - 1] = '\0';
        }
        else
        {
            hardwareIdW[0] = '\0';
        }
        /* 获取端口友好名称 */
        TCHAR friendlyNameW[max_length];
        DWORD friendlyNameActualLen = 0;
        BOOL got_friendly_name = SetupDiGetDeviceRegistryProperty(info, &data, SPDRP_FRIENDLYNAME, NULL, (PBYTE)friendlyNameW, max_length,
                                                                  &friendlyNameActualLen);
        if (got_friendly_name && friendlyNameActualLen > 0)
        {
            friendlyNameW[friendlyNameActualLen - 1] = '\0';
        }
        else
        {
            friendlyNameW[0] = '\0';
        }
        /* 获取本地环境属性 */
        TCHAR locationW[max_length];
        DWORD locationActualLen = 0;
        BOOL got_location = SetupDiGetDeviceRegistryProperty(info, &data, SPDRP_LOCATION_INFORMATION, NULL, (PBYTE)locationW, max_length,
                                                             &locationActualLen);
        if (got_location && locationActualLen > 0)
        {
            locationW[locationActualLen - 1] = '\0';
        }
        else
        {
            locationW[0] = '\0';
        }
#ifdef UNICODE
        std::string portName = utf8Encode(portNameW);
        std::string hardwareId = utf8Encode(hardwareIdW);
        std::string friendlyName = utf8Encode(friendlyNameW);
        std::string location = utf8Encode(locationW);
#else
        std::string portName = portNameW;
        std::string hardwareId = hardwareIdW;
        std::string friendlyName = friendlyNameW;
        std::string location = locationW;
#endif
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
