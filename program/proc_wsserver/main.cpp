#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "nsocket/websocket/server.h"
#include "utility/cmdline/cmdline.h"

std::shared_ptr<nsocket::ws::Server> g_server = nullptr; /* 服务器 */

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
    parser.header("WebSocket服务端");
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
    parser.add<int>("reply", 'r', "应答方式, 值: 0-不应答, 1-原数据返回, 默认:", false, 0, cmdline::oneof<int>(0, 1));
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
    auto reply = parser.get<int>("reply");
    std::string replyDesc;
    if (0 == reply)
    {
        replyDesc = "不应答";
    }
    else if (1 == reply)
    {
        replyDesc = "原数据返回";
    }
    g_server = std::make_shared<nsocket::ws::Server>("ws_server", 10, server, port);
    /* 设置新连接回调 */
    g_server->setConnectingCallback([&](const std::weak_ptr<nsocket::ws::Session>& wpSession) {
        const auto session = wpSession.lock();
        if (session)
        {
            printf("============================== 新连接 [%lld][%s:%d], URI: %s\n", session->getId(), session->getClientHost().c_str(),
                   session->getClientPort(), session->getUri().c_str());
        }
        return nullptr;
    });
    /* 设置打开回调 */
    g_server->setOpenCallback([&](const std::weak_ptr<nsocket::ws::Session>& wpSession) {
        const auto session = wpSession.lock();
        if (session)
        {
            printf("==================== 打开客户端 [%lld][%s:%d], URI: %s\n", session->getId(), session->getClientHost().c_str(),
                   session->getClientPort(), session->getUri().c_str());
        }
    });
    /* 设置收到PING包回调 */
    g_server->setPingCallback([&](const std::weak_ptr<nsocket::ws::Session>& wpSession) {
        const auto session = wpSession.lock();
        if (session)
        {
            printf("++++++++++ 收到 [%lld][%s:%d] PING 包, 应答 PONG 包\n", session->getId(), session->getClientHost().c_str(),
                   session->getClientPort());
            session->sendPong();
        }
    });
    /* 设置收到PONG包回调 */
    g_server->setPongCallback([&](const std::weak_ptr<nsocket::ws::Session>& wpSession) {
        const auto session = wpSession.lock();
        if (session)
        {
            printf("++++++++++ 收到 [%lld][%s:%d] PONG 包\n", session->getId(), session->getClientHost().c_str(), session->getClientPort());
        }
    });
    /* 设置消息接收者 */
    auto msger = std::make_shared<nsocket::ws::SrvMessager_simple>();
    msger->onMessage = [&, reply](const std::weak_ptr<nsocket::ws::Session>& wpSession, bool isText, const std::string& msg) {
        const auto session = wpSession.lock();
        if (session)
        {
            if (isText)
            {
                printf("++++++++++ 收到 [%lld][%s:%d] 消息(文本), 长度: %zu\n", session->getId(), session->getClientHost().c_str(),
                       session->getClientPort(), msg.size());
                printf("%s", msg.c_str());
            }
            else
            {
                printf("++++++++++ 收到 [%lld][%s:%d] 消息(二进制), 长度: %zu\n", session->getId(), session->getClientHost().c_str(),
                       session->getClientPort(), msg.size());
                /* 以十六进制格式打印数据 */
                for (size_t i = 0; i < msg.size(); ++i)
                {
                    printf("%02X ", msg[i]);
                }
            }
            printf("\n");
            if (1 == reply) /* 把收到的数据原封不动返回给客户端 */
            {
                if (isText)
                {
                    session->sendText(msg);
                }
                else
                {
                    std::vector<unsigned char> data;
                    data.insert(data.end(), msg.begin(), msg.end());
                    session->sendBytes(data);
                }
                printf("---------- 回复 [%lld][%s:%d], 长度: %zu\n", session->getId(), session->getClientHost().c_str(),
                       session->getClientPort(), msg.size());
            }
        }
    };
    g_server->setMessager(msger);
    /* 设置连接关闭回调 */
    g_server->setCloseCallback([&](uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        if (code)
        {
            printf("-------------------- 关闭客户端 [%lld][%s:%d], %d, %s\n", cid, clientHost.c_str(), clientPort, code.value(),
                   code.message().c_str());
        }
        else
        {
            printf("-------------------- 关闭客户端 [%lld][%s:%d]\n", cid, clientHost.c_str(), clientPort);
        }
    });
    /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
    try
    {
        if (1 == sslOn && !certFile.empty() && !pkFile.empty())
        {
            printf("启动服务器: %s:%d, SSL验证: %s, 应答: %s\n", server.c_str(), port, 1 == sslWay ? "单向" : "双向", replyDesc.c_str());
        }
        else
        {
            printf("启动服务器: %s:%d, 应答: %s\n", server.c_str(), port, replyDesc.c_str());
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
