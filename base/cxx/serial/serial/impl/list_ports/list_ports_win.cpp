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
static const DWORD port_name_max_length = 256;
static const DWORD friendly_name_max_length = 256;
static const DWORD hardware_id_max_length = 256;

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
        TCHAR portNameW[port_name_max_length];
        DWORD portNameLen = port_name_max_length;
        LONG ret = RegQueryValueEx(hkey, _T("PortName"), NULL, NULL, (LPBYTE)portNameW, &portNameLen);
        RegCloseKey(hkey);
        if (EXIT_SUCCESS != ret)
        {
            continue;
        }
        if (portNameLen > 0 && portNameLen <= port_name_max_length)
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
        /* 获取端口友好名称 */
        TCHAR friendlyNameW[friendly_name_max_length];
        DWORD friendlyNameActualLen = 0;
        BOOL got_friendly_name = SetupDiGetDeviceRegistryProperty(info, &data, SPDRP_FRIENDLYNAME, NULL, (PBYTE)friendlyNameW,
                                                                  friendly_name_max_length, &friendlyNameActualLen);
        if (got_friendly_name && friendlyNameActualLen > 0)
        {
            friendlyNameW[friendlyNameActualLen - 1] = '\0';
        }
        else
        {
            friendlyNameW[0] = '\0';
        }
        /* 获取硬件ID */
        TCHAR hardwareIdW[hardware_id_max_length];
        DWORD hardwareIdActualLen = 0;
        BOOL gotHardwareId = SetupDiGetDeviceRegistryProperty(info, &data, SPDRP_HARDWAREID, NULL, (PBYTE)hardwareIdW,
                                                              hardware_id_max_length, &hardwareIdActualLen);
        if (gotHardwareId && hardwareIdActualLen > 0)
        {
            hardwareIdW[hardwareIdActualLen - 1] = '\0';
        }
        else
        {
            hardwareIdW[0] = '\0';
        }
#ifdef UNICODE
        std::string portName = utf8Encode(portNameW);
        std::string friendlyName = utf8Encode(friendlyNameW);
        std::string hardwareId = utf8Encode(hardwareIdW);
#else
        std::string portName = portNameW;
        std::string friendlyName = friendlyNameW;
        std::string hardwareId = hardwareIdW;
#endif
        PortInfo entry;
        entry.port = portName;
        entry.description = friendlyName;
        entry.hardwareId = hardwareId;
        /* 插入列表 */
        portList.emplace_back(entry);
    }
    SetupDiDestroyDeviceInfoList(info);
    return portList;
}
} // namespace serial

#endif
