/**********************************************************************
* Author:	jaron.ho
* Date:		2018-05-10
* Brief:	process
**********************************************************************/
#include "process.h"
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <Windows.h>
#include <TlHelp32.h>
#endif
#pragma warning(disable: 4996)
//--------------------------------------------------------------------------
static char* wchar2char(const wchar_t* wstr) {
    char* buf = NULL;
    if (!wstr || 0 == wcslen(wstr)) {
        return buf;
    }
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    int len = WideCharToMultiByte(CP_ACP, 0, wstr, wcslen(wstr), NULL, 0, NULL, NULL);
    buf = (char*)malloc(sizeof(char) * (len + 1));
    WideCharToMultiByte(CP_ACP, 0, wstr, wcslen(wstr), buf, len, NULL, NULL);
    buf[len] = '\0';
#endif
    return buf;
}
//--------------------------------------------------------------------------
static wchar_t* char2wchar(const char* str) {
    wchar_t* buf = NULL;
    if (!str || 0 == strlen(str)) {
        return buf;
    }
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    int len = MultiByteToWideChar(CP_ACP, 0, str, strlen(str), NULL, 0);
    buf = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_ACP, 0, str, strlen(str), buf, len);
    buf[len] = '\0';
#endif
    return buf;
}
//--------------------------------------------------------------------------
static int isAbsolutePath(const char* path) {
    if (!path || 0 == strlen(path)) {
        return 0;
    }
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    if (strlen(path) >= 2 && ((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z')) && (':' == path[1])) {
        return 1;
    }
#else
    if (strlen(path) >= 1 && '/' == path[0]) {
        return 1;
    }
#endif
    return 0;
}
//--------------------------------------------------------------------------
static std::string replaceString(std::string str, const std::string& rep, const std::string& dest) {
    if (str.empty() || rep.empty()) {
        return str;
    }
    std::string::size_type pos = 0;
    while (std::string::npos != (pos = str.find(rep, pos))) {
        str.replace(pos, rep.size(), dest);
        pos += dest.size();
    }
    return str;
}
//--------------------------------------------------------------------------
int Process::enablePrivilege(void* process /*= NULL*/, bool enabled /*= true*/) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    if (!process) {
        process = GetCurrentProcess();
    }
    HANDLE token;
    LUID uid;
    /* 获取进程令牌 */
    if (!OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES, &token) || !LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &uid)) {
        return 1;
    }
    TOKEN_PRIVILEGES newState;
    newState.PrivilegeCount = 1;
    newState.Privileges[0].Luid = uid;
    newState.Privileges[0].Attributes = enabled ? SE_PRIVILEGE_ENABLED : 0;
    /* 提示进程权限,注意该函数也可以改变线程的权限,如果hToken指向一个线程的令牌 */
    if (!AdjustTokenPrivileges(token, FALSE, &newState, NULL, NULL, NULL)) {
        return 2;
    }
#endif
    return 0;
}
//--------------------------------------------------------------------------
std::vector<Process> Process::getList(const char* matchExeFile /*= NULL*/) {
    std::vector<Process> ps;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    HANDLE processSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == processSnap) {
        return ps;
    }
    PROCESSENTRY32 processEntry32;
    processEntry32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(processSnap, &processEntry32)) {
        CloseHandle(processSnap);
        return ps;
    }
    do {
        unsigned long processId = processEntry32.th32ProcessID;
        char* exeFile = wchar2char(processEntry32.szExeFile);
        if (matchExeFile && 0 != strlen(matchExeFile)) {
            if (!exeFile) {
                continue;
            }
            if (0 != strcmp(matchExeFile, exeFile)) {
                free(exeFile);
                continue;
            }
        }
        Process p;
        p.id = processId;
        if (exeFile) {
            p.exeFile = exeFile;
            free(exeFile);
        }
        ps.push_back(p);
    } while (Process32Next(processSnap, &processEntry32));
    CloseHandle(processSnap);
