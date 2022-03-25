#include "app_singleton.h"

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/process/process.h"
#include "utility/system/system.h"

namespace toolkit
{
void AppSingleton::create(const std::string& pidFilePath, const std::string& pidFileName, const std::function<void()>& exitFunc)
{
    static bool s_created = false;
    if (s_created) /* �����ظ����� */
    {
        return;
    }
    s_created = true;
    /* ����pid�ļ�Ŀ¼ */
    utility::FileInfo fi(utility::Process::getProcessExeFile());
    utility::PathInfo pi(pidFilePath.empty() ? fi.path() : pidFilePath, true);
    if (!pi.exist() && !pi.create())
    {
        return;
    }
    /* ��pid�ļ� */
    auto pidFile = pi.path() + (pidFileName.empty() ? fi.filename() + ".pid" : pidFileName);
#ifdef _WIN32
    HANDLE fd = CreateFile(pidFile.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, (DWORD)0, NULL);
    if (!fd)
    {
        return;
    }
#else
    int fd = open(pidFile.c_str(), O_RDWR | O_CREAT, 0777);
    if (-1 == fd)
    {
        return;
    }
#endif
    /* ����pid�ļ� */
    if (!utility::System::tryLockUnlockFile(fd, true, false)) /* ����ʧ��, ��ʾ���н��̴� */
    {
#ifdef _WIN32
        CloseHandle(fd);
#else
        close(fd);
#endif
        if (exitFunc)
        {
            exitFunc();
        }
        exit(0); /* �ر�������� */
    }
    /* д����ID���ļ� */
    std::string pid = std::to_string(utility::Process::getProcessId());
#ifdef _WIN32
    WriteFile(fd, pid.c_str(), pid.size(), (DWORD)0, (DWORD)0);
#else
    write(fd, pid.c_str(), pid.size());
#endif
}
} // namespace toolkit
