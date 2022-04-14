#include <chrono>
#include <thread>

#include "../toolkit/file_deleter.h"
#include "threading/async_proxy.h"
#include "utility/cmdline/cmdline.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

int main(int argc, char** argv)
{
    cmdline::parser parser;
    parser.add<std::string>("folder", 'd', "folder which to detect, e.g. \"/home/test/log\"", true, "");
    parser.add<int>("both", 'a', "whether detect both folder and file, if 0 only detect file", false, 1);
    parser.add<int>("expire", 't', "expired time (seconds), default 30 days", false, 2592000);
    parser.add<int>("interval", 'i', "detect interval (seconds), default 30 second", false, 30);
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto folder = parser.get<std::string>("folder");
    auto both = parser.get<int>("both");
    auto expireTime = parser.get<int>("expire");
    auto interval = parser.get<int>("interval");
    if (!utility::PathInfo(folder).exist())
    {
        printf("folder '%s' is not exist\n", folder.c_str());
        return 0;
    }
    if (expireTime <= 0)
    {
        printf("expireTime '%d' must > 0\n", expireTime);
        return 0;
    }
    if (interval <= 0)
    {
        printf("interval '%d' must > 0\n", interval);
        return 0;
    }
    printf("========================= Start File Deleter =========================\n");
    printf("[%s]          Folder: %s\n", utility::DateTime::getNow().yyyyMMddhhmmss().c_str(), folder.c_str());
    printf("[%s]   Detect Target: %s(s)\n", utility::DateTime::getNow().yyyyMMddhhmmss().c_str(), both ? "folder and file" : "only file");
    printf("[%s]    Expired Time: %d(s)\n", utility::DateTime::getNow().yyyyMMddhhmmss().c_str(), expireTime);
    printf("[%s] Detect Interval: %d(s)\n", utility::DateTime::getNow().yyyyMMddhhmmss().c_str(), interval);
    printf("\n");
    /* 启动异步任务模块 */
    threading::AsyncProxy::start(1);
    /* 启动删除器 */
    toolkit::FileDeleteConfig cfg;
    cfg.folder = folder;
    cfg.both = both > 0;
    cfg.expireSecond = expireTime;
    toolkit::FileDeleter deleter;
    deleter.setFolderDeletedCallback([&](const std::string& fullName, bool ok) {
        auto nowDate = utility::DateTime::getNow();
        printf("[%s] delete folder: %s, %s\n", nowDate.yyyyMMddhhmmss().c_str(), fullName.c_str(), ok ? "success" : "fail");
    });
    deleter.setFileDeletedCallback([&](const std::string& fullName, bool ok) {
        auto nowDate = utility::DateTime::getNow();
        printf("[%s] delete file: %s, %s\n", nowDate.yyyyMMddhhmmss().c_str(), fullName.c_str(), ok ? "success" : "fail");
    });
    deleter.start(interval, {cfg});
    /* 主循环 */
    while (1)
    {
        threading::AsyncProxy::runOnce();
        threading::Timer::runOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
