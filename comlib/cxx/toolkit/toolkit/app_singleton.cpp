#include "app_singleton.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/process/process.h"
#include "utility/system/system.h"

namespace toolkit
{
bool AppSingleton::create(const std::string& pidFilePath, const std::string& pidFileName,
                          const std::function<void(const std::string& pidFile)>& exitFunc)
{
    static bool s_created = false;
    if (s_created) /* 避免重复创建 */
    {
        return true;
    }
    s_created = true;
    /* 创建pid文件目录 */
    utility::FileInfo fi(utility::Process::getProcessExeFile());
    utility::PathInfo pi(pidFilePath.empty() ? fi.path() : pidFilePath, true);
    if (!pi.exist() && !pi.create())
    {
        return false;
    }
    /* 打开pid文件 */
    auto pidFile = pi.path() + (pidFileName.empty() ? fi.filename() + ".pid" : pidFileName);
#ifdef _WIN32
    HANDLE fd = CreateFile(pidFile.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, (DWORD)0, NULL);
    if (!fd)
    {
        return false;
    }
#else
    int fd = open(pidFile.c_str(), O_RDWR | O_CREAT, 0777);
    if (-1 == fd)
    {
        return false;
    }
#endif
    /* 加锁pid文件 */
    if (!utility::System::tryLockUnlockFile(fd, true, false)) /* 加锁失败, 表示已有进程打开 */
    {
#ifdef _WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif
        if (exitFunc)
        {
            exitFunc(pidFile);
        }
        exit(0); /* 关闭自身进程 */
        return false;
    }
    /* 写进程ID到文件 */
    std::string pid = std::to_string(utility::Process::getProcessId());
#ifdef _WIN32
    DWORD len;
    if (!WriteFile(fd, pid.c_str(), pid.size(), &len, NULL))
    {
        return false;
    }
#else
    if (-1 == write(fd, pid.c_str(), pid.size()))
    {
        return false;
    }
#endif
    return true;
}
} // namespace toolkit
