/**********************************************************************
* Author:	jaron.ho
* Date:		2018-05-10
* Brief:	logfile
**********************************************************************/
#include "daemon.h"
#ifdef WIN32
#include <TlHelp32.h>
#include <Windows.h>
#endif
//--------------------------------------------------------------------------
static char* wchar2char(const wchar_t* wp) {
    if (!wp || 0 == wcslen(wp)) {
        return NULL;
    }
    int len = WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
    char* buf = (char*)malloc(len + 1);
    WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), buf, len, NULL, NULL);
    buf[len] = '\0';
    return buf;
}
//--------------------------------------------------------------------------
std::vector<ProcessInfo> Daemon::getProcessInfos(const char* matchExeFile = /*NULL*/, const char* matchExePath = /*NULL*/, bool queryExePath = /*true*/) {
    std::vector<ProcessInfo> pis;
    HANDLE processSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == processSnap) {
        return pis;
    }
    PROCESSENTRY32 processEntry32;
    processEntry32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(processSnap, &processEntry32)) {
        CloseHandle(processSnap);
        return pis;
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
        ProcessInfo pi;
        pi.processId = processId;
        if (exeFile) {
            pi.exeFile = exeFile;
            free(exeFile);
        }
        if (queryExePath) {
            char* exePath = NULL;
            HANDLE moduleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processEntry32.th32ProcessID);
            if (INVALID_HANDLE_VALUE != moduleSnap) {
                MODULEENTRY32 moduleEntry32;
                moduleEntry32.dwSize = sizeof(MODULEENTRY32);
                if (Module32First(moduleSnap, &moduleEntry32)) {
                    exePath = wchar2char(moduleEntry32.szExePath);
                }
                CloseHandle(moduleSnap);
            } else {
                static unsigned long dwVer = 0;
                if (0 == dwVer) {
                    OSVERSIONINFO os;
                    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                    if (GetVersionEx(&os)) {
                        dwVer = os.dwMajorVersion;
                    }
                }
                if (dwVer >= 6) {   /* above windows vista */
                    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processEntry32.th32ProcessID);
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
            if (exePath) {
                if (matchExePath && 0 != strlen(matchExePath)) {
                    if (0 != strcmp(matchExePath, exePath)) {
                        free(exePath);
                        continue;
                    }
                }
                pi.exePath = exePath;
                free(exePath);
                unsigned int p = pi.exePath.find_last_of("\\/");
                if (std::string::npos != p) {
                    pi.exePath = pi.exePath.substr(0, p + 1);
                }
            }
        }
        pis.push_back(pi);
    } while (Process32Next(processSnap, &processEntry32));
    CloseHandle(processSnap);
    return pis;
}
//--------------------------------------------------------------------------