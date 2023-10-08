#ifdef __linux__

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <glob.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "../../serial.h"

namespace serial
{
std::string formatStr(const char* format, ...)
{
    va_list ap;
    size_t bufferSize = 256;
    std::string result;
    char* buffer = (char*)malloc(bufferSize);
    if (!buffer)
    {
        return result;
    }
    bool done = false;
    unsigned int loop_count = 0;
    while (!done)
    {
        va_start(ap, format);
        int ret = vsnprintf(buffer, bufferSize, format, ap);
        if (ret < 0)
        {
            done = true;
        }
        else if (ret >= bufferSize)
        {
            /* 重新分配内存并重试 */
            bufferSize = ret + 1;
            char* new_buffer_ptr = (char*)realloc(buffer, bufferSize);
            if (new_buffer_ptr)
            {
                buffer = new_buffer_ptr;
            }
            else
            {
                done = true;
            }
        }
        else
        {
            result = buffer;
            done = true;
        }
        va_end(ap);
        if (++loop_count > 5)
        {
            done = true;
        }
    }
    free(buffer);
    return result;
}

std::string readLine(const std::string& file)
{
    std::ifstream ifs(file.c_str(), std::ifstream::in);
    std::string line;
    if (ifs)
    {
        std::getline(ifs, line);
    }
    return line;
}

std::vector<std::string> getGlobInfo(const std::vector<std::string>& patterns)
{
    std::vector<std::string> deviceList;
    if (patterns.empty())
    {
        return deviceList;
    }
    glob_t glob_results;
    int glob_retval = glob(patterns[0].c_str(), 0, NULL, &glob_results);
    std::vector<std::string>::const_iterator iter = patterns.begin();
    while (patterns.end() != ++iter)
    {
        glob_retval = glob(iter->c_str(), GLOB_APPEND, NULL, &glob_results);
    }
    for (int path_index = 0; path_index < glob_results.gl_pathc; path_index++)
    {
        deviceList.push_back(glob_results.gl_pathv[path_index]);
    }
    globfree(&glob_results);
    return deviceList;
}

std::string getBaseName(const std::string& path)
{
    size_t pos = path.rfind("/");
    if (std::string::npos == pos)
    {
        return path;
    }
    return std::string(path, pos + 1, std::string::npos);
}

std::string getDirName(const std::string& path)
{
    size_t pos = path.rfind("/");
    if (std::string::npos == pos)
    {
        return path;
    }
    else if (0 == pos)
    {
        return "/";
    }
    return std::string(path, 0, pos);
}

bool isPathExist(const std::string& path)
{
    struct stat sb;
    if (0 == stat(path.c_str(), &sb))
    {
        return true;
    }
    return false;
}

std::string getRealPath(const std::string& path)
{
    std::string result;
    char* realPath = ::realpath(path.c_str(), NULL);
    if (realPath)
    {
        result = realPath;
        free(realPath);
    }
    return result;
}

std::string getUsbSysfsFriendlyName(const std::string& sysUsbPath)
{
    unsigned int deviceNum = 0;
    std::istringstream(readLine(sysUsbPath + "/devnum")) >> deviceNum;
    std::string manufacturer = readLine(sysUsbPath + "/manufacturer");
    std::string product = readLine(sysUsbPath + "/product");
    std::string serial = readLine(sysUsbPath + "/serial");
    if (manufacturer.empty() && product.empty() && serial.empty())
    {
        return "";
    }
    return formatStr("%s %s %s", manufacturer.c_str(), product.c_str(), serial.c_str());
}

std::string getUsbSysfsHwString(const std::string& sysfsPath)
{
    std::string serialNumber = readLine(sysfsPath + "/serial");
    if (serialNumber.length() > 0)
    {
        serialNumber = formatStr("SNR=%s", serialNumber.c_str());
    }
    std::string vid = readLine(sysfsPath + "/idVendor");
    std::string pid = readLine(sysfsPath + "/idProduct");
    return formatStr("USB VID:PID=%s:%s %s", vid.c_str(), pid.c_str(), serialNumber.c_str());
}

std::vector<std::string> getSysfsInfo(const std::string& device_path)
{
    std::string deviceName = getBaseName(device_path);
    std::string hardwareId;
    std::string friendlyName;
    std::string location;
    std::string sysDevicePath = formatStr("/sys/class/tty/%s/device", deviceName.c_str());
    if (0 == deviceName.compare(0, 6, "ttyUSB"))
    {
        sysDevicePath = getDirName(getDirName(getRealPath(sysDevicePath)));
        if (isPathExist(sysDevicePath))
        {
            hardwareId = getUsbSysfsHwString(sysDevicePath);
            friendlyName = getUsbSysfsFriendlyName(sysDevicePath);
            location = sysDevicePath;
        }
    }
    else if (0 == deviceName.compare(0, 6, "ttyACM"))
    {
        sysDevicePath = getDirName(getRealPath(sysDevicePath));
        if (isPathExist(sysDevicePath))
        {
            hardwareId = getUsbSysfsHwString(sysDevicePath);
            friendlyName = getUsbSysfsFriendlyName(sysDevicePath);
            location = sysDevicePath;
        }
    }
    else
    {
        /* Try to read ID string of PCI device */
        std::string sysIdPath = sysDevicePath + "/id";
        if (isPathExist(sysIdPath))
        {
            hardwareId = readLine(sysIdPath);
        }
    }
    std::vector<std::string> result;
    result.push_back(hardwareId);
    result.push_back(friendlyName);
    result.push_back(location);
    return result;
}

std::vector<PortInfo> Serial::getAllPorts()
{
    std::vector<PortInfo> portList;
    std::vector<std::string> globList;
    globList.push_back("/dev/ttyACM*");
    globList.push_back("/dev/ttyS*");
    globList.push_back("/dev/ttyUSB*");
    globList.push_back("/dev/tty.*");
    globList.push_back("/dev/cu.*");
    globList.push_back("/dev/rfcomm*");
    std::vector<std::string> deviceList = getGlobInfo(globList);
    std::vector<std::string>::iterator iter = deviceList.begin();
    while (iter != deviceList.end())
    {
        std::string device = *iter++;
        std::vector<std::string> info = getSysfsInfo(device);
        std::string hardwareId = info[0];
        std::string friendlyName = info[1];
        std::string location = info[2];
        PortInfo entry;
        entry.port = device;
        entry.hardwareId = hardwareId;
        entry.description = friendlyName;
        entry.location = location;
        portList.emplace_back(entry);
    }
    return portList;
}
} // namespace serial

#endif