#endif
    return ps;
}
//--------------------------------------------------------------------------
std::string Process::getExePath(unsigned long processId) {
    char* exePath = NULL;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    HANDLE moduleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
    if (INVALID_HANDLE_VALUE != moduleSnap) {
        MODULEENTRY32 moduleEntry32;
        moduleEntry32.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(moduleSnap, &moduleEntry32)) {
            exePath = wchar2char(moduleEntry32.szExePath);
        }
        CloseHandle(moduleSnap);
    } else {
        HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        if (process) {
            if (NTDDI_VERSION >= NTDDI_VISTA) {
                TCHAR szExePath[MAX_PATH] = { 0 };
                DWORD szSize = MAX_PATH;
                if (QueryFullProcessImageName(process, 0, szExePath, &szSize)) {
                    exePath = wchar2char(szExePath);
                }
            }
            CloseHandle(process);
        }
    }
#endif
    std::string processExePath;
    if (exePath) {
        processExePath = exePath;
        free(exePath);
        unsigned int pos = processExePath.find_last_of("\\/");
        if (std::string::npos != pos) {
            processExePath = processExePath.substr(0, pos + 1);
        }
    }
    return processExePath;
}
//--------------------------------------------------------------------------
int Process::kill(unsigned long processId) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!process) {
        return 1;
    }
    if (0 == TerminateProcess(process, 0)) {
        return 2;
    }
    CloseHandle(process);
#endif
    return 0;
}
//--------------------------------------------------------------------------
void Process::killApp(const char* appName) {
    if (!appName || 0 == strlen(appName)) {
        return;
    }
    std::string applicationName = appName;
    bool matchFullName = false;
    if (isAbsolutePath(appName)) {
        matchFullName = true;
        applicationName = replaceString(applicationName, "\\", "/");
    } else {
        matchFullName = false;
        unsigned int pos = applicationName.find_last_of("\\/");
        if (std::string::npos != pos) {
            applicationName = applicationName.substr(pos);
        }
    }
    std::vector<Process> ps = getList(NULL);
    for (unsigned int i = 0, len = ps.size(); i < len; ++i) {
        if (matchFullName) {
            if (applicationName == replaceString(ps[i].exePath(), "\\", "/") + ps[i].exeFile) {
                kill(ps[i].id);
            }
        } else {
            if (applicationName == ps[i].exeFile) {
                kill(ps[i].id);
            }
        }
    }
}
//--------------------------------------------------------------------------
int Process::runApp(const char* appName, const char* workingDir /*= NULL*/) {
    if (!appName || 0 == strlen(appName)) {
        return 1;
    }
    if (!isAbsolutePath(appName)) {
        return 2;
    }
    if (workingDir && !isAbsolutePath(workingDir)) {
        return 3;
    }
    std::string appWorkingDir;
    if (workingDir) {
        appWorkingDir = workingDir;
    } else {
        appWorkingDir = appName;
        unsigned int pos = appWorkingDir.find_last_of("\\/");
        if (std::string::npos != pos) {
            appWorkingDir = appWorkingDir.substr(0, pos + 1);
        }
    }
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    wchar_t* appNameW = char2wchar(appName);
    if (!appNameW) {
        return 1;
    }
    wchar_t* workingDirW = char2wchar(appWorkingDir.c_str());
    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(PROCESS_INFORMATION));
    if (!CreateProcess(appNameW, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, workingDirW, &si, &pi)) {
        free(appNameW);
        if (workingDirW) {
            free(workingDirW);
        }
        return 4;
    }
    free(appNameW);
    if (workingDirW) {
        free(workingDirW);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
#endif
    return 0;
}
//--------------------------------------------------------------------------
int Process::isAppFileExist(const char* appName) {
    if (!appName || 0 == strlen(appName)) {
        return 1;
    }
    FILE* fp = fopen(appName, "r");
    if (!fp) {
        return 2;
    }
    fclose(fp);
    return 0;
}
//--------------------------------------------------------------------------
const std::string& Process::exePath(void) {
    if (mExePath.empty()) {
        mExePath = getExePath(id);
    }
    return mExePath;
}
//--------------------------------------------------------------------------