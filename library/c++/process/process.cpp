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
static char* wchar2char(const wchar_t* wp) {
    char* buf = NULL;
    if (!wp || 0 == wcslen(wp)) {
        return buf;
    }
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    int len = WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
    buf = (char*)malloc(len + 1);
    WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), buf, len, NULL, NULL);
    buf[len] = '\0';
#endif
    return buf;
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
        if (NTDDI_VERSION >= NTDDI_VISTA) {
            HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
            if (process) {
                TCHAR szExePath[MAX_PATH] = { 0 };
                DWORD szSize = MAX_PATH;
                if (QueryFullProcessImageName(process, 0, szExePath, &szSize)) {
                    exePath = wchar2char(szExePath);
                }
                CloseHandle(process);
            }
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
std::string Process::exePath(void) {
    return getExePath(id);
}
//--------------------------------------------------------------------------