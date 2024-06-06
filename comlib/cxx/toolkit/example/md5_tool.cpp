#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "../toolkit/tool.h"
#include "algorithm/md5/md5.h"
#include "threading/thread_proxy.hpp"
#include "utility/charset/charset.h"
#include "utility/cmdline/cmdline.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

/* 转换字节到合适的单位 */
static std::string convertBytesToAppropriateUnit(double bytes)
{
    /* 定义字节到KB, MB, GB的转换常量 */
    static const double BYTES_IN_KB = 1024.0;
    static const double BYTES_IN_MB = BYTES_IN_KB * 1024.0;
    static const double BYTES_IN_GB = BYTES_IN_MB * 1024.0;
    std::ostringstream oss;
    if (bytes < BYTES_IN_KB) /* 如果字节数小于1KB, 直接以字节为单位显示 */
    {
        return std::to_string((size_t)bytes) + " B";
    }
    else if (bytes < BYTES_IN_MB) /* 如果字节数小于1MB，以KB为单位显示 */
    {
        oss << std::fixed << std::setprecision(2) << (bytes / BYTES_IN_KB);
        return oss.str() + " KB";
    }
    else if (bytes < BYTES_IN_GB) /* 如果字节数小于1GB，以MB为单位显示 */
    {
        oss << std::fixed << std::setprecision(2) << (bytes / BYTES_IN_MB);
        return oss.str() + " MB";
    }
    /* 如果字节数大于或等于1GB，以GB为单位显示 */
    oss << std::fixed << std::setprecision(2) << (bytes / BYTES_IN_GB);
    return oss.str() + " GB";
}

