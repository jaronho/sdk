#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utility/process/process.h"

void testProcess()
{
    printf("\n============================== test process =============================\n");
    printf("process id: %d\n", utility::Process::getProcessId());
    printf("process exe file: %s\n", utility::Process::getProcessExeFile().c_str());
#ifdef _WIN32
    std::string filename = "example_utilitiy.exe";
#else
    std::string filename = "example_utilitiy";
#endif
    int count = utility::Process::searchProcess(filename, [](const std::string& exeFile, int pid, int ppid) {
        printf("--- pid: %d, ppid: %d, exeFile: %s\n", pid, ppid, exeFile.c_str());
        return true;
    });
    printf("----- count: %d\n", count);
}
