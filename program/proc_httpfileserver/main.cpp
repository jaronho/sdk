#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>

#include "hfs/http_file_server.h"
#include "utility/cmdline/cmdline.h"
#include "utility/filesystem/path_info.h"

utility::PathInfo g_rootDir; /* 根目录 */
std::shared_ptr<hfs::HttpFileServer> g_server = nullptr; /* 服务器 */

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
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("HTTP文件服务器");
    parser.add<std::string>("server", 's', "服务器地址, 默认:", false, "0.0.0.0");
    parser.add<int>("port", 'p', "服务器端口, 默认:", false, 4444, cmdline::range(1, 65535));
#if (1 == ENABLE_NSOCKET_OPENSSL)
    parser.add<int>("ssl-on", 't', "是否启用TLS, 值: 0-不启用, 1-启用, 默认:", false, 0, cmdline::range(0, 1));
    parser.add<int>("ssl-way", 'w', "SSL验证, 值: 1-单向验证, 2-双向验证, 默认:", false, 1, cmdline::oneof<int>(1, 2));
    parser.add<int>("cert-fmt", 'f', "证书文件格式, 值: 1-DER, 2-PEM, 默认:", false, 2, cmdline::oneof<int>(1, 2));
    parser.add<std::string>("cert-file", 'c', "证书文件名, 例如: server.crt, 默认:", false, "");
    parser.add<std::string>("pk-file", 'k', "私钥文件名, 例如: server.key, 默认:", false, "");
    parser.add<std::string>("pk-pwd", 'P', "私钥文件密码, 例如: 123456, 默认:", false, "");
#endif
    parser.add<std::string>("dir", 'd', "资源文件路径, 例如: /home/data/files, 默认:", false, "");
    parser.add<int>("thread-num", 'n', "并发线程数量, 默认:", false, 10, cmdline::range(1, 1024));
    parser.parse_check(argc, argv, "用法", "选项", "显示帮助信息并退出");
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    auto server = parser.get<std::string>("server");
    auto port = parser.get<int>("port");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    auto sslOn = parser.get<int>("ssl-on");
    auto sslWay = parser.get<int>("ssl-way");
    auto certFmt = parser.get<int>("cert-fmt");
    auto certFile = parser.get<std::string>("cert-file");
    auto pkFile = parser.get<std::string>("pk-file");
    auto pkPwd = parser.get<std::string>("pk-pwd");
#endif
    auto fileDir = parser.get<std::string>("dir");
    auto threadNum = parser.get<int>("thread-num");
    fileDir = fileDir.empty() ? utility::PathInfo::getcwd() : fileDir;
    g_rootDir = utility::PathInfo(fileDir);
    if (!g_rootDir.exist())
    {
        printf("资源文件路径: %s 不存在\n", g_rootDir.path().c_str());
        return 0;
    }
    else if (!g_rootDir.isAbsolute())
    {
        printf("资源文件路径: %s 不能为相对路径\n", g_rootDir.path().c_str());
        return 0;
    }
    printf("资源文件路径: %s\n", g_rootDir.path().c_str());
    g_server = std::make_shared<hfs::HttpFileServer>("hfs", threadNum, server, port, g_rootDir.path());
    /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
    try
    {
        if (1 == sslOn && !certFile.empty() && !pkFile.empty())
        {
            printf("启动服务器: %s:%d, SSL验证: %s\n", server.c_str(), port, 1 == sslWay ? "单向" : "双向");
        }
        else
        {
            printf("启动服务器: %s:%d\n", server.c_str(), port);
        }
        std::string errDesc;
        if (g_server->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd, &errDesc))
        {
            /* 主线程 */
            while (1)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        else
        {
            printf("服务器启动失败: %s\n", errDesc.c_str());
        }
    }
    catch (const std::exception& e)
    {
        printf("========== 异常: %s\n", e.what());
    }
    catch (...)
    {
        printf("========== 异常: 未知\n");
    }
    return 0;
}
