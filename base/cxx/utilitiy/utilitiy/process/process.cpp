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

namespace utilitiy
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

static void SetThreadName(DWORD dwThreadID, const char* threadName)
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
    SetThreadName(-1, name.c_str());
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

char** args2argv(const std::string exeFile, const std::string& args, int& argvCount)
{
    argvCount = 0;
    if (exeFile.empty())
    {
        return NULL;
    }
    std::vector<std::string> vec;
    vec.emplace_back(exeFile);
    std::string seg;
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (' ' == args[i])
        {
            if (!seg.empty())
            {
                vec.emplace_back(seg);
                seg.clear();
            }
        }
        else
        {
            seg.push_back(args[i]);
        }
    }
    if (!seg.empty())
    {
        vec.emplace_back(seg);
        seg.clear();
    }
    argvCount = vec.size();
    char** argv = (char**)malloc(sizeof(char*) * (argvCount + (size_t)1));
    if (argv)
    {
        for (size_t i = 0; i < argvCount; ++i)
        {
            char* arg = (char*)malloc(sizeof(char) * (vec[i].size() + 1));
            if (arg)
            {
                memset(arg, 0, vec[i].size() + 1);
                memcpy(arg, vec[i].c_str(), vec[i].size());
            }
            *(argv + i) = arg;
        }
        *(argv + argvCount) = NULL; /* */
    }
    return argv;
}

std::string Process::getProcessExeFile()
{
#ifdef _WIN32
#ifdef UNICODE
    TCHAR exeFileW[MAX_PATH + 1] = {0};
    GetModuleFileName(NULL, exeFileW, MAX_PATH);
    return wstring2string(exeFileW);
#else
    CHAR exeFile[MAX_PATH + 1] = {0};
    GetModuleFileName(NULL, exeFile, MAX_PATH);
    return exeFile;
#endif
#else
    char exeFile[261] = {0};
    unsigned int exeFileLen = readlink("/proc/self/exe", exeFile, sizeof(exeFile) - 1);
    if (exeFileLen <= 0 || exeFileLen >= sizeof(exeFile) - 1)
    {
        return std::string();
    }
    exeFile[exeFileLen] = '\0';
    return exeFile;
#endif
}

int Process::searchProcess(const std::string& filename, const std::function<bool(const std::string& exeFile, int pid)>& callback)
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
        std::string exeFile;
#ifdef UNICODE
        exeFile = wstring2string(moduleEntry32.szExePath);
#else
        exeFile = moduleEntry32.szExePath;
#endif
        unsigned int exeFileLen = exeFile.size();
        if (filename.size() > exeFileLen)
        {
            continue;
        }
        bool matched = true;
        if (!filename.empty())
        {
            for (unsigned int i = 1, nameLen = filename.size(); i <= nameLen; ++i)
            {
                if (exeFile[exeFileLen - i] != filename[nameLen - i])
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
                if (!callback(exeFile, moduleEntry32.th32ProcessID))
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
    char exeFile[261];
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
        memset(exeFile, 0, sizeof(exeFile));
        unsigned int exeFileLen = readlink(temp, exeFile, sizeof(exeFile) - 1);
        if (exeFileLen <= 0 || exeFileLen >= sizeof(exeFile) - 1)
        {
            continue;
        }
        exeFile[exeFileLen] = '\0';
        if (filename.size() > exeFileLen)
        {
            continue;
        }
        bool matched = true;
        if (!filename.empty())
        {
            for (unsigned int i = 1, nameLen = filename.size(); i <= nameLen; ++i)
            {
                if (exeFile[exeFileLen - i] != filename[nameLen - i])
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
                if (!callback(exeFile, atoi(dirp->d_name)))
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

void Process::runProcess(const std::string& exeFile, const std::string& args, int flag, const std::function<void(int pid)>& callback)
{
    if (exeFile.empty())
    {
        if (callback)
        {
            callback(-1);
        }
        return;
    }
#ifdef _WIN32
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    std::string cmdline = exeFile;
    if (!args.empty())
    {
        cmdline.append(" ").append(args);
    }
#ifdef UNICODE
    std::wstring cmdlineW = string2wstring(cmdline);
    if (cmdlineW.empty())
    {
        if (callback)
        {
            callback(-1);
        }
        return;
    }
    if (!CreateProcess(NULL, (WCHAR*)cmdlineW.c_str(), NULL, NULL, FALSE, (0 == flag ? 0 : CREATE_NEW_CONSOLE), NULL, NULL, &si, &pi))
#else
    if (!CreateProcess(NULL, (CHAR*)(cmdline.c_str()), NULL, NULL, FALSE, (0 == flag ? 0 : CREATE_NEW_CONSOLE), NULL, NULL, &si, &pi))
#endif
    {
        if (callback)
        {
            callback(-1);
        }
        return;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (callback)
    {
        callback(pi.dwProcessId);
    }
#else
    /* 
     * 这里要fork两次, 利用系统孤儿进程的回收机制来处理, 否则会出现僵尸进程.
     * 具体做法是: 利用一代子进程再产生二代子进程, 同时将一代子进程结束掉, 并在父进程中进行收尸处理.
     */
    if (0 != access(exeFile.c_str(), F_OK | R_OK | X_OK)) /* 文件必须具有可读, 可执行权限 */
    {
        if (callback)
        {
            callback(-1);
        }
        return;
    }
    pid_t firstPid = fork(); /* 创建一代子进程 */
    if (firstPid < 0) /* 一代子进程创建失败 */
    {
        if (callback)
        {
            callback(-1);
        }
        return;
    }
    else if (0 == firstPid) /* 创建成功, 此处是一代子进程的代码 */
    {
        pid_t secondPid = fork(); /* 创建二代孙进程 */
        if (secondPid < 0) /* 二代孙进程创建失败 */
        {
            if (callback)
            {
                callback(-1);
            }
            return;
        }
        else if (0 == secondPid) /* 创建成功, 此处是二代孙进程的代码 */
        {
            if (1 == flag)
            {
                fcntl(1, F_SETFD, FD_CLOEXEC); /* 1-关闭标准输出, 子进程的输出将无法显示 */
            }
            if (callback)
            {
                callback((int)getpid());
            }
            int argCount = 0;
            char** argv = args2argv(exeFile, args, argCount);
            if (-1 != execvp(exeFile.c_str(), argv)) /* 在子进程中执行该程序 */
            {
                if (argv) /* 参数内存回收 */
                {
                    for (int i = 0; i < argCount; ++i)
                    {
                        if (argv[i])
                        {
                            free(argv[i]);
                        }
                    }
                    free(argv);
                }
                return; /* 执行完毕直接退出 */
            }
            if (argv) /* 参数内存回收 */
            {
                for (int i = 0; i < argCount; ++i)
                {
                    if (argv[i])
                    {
                        free(argv[i]);
                    }
                }
                free(argv);
            }
            if (callback)
            {
                callback(-1);
            }
            return;
        }
        exit(1); /* 创建成功, 此处是一代子进程的代码 */
    }
    else /* 创建成功, 此处是父进程的代码 */
    {
        if (waitpid(firstPid, NULL, 0) != firstPid) /* 父进程必须为一代子进程收尸 */
        {
        }
    }
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
} // namespace utilitiy
