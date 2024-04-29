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
    parser.add<int>("type", 't', "算法类型: 0-计算文件内容(默认值), 1-计算(ASCII或GBK编码)目录/文件名和文件内容", false, 0,
                    cmdline::range(0, 1));
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
    auto type = parser.get<int>("type");
    auto blockSize = parser.get<int>("block");
    utility::FileAttribute attr;
    utility::getFileAttribute(target, attr);
    std::string value;
    if (attr.isDir) /* 目录 */
    {
        if (parser.exist("verbose"))
        {
            size_t totalFolderCount = 0, totalFileCount = 0, nowCount = 0;
            printf("[%s] 开始计算目录和文件数量\n", dtString().c_str());
            utility::PathInfo pi(target, true);
            pi.traverse(
                [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
                    ++totalFolderCount;
                    return true;
                },
                [&](const std::string& name, const utility::FileAttribute& attr, int depth) { ++totalFileCount; }, nullptr, true, false);
            printf("[%s] 目录数: %zu, 文件数: %zu\n", dtString().c_str(), totalFolderCount, totalFileCount);
            auto tp = std::chrono::steady_clock::now();
            value = toolkit::Tool::md5Directory(
                target, type,
                [&](const std::string& name, bool isDir, size_t fileSize) {
                    ++nowCount;
                    auto totalCountStr = std::to_string(totalFolderCount + totalFileCount);
                    auto nowCountStr = std::to_string(nowCount);
                    auto progress = "[" + utility::StrTool::fillPlace(nowCountStr, ' ', totalCountStr.size()) + "/" + totalCountStr + "]";
                    auto fileDesc = utility::StrTool::replace(name.substr(pi.path().size()), "\\", "/");
                    if (!fileDesc.empty() && '/' == fileDesc[0])
                    {
                        fileDesc.erase(0);
                    }
                    if (!isDir)
                    {
                        fileDesc += " (" + convertBytesToAppropriateUnit(fileSize) += ")";
                    }
                    printf("[%s] %s [%c] %s\n", dtString().c_str(), progress.c_str(), (isDir ? 'D' : 'F'), fileDesc.c_str());
                },
                blockSize);
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
            value = toolkit::Tool::md5Directory(target, type, nullptr, blockSize);
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
