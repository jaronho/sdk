#include "crash_dump.h"

#include "stacktrace.h"

#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/timeb.h>

namespace stacktrace
{
/**
 * @brief 获取每条堆栈信息的地址
 * @param traceLine 堆栈信息
 * @return 堆栈地址
 */
std::string getTraceAddress(const std::string& traceLine)
{
    size_t pstart = std::string::npos;
    size_t pend = std::string::npos;
    for (int i = traceLine.size() - 1; i >= 0; --i)
    {
        char ch = traceLine.at(i);
        if ('[' == ch)
        {
            if (std::string::npos == pend)
            {
                continue;
            }
            pstart = i;
            break;
        }
        else if (']' == ch)
        {
            pend = i;
        }
    }
    if (std::string::npos == pstart || std::string::npos == pend)
    {
        return std::string();
    }
    return traceLine.substr(pstart + 1, pend - pstart - 1);
}

#ifndef _WIN32
/**
 * @brief 崩溃捕获句柄
 * @param sigNum 信号值
 */
void catchHandler(int sigNum)
{
    /* step1: 判断程序是否存在 */
    bool isProcExist = false;
    FILE* fp = fopen(CrashDump::m_fullProcName.c_str(), "r");
    if (fp)

    {
        isProcExist = true;
        fclose(fp);
    }
    /* step2: 创建堆栈文件 */
    time_t now;
    time(&now);
    struct tm t = *localtime(&now);
    char datetime[16] = {0};
    strftime(datetime, sizeof(datetime), "%Y%m%d%H%M%S", &t);
    struct timeb tb;
    ftime(&tb);
    char ms[4] = {0};
    sprintf(ms, "%03d", tb.millitm);
    std::string fullDumpFilename;
    fullDumpFilename.append(CrashDump::m_dumpFilePath).append(datetime).append(ms).append(".dump");
    fp = fopen(fullDumpFilename.c_str(), "w");
    /* step3: 获取堆栈信息 */
    std::vector<std::string> traceList = getStackTrace();
    for (size_t i = 0, len = traceList.size(); i < len; ++i)
    {
        std::string traceLine;
        traceLine.append("[").append(std::to_string(i + 1)).append("] ").append(traceList[i]);
        /* step4: 解析出详细信息 */
        std::string traceAddress = getTraceAddress(traceLine);
        if (!traceAddress.empty() && isProcExist)
        {
            std::string detail = getAddressDetail(CrashDump::m_fullProcName, traceAddress);
            if (!detail.empty()) /* 追加详细信息 */
            {
                traceLine.append(" [").append(detail).append("]");
            }
        }
        /* step5: 写堆栈文件 */
        std::cout << traceLine << std::endl;
        if (fp)
        {
            fwrite(traceLine.c_str(), 1, traceLine.size(), fp);
            if (i < len - 1)
            {
                fwrite("\n", 1, 1, fp);
            }
        }
    }
    if (fp)
    {
        fclose(fp);
    }
    /* step6: 回调到外部 */
    if (CrashDump::m_callback)
    {
        CrashDump::m_callback(fullDumpFilename);
    }
    exit(0);
}
#endif

std::string CrashDump::m_dumpFilePath;
std::string CrashDump::m_fullProcName;
FinishCallback CrashDump::m_callback = nullptr;

void CrashDump::open(const std::string& dumpFilePath, const std::string& fullProcName, const FinishCallback& callback)
{
    m_dumpFilePath = dumpFilePath;
    size_t dumpFilePathLen = m_dumpFilePath.size();

    m_fullProcName = fullProcName;
    m_callback = callback;
#ifdef _WIN32
    if (!m_dumpFilePath.empty() && '\\' != m_dumpFilePath.at(dumpFilePathLen - 1))
    {
        m_dumpFilePath.append("\\");
    }
#else
    if (!m_dumpFilePath.empty() && '/' != m_dumpFilePath.at(dumpFilePathLen - 1))
    {
        m_dumpFilePath.append("/");
    }
    signal(SIGSEGV, catchHandler);
    signal(SIGABRT, catchHandler);
#endif
}
} // namespace stacktrace
