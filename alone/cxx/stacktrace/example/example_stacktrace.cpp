#include "stacktrace/crash_dump.h"

#include <iostream>
#include <string.h>
#ifdef _WIN32
#else
#include <unistd.h>
#endif

int main(int argc, char** argv)
{
    std::string fullProcName;
#ifdef _WIN32
#else
    char buffer[256];
    getcwd(buffer, sizeof(buffer));
    fullProcName.append(buffer).append("/");
#endif
    std::string procName(argv[0]);
    size_t pos = procName.find("/");
    if (std::string::npos == pos)
    {
        fullProcName.append(procName);
    }
    else
    {
        fullProcName.append(procName.substr(pos + 1, procName.size() - pos));
    }
    std::cout << "====" << fullProcName << std::endl;
    stacktrace::CrashDump::open("", fullProcName,
                                [](const std::string& dumpFilename) { std::cout << "dumpFilename: " << dumpFilename << std::endl; });
    char* str = nullptr;
    memset(str, 0, 6);
    return 0;
}
