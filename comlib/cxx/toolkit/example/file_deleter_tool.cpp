#include <chrono>
#include <thread>

#include "../toolkit/file_deleter.h"
#include "fileparse/nlohmann/helper.hpp"
#include "utility/cmdline/cmdline.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

static int s_expireDetectPeriod = 0; /* 过期删除检测周期 */
static std::vector<toolkit::FileDeleter::ExpireConfig> s_expireConfigList; /* 过期删除配置列表 */

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
            if (firstFlag || elapsed >= (s_expireDetectPeriod * 60))
            {
                lastWatchTimepoint = ntp;
                firstFlag = false;
                toolkit::FileDeleter::deleteExpired(
                    s_expireConfigList,
                    [&](const std::string& name, const utility::FileAttribute& attr, int depth, bool ok) {
                        printf("[%s] delete folder: %s, %s\n", dtString().c_str(), name.c_str(), ok ? "success" : "fail");
                    },
                    [&](const std::string& name, const utility::FileAttribute& attr, int depth, bool ok) {
                        printf("[%s] delete file: %s, %s\n", dtString().c_str(), name.c_str(), ok ? "success" : "fail");
                    });
            }
        }
        catch (const std::exception& e)
        {
            printf("exception: {}", e.what());
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
    parser.add<int>("expire", 'e', "expired time (seconds), default 30 days", false, 2592000);
    parser.add<std::string>("file", 'f', "config file, file content e.g. [{\"folder\":\"/home/test/log\",\"expire\":2592000}]", false, "");
    parser.add<int>("period", 'p', "detect period (seconds), default 30 second", false, 30);
    printf("%s\n", parser.usage().c_str());
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto folder = parser.get<std::string>("folder");
    auto expireTime = parser.get<int>("expire");
    auto fullFile = parser.get<std::string>("file");
    auto period = parser.get<int>("period");
    if (period <= 0)
    {
        printf("period '%d' must > 0\n", period);
        return 0;
    }
    std::vector<toolkit::FileDeleter::ExpireConfig> expireConfigList;
    if (fullFile.empty())
    {
        if (folder.empty() || !utility::PathInfo(folder).exist())
        {
            printf("folder '%s' is not exist\n", folder.c_str());
            return 0;
        }
        if (expireTime <= 0)
        {
            printf("expireTime '%d' must > 0\n", expireTime);
            return 0;
        }
        expireConfigList.emplace_back(toolkit::FileDeleter::ExpireConfig(folder, expireTime));
    }
    else
    {
        auto arr = nlohmann::parse(utility::FileInfo(fullFile).readAll(true));
        if (arr.is_array())
        {
            for (size_t i = 0; i < arr.size(); ++i)
            {
                auto folder = nlohmann::getter<std::string>(arr[i], "folder");
                auto expireTime = nlohmann::getter<int>(arr[i], "expire");
                if (folder.empty() || !utility::PathInfo(folder).exist())
                {
                    printf("folder '%s' is not exist\n", folder.c_str());
                    continue;
                }
                if (expireTime <= 0)
                {
                    printf("expireTime '%d' must > 0\n", expireTime);
                    continue;
                }
                expireConfigList.emplace_back(toolkit::FileDeleter::ExpireConfig(folder, expireTime));
            }
        }
        if (expireConfigList.empty())
        {
            printf("file '%s' invalid\n", fullFile.c_str());
            return 0;
        }
        if (!folder.empty() && utility::PathInfo(folder).exist() && expireTime > 0)
        {
            expireConfigList.emplace_back(toolkit::FileDeleter::ExpireConfig(folder, expireTime));
        }
    }
    printf("========================= Start File Deleter =========================\n");
    printf("[%s] Detect Period: %d(s)\n", dtString().c_str(), period);
    for (size_t i = 0; i < expireConfigList.size(); ++i)
    {
        printf("[%s] ---------- [%d] ----------\n", dtString().c_str(), (int)i + 1);
        printf("[%s]        Folder: %s\n", dtString().c_str(), expireConfigList[i].folder.c_str());
        printf("[%s]  Expired Time: %d(s)\n", dtString().c_str(), (int)expireConfigList[i].expireTime);
    }
    printf("======================================================================\n");
    s_expireDetectPeriod = period;
    s_expireConfigList = expireConfigList;
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
