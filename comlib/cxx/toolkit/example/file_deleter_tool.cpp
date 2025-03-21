#include <chrono>
#include <thread>

#include "../toolkit/file_deleter.h"
#include "fileparse/nlohmann/helper.hpp"
#include "utility/cmdline/cmdline.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

static size_t s_detectPeriod = 0; /* 检测周期 */
static std::vector<toolkit::FileDeleter::Config> s_configList; /* 配置列表 */

static std::string dtString()
{
    return utility::DateTime::getNow().yyyyMMddhhmmss("-", " ", ":", ".");
}

static void fileDeleterWatch()
{
    auto lastWatchTimepoint = std::chrono::steady_clock::now();
    bool firstFlag = true;
    while (1)
    {
        try
        {
            auto ntp = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(ntp - lastWatchTimepoint).count();
            if (firstFlag || elapsed >= (s_detectPeriod * 60))
            {
                lastWatchTimepoint = ntp;
                firstFlag = false;
                auto ndt = utility::DateTime::getNow().yyyyMMdd();
                auto isToday = [&](const utility::FileAttribute& attr) { return (ndt == attr.modifyTimeFmt("%Y-%m-%d")); };
                toolkit::FileDeleter::tryOnce(
                    s_configList,
                    [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
                        auto dirName = utility::FileInfo(name).filename();
                        if ("$RECYCLE.BIN" == dirName || "System Volume Information" == dirName) /* 跳过Windows文件系统目录 */
                        {
                            return false;
                        }
                        return !isToday(attr);
                    },
                    [&](const std::string& name, const utility::FileAttribute& attr, int depth, bool ok) {
                        printf("[%s] delete folder: %s, %s\n", dtString().c_str(), name.c_str(), ok ? "success" : "fail");
                    },
                    [&](const std::string& name, const utility::FileAttribute& attr, int depth) { return !isToday(attr); },
                    [&](const std::string& name, const utility::FileAttribute& attr, int depth, bool ok) {
                        printf("[%s] delete file: %s, %s\n", dtString().c_str(), name.c_str(), ok ? "success" : "fail");
                    });
            }
        }
        catch (const std::exception& e)
        {
            printf("exception: %s", e.what());
        }
        catch (...)
        {
            printf("exception: 未知");
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char** argv)
{
    cmdline::parser parser;
    parser.add<std::string>("folder", 'd', "folder which to detect, e.g. \"/home/test/log\"", false, "");
    parser.add<int>("expire_time", 't', "expired time (seconds), default 30 days", false, 2592000);
    parser.add<int>("occupy_size", 's', "occupy size (bytes), default 0", false, 0);
    parser.add<float>("occupy_line", 'l', "occupy line [0.0, 1.0], default 80%", false, 0.8f);
    parser.add<std::string>("file", 'f', "config file, file content e.g. [{\"folder\":\"/home/test/log\",\"expire\":2592000}]", false, "");
    parser.add<int>("period", 'p', "detect period (seconds), default 30 second", false, 30);
    printf("%s\n", parser.usage().c_str());
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto folder = parser.get<std::string>("folder");
    auto expireTime = parser.get<int>("expire_time");
    auto occupySize = parser.get<int>("occupy_size");
    auto occupyLine = parser.get<float>("occupy_line");
    auto fullFile = parser.get<std::string>("file");
    auto period = parser.get<int>("period");
    if (period <= 0)
    {
        printf("period '%d' must > 0\n", period);
        return 0;
    }
    std::vector<toolkit::FileDeleter::Config> cfgList;
    if (fullFile.empty())
    {
        if (folder.empty() || !utility::PathInfo(folder).exist())
        {
            printf("folder '%s' is not exist\n", folder.c_str());
            return 0;
        }
        if (expireTime <= 0 && occupySize <= 0)
        {
            printf("expireTime '%d' or occupySize '%d' must > 0\n", expireTime, occupySize);
            return 0;
        }
        cfgList.emplace_back(toolkit::FileDeleter::Config(folder, expireTime, occupySize, occupyLine));
    }
    else
    {
        auto arr = nlohmann::parse(utility::FileInfo(fullFile).readAll());
        if (arr.is_array())
        {
            for (size_t i = 0; i < arr.size(); ++i)
            {
                auto folder = nlohmann::getter<std::string>(arr[i], "folder");
                auto expireTime = nlohmann::getter<int>(arr[i], "expire_time");
                auto occupySize = nlohmann::getter<int>(arr[i], "occupy_size");
                auto occupyLine = nlohmann::getter<float>(arr[i], "occupy_line");
                if (folder.empty() || !utility::PathInfo(folder).exist())
                {
                    printf("folder '%s' is not exist\n", folder.c_str());
                    continue;
                }
                if (expireTime <= 0 && occupySize <= 0)
                {
                    printf("expireTime '%d' or occupySize '%d' must > 0\n", expireTime, occupySize);
                    continue;
                }
                cfgList.emplace_back(toolkit::FileDeleter::Config(folder, expireTime, occupySize, occupyLine));
            }
        }
        if (cfgList.empty())
        {
            printf("file '%s' invalid\n", fullFile.c_str());
            return 0;
        }
        if (!folder.empty() && utility::PathInfo(folder).exist() && expireTime > 0)
        {
            cfgList.emplace_back(toolkit::FileDeleter::Config(folder, expireTime));
        }
    }
    printf("========================= Start File Deleter =========================\n");
    printf("[%s] Detect Period: %d(s)\n", dtString().c_str(), period);
    for (size_t i = 0; i < cfgList.size(); ++i)
    {
        printf("[%s] ---------- [%d] ----------\n", dtString().c_str(), (int)i + 1);
        printf("[%s]        Folder: %s\n", dtString().c_str(), cfgList[i].folder.c_str());
        printf("[%s]   Expire Time: %zu(s)\n", dtString().c_str(), cfgList[i].expireTime);
        printf("[%s]   Occupy Size: %zu(byte)\n", dtString().c_str(), cfgList[i].occupySize);
        printf("[%s]   Occupy Line: %.2f\n", dtString().c_str(), cfgList[i].occupyLine * 100);
    }
    printf("======================================================================\n");
    s_detectPeriod = period;
    s_configList = cfgList;
    /* 启动删除器 */
    std::thread th(fileDeleterWatch);
    th.detach();
    /* 主循环 */
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
    return 0;
}
