#include "process.h"

#include <cstdint>
#include <sstream>
#include <string.h>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#pragma warning(disable : 4996)
#else
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <TlHelp32.h> /* 需要放在#include <Windows.h>后面, 否则编译会出错 */
#endif

namespace utility
{
int Process::getProcessId()
{
#ifdef _WIN32
    return _getpid();
#else
    return (int)getpid();
#endif
}

int Process::getThreadId()
{
#ifdef _WIN32
    return GetCurrentThreadId();
#else
    return syscall(__NR_gettid);
#endif
}

bool Process::isValidThreadId(int threadId)
{
#ifdef _WIN32
    if (threadId <= 0)
    {
        return false;
    }
    if (GetCurrentThreadId() == threadId)
    {
        return true;
    }
    bool bThreadAlive = false;
    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, threadId);
    if (hThread)
    {
        DWORD dwExitCode = 0;
        bThreadAlive = GetExitCodeThread(hThread, &dwExitCode) && (STILL_ACTIVE == dwExitCode);
        if (!bThreadAlive)
        {
            dwExitCode = 0;
        }
        CloseHandle(hThread);
    }
    return bThreadAlive;
#else
    return sched_getscheduler(threadId) >= 0;
#endif
}

#ifdef _WIN32
namespace
{
/// See <http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx>
/// and <http://blogs.msdn.com/b/stevejs/archive/2005/12/19/505815.aspx> for
/// more information on the code below.
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void pthread_setname_np(DWORD dwThreadID, const char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}
} /* namespace */
#endif

void Process::setThreadName(const std::string& name)
{
#ifdef _WIN32
    pthread_setname_np(-1, name.c_str());
#else
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
#endif
}

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

static std::wstring string2wstring(const std::string& str)
{
    if (str.empty())
    {
        return std::wstring();
    }
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    wchar_t* buf = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buf, len);
    buf[len] = '\0';
    std::wstring wstr(buf);
    free(buf);
    return wstr;
}
#endif

char** string2argv(const std::string& str, int& argvCount)
{
    std::vector<std::string> argVec;
    std::string arg;
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (' ' == str[i])
        {
            if (!arg.empty())
            {
                argVec.emplace_back(arg);
                arg.clear();
            }
        }
        else
        {
            arg.push_back(str[i]);
        }
    }
    if (!arg.empty())
    {
        argVec.emplace_back(arg);
        arg.clear();
    }
    argvCount = argVec.size();
    char** argv = (char**)malloc(sizeof(char*) * (argvCount + (size_t)1));
    if (argv)
    {
        for (size_t i = 0; i < argvCount; ++i)
        {
            char* arg = (char*)malloc(sizeof(char) * (argVec[i].size() + 1));
            if (arg)
            {
                memset(arg, 0, argVec[i].size() + 1);
                memcpy(arg, argVec[i].c_str(), argVec[i].size());
            }
            *(argv + i) = arg;
        }
        *(argv + argvCount) = NULL; /* 最后一个必须设置为NULL */
    }
    return argv;
}

std::string Process::getProcessExeFile(int pid)
{
#ifdef _WIN32
    if (pid <= 0)
    {
        TCHAR exeFile[MAX_PATH + 1] = {0};
        GetModuleFileName(NULL, exeFile, MAX_PATH);
#ifdef UNICODE
        return wstring2string(exeFile);
#else
        return exeFile;
#endif
    }
    else
    {
        std::string exeFile;
        HANDLE moduleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        if (INVALID_HANDLE_VALUE != moduleSnap)
        {
            MODULEENTRY32 moduleEntry32;
            moduleEntry32.dwSize = sizeof(MODULEENTRY32);
            if (Module32First(moduleSnap, &moduleEntry32))
            {
#ifdef UNICODE
                exeFile = wstring2string(moduleEntry32.szExePath);
#else
                exeFile = moduleEntry32.szExePath;
#endif
            }
            CloseHandle(moduleSnap);
        }
        return exeFile;
    }
#else
    char temp[64] = {0};
    if (pid <= 0)
    {
        sprintf(temp, "/proc/self/exe");
    }
    else
    {
        sprintf(temp, "/proc/%d/exe", pid);
    }
    char exeFile[261] = {0};
    unsigned int exeFileLen = readlink(temp, exeFile, sizeof(exeFile) - 1);
    if (exeFileLen <= 0 || exeFileLen >= sizeof(exeFile) - 1)
    {
        return std::string();
    }
    exeFile[exeFileLen] = '\0';
    return exeFile;
#endif
}

int Process::searchProcess(const std::string& exeFile, const std::function<bool(const std::string& exeFile, int pid)>& callback)
{
    int matchCount = 0;
#ifdef _WIN32
    HANDLE processSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == processSnap)
    {
        return matchCount;
    }
    PROCESSENTRY32 processEntry32;
    processEntry32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(processSnap, &processEntry32))
    {
        CloseHandle(processSnap);
        return matchCount;
    }
    do
    {
        if (0 == processEntry32.th32ProcessID)
        {
            continue;
        }
        HANDLE moduleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processEntry32.th32ProcessID);
        if (INVALID_HANDLE_VALUE == moduleSnap)
        {
            continue;
        }
        MODULEENTRY32 moduleEntry32;
        moduleEntry32.dwSize = sizeof(MODULEENTRY32);
        if (!Module32First(moduleSnap, &moduleEntry32))
        {
            CloseHandle(moduleSnap);
            continue;
        }
        CloseHandle(moduleSnap);
        std::string exeFilePath;
