#include <chrono>
#include <thread>

#include "../toolkit/file_deleter.h"
#include "fileparse/nlohmann/helper.hpp"
#include "threading/async_proxy.h"
#include "utility/cmdline/cmdline.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

static std::string dtString()
{
    return utility::DateTime::getNow().yyyyMMddhhmmss("-", " ", ":", ".");
}

int main(int argc, char** argv)
{
    cmdline::parser parser;
    parser.add<std::string>("folder", 'd', "folder which to detect, e.g. \"/home/test/log\"", false, "");
    parser.add<int>("both", 'a', "whether detect both folder and file, if 0 only detect file", false, 1);
    parser.add<int>("expire", 't', "expired time (seconds), default 30 days", false, 2592000);
    parser.add<std::string>("file", 'f',
                            "config file, file content e.g. [{\"folder\":\"/home/test/log\",\"both\":true,\"expire\":2592000}]", false, "");
    parser.add<int>("interval", 'i', "detect interval (seconds), default 30 second", false, 30);
    printf("%s\n", parser.usage().c_str());
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto folder = parser.get<std::string>("folder");
    auto both = parser.get<int>("both");
    auto expireTime = parser.get<int>("expire");
    auto fullFile = parser.get<std::string>("file");
    auto interval = parser.get<int>("interval");
    if (interval <= 0)
    {
        printf("interval '%d' must > 0\n", interval);
        return 0;
    }
    std::vector<toolkit::FileDeleteConfig> cfgList;
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
        cfgList.emplace_back(toolkit::FileDeleteConfig(folder, both > 0 ? true : false, expireTime));
    }
    else
    {
        auto arr = nlohmann::parse(utility::FileInfo(fullFile).readAll(true));
        if (arr.is_array())
        {
            for (size_t i = 0; i < arr.size(); ++i)
            {
                auto folder = nlohmann::getter<std::string>(arr[i], "folder");
                auto both = nlohmann::getter<int>(arr[i], "both");
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
                cfgList.emplace_back(toolkit::FileDeleteConfig(folder, both > 0 ? true : false, expireTime));
            }
        }
        if (cfgList.empty())
        {
            printf("file '%s' invalid\n", fullFile.c_str());
            return 0;
        }
        if (!folder.empty() && utility::PathInfo(folder).exist() && expireTime > 0)
        {
            cfgList.emplace_back(toolkit::FileDeleteConfig(folder, both > 0 ? true : false, expireTime));
        }
    }

    printf("========================= Start File Deleter =========================\n");
    printf("[%s] Detect Interval: %d(s)\n", dtString().c_str(), interval);
    for (size_t i = 0; i < cfgList.size(); ++i)
    {
        printf("[%s] ---------- [%d] ----------\n", dtString().c_str(), (int)i + 1);
        printf("[%s]        Folder: %s\n", dtString().c_str(), cfgList[i].folder.c_str());
        printf("[%s] Detect Target: %s\n", dtString().c_str(), cfgList[i].both ? "both folder and file" : "only file");
        printf("[%s]  Expired Time: %d(s)\n", dtString().c_str(), (int)cfgList[i].expireSecond);
    }
    printf("======================================================================\n");
    /* 启动异步任务模块 */
    threading::AsyncProxy::start(1);
    /* 启动删除器 */
    toolkit::FileDeleter deleter;
    deleter.setFolderDeletedCallback([&](const std::string& fullName, bool ok) {
        printf("[%s] delete folder: %s, %s\n", dtString().c_str(), fullName.c_str(), ok ? "success" : "fail");
    });
    deleter.setFileDeletedCallback([&](const std::string& fullName, bool ok) {
        printf("[%s] delete file: %s, %s\n", dtString().c_str(), fullName.c_str(), ok ? "success" : "fail");
    });
    deleter.start(interval, cfgList);
    /* 主循环 */
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
