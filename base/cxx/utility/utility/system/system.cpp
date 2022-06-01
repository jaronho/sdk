#include "system.h"

#include <chrono>
#include <string.h>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace utility
{
#ifdef _WIN32
static std::string wstring2string(const std::wstring& wstr)
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
#endif

int System::runCmd(const std::string& cmd, std::string* outStr, std::vector<std::string>* outVec)
{
    if (outStr)
    {
        (*outStr).clear();
    }
    if (outVec)
    {
        (*outVec).clear();
    }
    if (cmd.empty())
    {
        return -1;
    }
    FILE* stream = NULL;
#ifdef _WIN32
    stream = _popen(cmd.c_str(), "r");
#else
    stream = popen(cmd.c_str(), "r");
#endif
    if (!stream)
    {
        return -2;
    }
    if (outStr || outVec)
    {
        const size_t bufferSize = 1024;
        char buffer[bufferSize] = {0};
        std::string line;
        while (fread(buffer, 1, bufferSize, stream) > 0)
        {
            if (outStr)
            {
                (*outStr).append(buffer);
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
                    (*outVec).emplace_back(line.substr(0, pos));
                    line = line.substr(pos + offset, line.size() - pos - offset);
                }
            }
            memset(buffer, 0, bufferSize);
        }
        if (outVec && !line.empty())
        {
            (*outVec).emplace_back(line);
        }
    }
#ifdef _WIN32
    return _pclose(stream);
#else
    int ret = pclose(stream);
    if (0 != ret && 10 == errno) /* 当进程某处设置了`signal(SIGCHLD, SIG_IGN)`时, 会出现"No child processes", 这里就设置不认为出错 */
    {
        ret = 0;
    }
    return ret;
#endif
}

#ifdef _WIN32
bool System::tryLockUnlockFile(HANDLE fd, bool lock, bool block)
#else
bool System::tryLockUnlockFile(int fd, bool lock, bool block)
#endif
{
    if (fd <= 0) /* 文件不存在 */
    {
        return false;
    }
#ifdef _WIN32
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(OVERLAPPED));
    if (lock)
    {
        if (0 == LockFileEx(fd, block ? LOCKFILE_EXCLUSIVE_LOCK : LOCKFILE_FAIL_IMMEDIATELY, (DWORD)0, (DWORD)0, (DWORD)0, &overlapped))
        {
            return false;
        }
    }
    else
    {
        if (0 == UnlockFileEx(fd, (DWORD)0, (DWORD)0, (DWORD)0, &overlapped))
        {
            return false;
        }
    }
#else
    int lockType = lock ? F_WRLCK : F_UNLCK;
    struct flock fl;
    fl.l_type = lockType; /* 锁的类型 */
    fl.l_whence = SEEK_SET; /* 偏移量的起始位置 */
    fl.l_start = 0; /* 加锁的起始偏移 */
    fl.l_len = 0; /* 上锁的字节数, 为0时表示锁的区域从起点开始直至最大的可能位置 */
    int ret = fcntl(fd, block ? F_SETLKW : F_SETLK, &fl);
    if (0 != ret || lockType != fl.l_type)
    {
        return false;
    }
#endif
    return true;
}

bool System::tryLockFile(const std::string& filename, bool block)
{
    if (filename.empty())
    {
        return false;
    }
    /* 打开要加锁的文件(注意: 这里文件句柄不做关闭处理) */
#ifdef _WIN32
    HANDLE fd = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, (DWORD)0, NULL);
    if (!fd)
    {
        return false;
    }
#else
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0777);
    if (-1 == fd)
    {
        return false;
    }
#endif
    /* 加锁 */
    return tryLockUnlockFile(fd, true, block);
}

bool System::tryLockFileTemporary(const std::string& filename, const std::function<void()>& func, const std::string& suffix)
{
    if (filename.empty())
    {
        return false;
    }
    std::string fileLockName = filename + (suffix.empty() ? ".lock" : ('.' == suffix[0] ? suffix : ("." + suffix)));
    /* 打开要加锁的文件 */
#ifdef _WIN32
    HANDLE fd = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, (DWORD)0, NULL);
    if (!fd)
    {
        return false;
    }
#else
    int fd = open(fileLockName.c_str(), O_RDWR | O_CREAT, 0777);
    if (-1 == fd)
    {
        return false;
    }
#endif
    /* 加锁 */
    tryLockUnlockFile(fd, true, true);
    /* 执行逻辑 */
    if (func)
    {
        func();
    }
    /* 解锁 */
    tryLockUnlockFile(fd, false, true);
    /* 关闭加锁文件 */
#ifdef _WIN32
    CloseHandle(fd);
#else
    close(fd);
#endif
    return true;
}

#ifdef _WIN32
bool System::checkFileLock(HANDLE fd)
#else
bool System::checkFileLock(int fd)
#endif
{
    if (fd <= 0) /* 文件不存在 */
    {
        return false;
    }
#ifdef _WIN32
    /* 尝试解锁 */
    tryLockUnlockFile(fd, false, false);
    /* 获取错误码, 参考: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499- */
    if (0x9E == GetLastError()) /* The segment is already unlocked */
    {
        return false;
    }
#else
    struct flock fl;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fcntl(fd, F_GETLK, &fl);
    if (F_UNLCK == fl.l_type)
    {
        return false;
    }
    return true;
#endif
}

bool System::checkFileLock(const std::string& filename)
{
#ifdef _WIN32
    HANDLE fd = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, (DWORD)0, NULL);
    if (!fd)
    {
        return false;
    }
    bool ret = checkFileLock(fd);
    CloseHandle(fd);
    return ret;
#else
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd <= 0) /* 文件不存在 */
    {
        return false;
    }
    bool ret = checkFileLock(fd);
    close(fd);
    return ret;
#endif
}

void System::waitForTime(unsigned int maxMS, const std::function<bool()>& func, unsigned int loopGap)
{
    if (func)
    {
        auto endTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxMS);
        while (0 == maxMS || std::chrono::steady_clock::now() < endTime)
        {
            if (func())
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(loopGap));
        }
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(maxMS));
    }
}

void System::waitForCount(unsigned int maxCount, const std::function<bool()>& func, unsigned int loopGap)
{
    if (func)
    {
        unsigned int count = 0;
        while (0 == maxCount || count < maxCount)
        {
            if (func())
            {
                break;
            }
            ++count;
            std::this_thread::sleep_for(std::chrono::milliseconds(loopGap));
        }
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(maxCount * loopGap));
    }
}

std::string System::getHostname()
{
#ifdef _WIN32
    TCHAR hostname[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD hostnameLen = MAX_COMPUTERNAME_LENGTH;
    GetComputerName(hostname, &hostnameLen);
#ifdef UNICODE
    return wstring2string(hostname);
#else
    return hostname;
#endif
#else
    char hostname[32] = {0};
    gethostname(hostname, sizeof(hostname));
    return hostname;
#endif
}
} // namespace utility