#ifdef UNICODE
        exeFilePath = wstring2string(moduleEntry32.szExePath);
#else
        exeFilePath = moduleEntry32.szExePath;
#endif
        unsigned int exeFilePathLen = exeFilePath.size();
        if (exeFile.size() > exeFilePathLen)
        {
            continue;
        }
        bool matched = true;
        if (!exeFile.empty())
        {
            for (unsigned int i = 1, nameLen = exeFile.size(); i <= nameLen; ++i)
            {
                if (exeFilePath[exeFilePathLen - i] != exeFile[nameLen - i])
                {
                    matched = false;
                    break;
                }
            }
        }
        if (matched)
        {
            ++matchCount;
            if (callback)
            {
                if (!callback(exeFilePath, moduleEntry32.th32ProcessID))
                {
                    break;
                }
            }
        }
    } while (Process32Next(processSnap, &processEntry32));
    CloseHandle(processSnap);
#else
    DIR* dir = opendir("/proc");
    if (!dir)
    {
        return matchCount;
    }
    char temp[64];
    char exeFilePath[261];
    struct dirent* dirp = NULL;
    while ((dirp = readdir(dir)))
    {
        if (DT_DIR != dirp->d_type)
        {
            continue;
        }
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name) || 0 == atoi(dirp->d_name))
        {
            continue;
        }
        memset(temp, 0, sizeof(temp));
        sprintf(temp, "/proc/%s/exe", dirp->d_name);
        memset(exeFilePath, 0, sizeof(exeFilePath));
        unsigned int exeFilePathLen = readlink(temp, exeFilePath, sizeof(exeFilePath) - 1);
        if (exeFilePathLen <= 0 || exeFilePathLen >= sizeof(exeFilePath) - 1)
        {
            continue;
        }
        exeFilePath[exeFilePathLen] = '\0';
        if (exeFile.size() > exeFilePathLen)
        {
            continue;
        }
        bool matched = true;
        if (!exeFile.empty())
        {
            for (unsigned int i = 1, nameLen = exeFile.size(); i <= nameLen; ++i)
            {
                if (exeFilePath[exeFilePathLen - i] != exeFile[nameLen - i])
                {
                    matched = false;
                    break;
                }
            }
        }
        if (matched)
        {
            ++matchCount;
            if (callback)
            {
                if (!callback(exeFilePath, atoi(dirp->d_name)))
                {
                    break;
                }
            }
        }
    }
    closedir(dir);
#endif
    return matchCount;
}

int Process::runProcess(const std::string& exeFile, const std::string& args, int flag)
{
    if (exeFile.empty())
    {
        return -1;
    }
    std::string cmdline = exeFile;
    if (!args.empty())
    {
        cmdline.append(" ").append(args);
    }
#ifdef _WIN32
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
#ifdef UNICODE
    std::wstring cmdlineW = string2wstring(cmdline);
    if (cmdlineW.empty())
    {
        return -1;
    }
    if (!CreateProcess(NULL, (WCHAR*)cmdlineW.c_str(), NULL, NULL, FALSE, (0 == flag ? 0 : CREATE_NEW_CONSOLE), NULL, NULL, &si, &pi))
#else
    if (!CreateProcess(NULL, (CHAR*)(cmdline.c_str()), NULL, NULL, FALSE, (0 == flag ? 0 : CREATE_NEW_CONSOLE), NULL, NULL, &si, &pi))
#endif
    {
        return -1;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return pi.dwProcessId;
#else
    signal(SIGCHLD, SIG_IGN); /* 重要: 设置父进程不关心子进程什么时候结束, 通知内核在子进程结束时进行回收, 避免子进程成为僵尸进程 */
    pid_t pid = fork(); /* 创建子进程 */
    if (pid < 0) /* 子进程创建失败 */
    {
        return pid;
    }
    else if (0 == pid) /* 创建成功, 进入子进程空间 */
    {
        setpgid(0, 0);
        if (1 == flag)
        {
            fcntl(1, F_SETFD, FD_CLOEXEC); /* 1-关闭标准输出, 子进程的输出将无法显示 */
        }
        /* 在子进程中执行该程序 */
        if (args.empty()) /* 无进程参数 */
        {
            execlp(exeFile.c_str(), exeFile.c_str(), NULL);
        }
        else /* 有进程参数 */
        {
            int argvCount;
            char** argv = string2argv(cmdline, argvCount);
            execvp(exeFile.c_str(), argv);
            if (argv)
            {
                for (int i = 0; i < argvCount; ++i)
                {
                    if (argv[i])
                    {
                        free(argv[i]);
                    }
                }
                free(argv);
            }
        }
    }
    /* 父进程空间 */
    return pid;
#endif
}

bool Process::killProcess(int pid)
{
    if (pid <= 0)
    {
        return false;
    }
#ifdef _WIN32
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process)
    {
        return false;
    }
    if (0 == TerminateProcess(process, 0))
    {
        CloseHandle(process);
        return false;
    }
    CloseHandle(process);
#else
    if (0 != kill(pid, SIGKILL))
    {
        return false;
    }
#endif
    return true;
}
} // namespace utility
