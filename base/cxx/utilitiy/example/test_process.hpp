#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utilitiy/process/process.h"

void testProcess()
{
    printf("\n============================== test process =============================\n");
    printf("process id: %d\n", utilitiy::Process::getProcessId());
    printf("process exe file: %s\n", utilitiy::Process::getProcessExeFile().c_str());
#ifdef _WIN32
    std::string filename = "example_utilitiy.exe";
#else
    std::string filename = "example_utilitiy";
#endif
    int count = utilitiy::Process::searchProcess(
        filename, [](const std::string& exeFile, int pid) { printf("--- pid: %d, exeFile: %s\n", pid, exeFile.c_str()); });
    printf("----- count: %d\n", count);
}