static std::string dtString()
{
    return utility::DateTime::getNow().yyyyMMddhhmmss("-", " ", ":", ".");
}

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    cmdline::parser parser;
    parser.add<std::string>("input", 'i', "输入指定文件路径或目录路径", false, "");
    parser.add<int>("thread", 't', "并发计算线程数量, 小等于1表示单线程, 默认: 1", false, 1);
    parser.add<int>("block", 'b', "每次读文件的块大小(字节), 默认: 1Mb", false, 1024 * 1024);
    parser.add("verbose", 'v', "显示进度信息");
    parser.add("help", 'h', "显示帮助信息");
    parser.parse_check(argc, argv);
    auto target = parser.get<std::string>("input");
    if (target.empty() || parser.exist("help"))
    {
        printf("%s\n", parser.usage().c_str());
        return 0;
    }
    auto threadCount = parser.get<int>("thread");
    threadCount = threadCount >= 0 ? threadCount : 0;
    auto blockSize = parser.get<int>("block");
    utility::FileAttribute attr;
    utility::getFileAttribute(target, attr);
    std::string value;
    if (attr.isDir) /* 目录 */
    {
        threading::ExecutorPtr calcExecutor = nullptr;
        if (threadCount > 1)
        {
            calcExecutor = threading::ThreadProxy::createAsioExecutor("calc", threadCount);
        }
        if (parser.exist("verbose"))
        {
            printf("[%s] 开始计算文件数量和大小\n", dtString().c_str());
            size_t totalFileCount = 0, totalFileSize = 0;
            std::atomic_size_t nowCount = {0};
            utility::PathInfo pi(target, true);
            auto tp = std::chrono::steady_clock::now();
            value = toolkit::Tool::md5Directory(
                target,
                [&](size_t totalCount, size_t totalSize) {
                    printf("[%s] 文件总数量: %zu, 文件总大小: %s\n", dtString().c_str(), totalCount,
                           convertBytesToAppropriateUnit(totalSize).c_str());
                    totalFileCount = totalCount;
                    totalFileSize = totalSize;
                },
                [&, calcExecutor](const std::string& name, size_t fileSize, const std::function<std::string()>& calcFunc) {
                    auto relativeName = utility::StrTool::replace(name.substr(pi.path().size()), "\\", "/");
                    if (!relativeName.empty() && '/' == relativeName[0])
                    {
                        relativeName.erase(0, 1);
                    }
                    if (utility::Charset::Coding::gbk == utility::Charset::getCoding(relativeName))
                    {
                        relativeName = utility::Charset::gbkToUtf8(relativeName);
                    }
                    if (calcExecutor)
                    {
                        calcExecutor->post("calc", [&, totalFileCount, relativeName, fileSize, calcFunc]() {
                            auto totalCountStr = std::to_string(totalFileCount);
                            std::string placeStr(totalCountStr.size() * 2 + 3, '-');
                            auto fileDesc = relativeName + " (" + convertBytesToAppropriateUnit(fileSize) + ")";
                            printf("[%s] %s %s\n", dtString().c_str(), placeStr.c_str(), fileDesc.c_str());
                            calcFunc();
                            nowCount += 1;
                            auto nowCountStr = std::to_string(nowCount.load());
                            auto progress =
                                "[" + utility::StrTool::fillPlace(nowCountStr, ' ', totalCountStr.size()) + "/" + totalCountStr + "]";
                            printf("[%s] %s %s\n", dtString().c_str(), progress.c_str(), fileDesc.c_str());
                        });
                    }
                    else
                    {
                        nowCount += 1;
                        auto totalCountStr = std::to_string(totalFileCount);
                        auto nowCountStr = std::to_string(nowCount.load());
                        auto progress =
                            "[" + utility::StrTool::fillPlace(nowCountStr, ' ', totalCountStr.size()) + "/" + totalCountStr + "]";
                        auto fileDesc = relativeName + " (" + convertBytesToAppropriateUnit(fileSize) + ")";
                        printf("[%s] %s %s\n", dtString().c_str(), progress.c_str(), fileDesc.c_str());
                        calcFunc();
                    }
                },
                nullptr, blockSize);
            printf("\n");
            if (!value.empty())
            {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp).count();
                std::string timeStr;
                if (elapsed < 1000) /* 1秒内 */
                {
                    timeStr = std::to_string(elapsed) + " ms";
                }
                else if (elapsed < 60000) /* 1分钟内 */
                {
                    auto second = elapsed / 1000;
                    auto millsecond = (elapsed - second * 1000) % 1000;
                    timeStr = std::to_string(second);
                    if (millsecond > 0)
                    {
                        timeStr += "." + std::to_string(millsecond);
                    }
                    timeStr += " s";
                }
                else if (elapsed < 3600000) /* 1小时内 */
                {
                    auto minute = elapsed / 60000;
                    auto second = (elapsed % 60000) / 1000;
                    auto millsecond = (elapsed - second * 1000) % 1000;
                    timeStr = std::to_string(minute);
                    if (second > 0 || millsecond > 0)
                    {
                        timeStr += ":";
                        char buf[16] = {0};
                        sprintf(buf, "%02zu", second);
                        timeStr += buf;
                        if (millsecond > 0)
                        {
                            timeStr += "." + std::to_string(millsecond);
                        }
                    }
                    timeStr += " m";
                }
                else if (elapsed < 86400000) /* 1天内 */
                {
                    auto hour = elapsed / 3600000;
                    auto minute = (elapsed % 3600000) / 60000;
                    auto second = (elapsed % 60000) / 1000;
                    auto millsecond = (elapsed - second * 1000) % 1000;
                    timeStr = std::to_string(hour);
                    if (minute > 0 || second > 0 || millsecond > 0)
                    {
                        timeStr += ":";
                        char buf1[16] = {0};
                        sprintf(buf1, "%02zu", minute);
                        timeStr += buf1;
                        timeStr += ":";
                        if (second > 0 || millsecond > 0)
                        {
                            char buf2[16] = {0};
                            sprintf(buf2, "%02zu", second);
                            timeStr += buf2;
                            if (millsecond > 0)
                            {
                                timeStr += "." + std::to_string(millsecond);
                            }
                        }
                    }
                    timeStr += " h";
                }
                else
                {
                    timeStr = std::to_string(elapsed / 60000) + " m";
                }
                printf("[%s] 耗时 (%zu ms): %s\n", dtString().c_str(), elapsed, timeStr.c_str());
                printf("\n");
            }
        }
        else
        {
            value = toolkit::Tool::md5Directory(
                target, nullptr,
                [&, calcExecutor](const std::string& name, size_t fileSize, const std::function<std::string()>& calcFunc) {
                    if (calcExecutor)
                    {
                        calcExecutor->post("calc", calcFunc);
                    }
                    else
                    {
                        calcFunc();
                    }
                },
                nullptr, blockSize);
        }
    }
    else if (attr.isFile) /* 文件 */
    {
        auto buf = algorithm::md5SignFile(target.c_str(), blockSize);
        if (buf)
        {
            value = buf;
            free(buf);
        }
    }
    if (!value.empty())
    {
        printf("%s", value.c_str());
    }
    return 0;
}
