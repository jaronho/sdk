#include "stacktrace.h"

#ifdef _WIN32
#else
#include <execinfo.h>
#endif
#include <stdlib.h>
#include <string.h>

namespace stacktrace
{
std::vector<std::string> getStackTrace()
{
    std::vector<std::string> traceList;
#ifdef _WIN32
#else
    const size_t ARRAY_CAPACITY = 64;
    void* traceArray[ARRAY_CAPACITY];
    size_t arraySize = backtrace(traceArray, ARRAY_CAPACITY);
    char** symbolArray = (char**)backtrace_symbols(traceArray, arraySize);
    for (size_t i = 0; i < arraySize; ++i)
    {
        traceList.push_back(symbolArray[i]);
    }
    free(symbolArray);
#endif
    return traceList;
}

#ifndef _WIN32
std::string getAddressDetail(const std::string& procName, const std::string& address)
{
    std::string detail;
    if (address.empty())
    {
        return detail;
    }
    std::string cmd;
    cmd.append("addr2line").append(" -e ").append(procName).append(" ").append(address).append(" -fp");
    FILE* stream;
    stream = popen(cmd.c_str(), "r");
    if (stream)
    {
        char line[1024] = {0};
        while (memset(line, 0, sizeof(line)) && fgets(line, sizeof(line) - 1, stream))
        {
            line[strlen(line) - 1] = '\0';
            if (strlen(line) > 0)
            {
                detail.append(line);
                break;
            }
        }

        pclose(stream);
    }
    return detail;
}
#endif
} // namespace stacktrace
