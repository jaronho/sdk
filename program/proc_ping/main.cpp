#include <chrono>
#include <thread>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "logger/logger_manager.h"
#include "toolkit/net_config.h"
#include "utility/cmdline/cmdline.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/process/process.h"

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    /* 关闭控制台程序的快速编辑模式, 否则会出现点击界面, 程序将会变成阻塞状态, 不按回车无法继续运行 */
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE; /* 移除快速编辑模式 */
    SetConsoleMode(hStdin, mode);
#endif
    utility::FileInfo fi(utility::Process::getProcessExeFile());
    /* 日志记录器 */
    logger::LogConfig cfg;
    cfg.path = fi.path();
    cfg.name = fi.basename() + "_"; /* 设置默认日志文件名前缀 */
    cfg.fileExtName = ".log";
    cfg.fileMaxSize = 15 * 1024 * 1024;
    cfg.fileMaxCount = 0;
    cfg.newFolderDaily = true;
    cfg.consoleMode = 1;
    logger::LoggerManager::setConfig(cfg);
    auto g_logger = logger::LoggerManager::getLogger();
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("ping命令扩展工具");
    parser.add<std::string>("dst", 'd', "目标地址, 可以为IP或域名.", true);
    parser.add<std::string>(
        "src", 's',
        "本机要使用的网络接口(当本机有多个网卡时, 可以指定使用哪一个), 为空表示默认, 例如: 网卡名(enp1s0) 或 IP(192.168.3.123).", false);
    parser.add<int>("count", 'c', "次数, 小等于0表示不限次数, 默认次数:", false, 1);
    parser.add<int>("timeout", 't', "超时时间(单位: 秒), 小等于0表示使用默认超时:", false, 0);
    parser.add<int>("record", 'r', "是否把相关信息记录到本地日志, 0-不记录, 1-记录, 默认:", false, 0);
    parser.parse_check(argc, argv, "用法", "选项", "显示帮助信息并退出");
    /* 参数解析 */
    auto dst = parser.get<std::string>("dst");
    auto src = parser.get<std::string>("src");
    auto count = parser.get<int>("count");
    auto timeout = parser.get<int>("timeout");
    auto record = parser.get<int>("record");
    count = count > 0 ? count : 0;
    if (record > 0)
    {
        INFO_LOG_PURE(g_logger, "{}", parser.usage());
    }
    else
    {
        printf("%s\n", parser.usage().c_str());
    }
    int cnt = 1;
    std::string result;
    while (count <= 0 || cnt <= count)
    {
        auto ret = toolkit::NetConfig::checkPing(src, dst, timeout, &result);
        if (record > 0)
        {
            INFO_LOG_PURE(g_logger, "第 [{}] 次检测: {}{}", cnt, ret ? "成功" : "失败", result);
        }
        else
        {
            printf("[%s] 第 [%d] 次检测: %s%s\n", utility::DateTime::getNow().yyyyMMddhhmmss("-", " ", ":", ".").c_str(), cnt,
                   ret ? "成功" : "失败", result.c_str());
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++cnt;
    }
    return 0;
}
