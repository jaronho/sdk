#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atomic>
#include <chrono>
#include <thread>

#include "nsocket/tcp/tcp_client.h"
#include "utility/cmdline/cmdline.h"
#include "utility/digit/digit.h"
#include "utility/filesystem/file_info.h"
#include "utility/strtool/strtool.h"

std::shared_ptr<nsocket::TcpClient> g_client = nullptr; /* 客户端 */
std::atomic_bool g_connected = {false}; /* 是否已连接上 */
boost::asio::ip::tcp::endpoint g_localEndpoint; /* 本地端点 */

/**
 * @brief 发送数据
 */
bool sendData(const std::vector<unsigned char>& data)
{
    if (g_client)
    {
        /* 发送数据 */
        std::size_t length;
        auto code = g_client->send(data, length);
        if (code)
        {
            printf("-------------------- 发送失败: %d, %s\n", code.value(), code.message().c_str());
        }
        else
        {
            printf("++++++++++++++++++++ 发送成功, 数据长度: %zu\n", length);
            return true;
        }
    }
    else
    {
        printf("-------------------- 发送失败: 客户端为空\n");
    }
    return false;
}

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    /* 关闭控制台程序的快速编辑模式, 否则会出现点击界面, 程序将会变成阻塞状态, 不按回车无法继续运行 */
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE; /* 移除快速编辑模式 */
    SetConsoleMode(hStdin, mode);
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("TCP客户端");
    parser.add<int>("local-port", 'l', "本地端口(0表示自动分配), 默认:", false, 0, cmdline::range(1, 65535));
    parser.add<std::string>("server", 's', "服务器地址, 默认:", false, "127.0.0.1");
    parser.add<int>("port", 'p', "服务器端口, 默认:", false, 4444, cmdline::range(1, 65535));
#if (1 == ENABLE_NSOCKET_OPENSSL)
    parser.add<int>("ssl-on", 't', "是否启用TLS, 值: 0-不启用, 1-启用, 默认:", false, 0, cmdline::range(0, 1));
    parser.add<int>("ssl-way", 'w', "SSL验证, 值: 1-单向验证, 2-双向验证, 默认:", false, 1, cmdline::oneof<int>(1, 2));
    parser.add<int>("cert-fmt", 'f', "证书文件格式, 值: 1-DER, 2-PEM, 默认:", false, 2, cmdline::oneof<int>(1, 2));
    parser.add<std::string>("cert-file", 'c', "证书文件名, 例如: server.crt, 默认:", false, "");
    parser.add<std::string>("pk-file", 'k', "私钥文件名, 例如: server.key, 默认:", false, "");
    parser.add<std::string>("pk-pwd", 'P', "私钥文件密码, 例如: 123456, 默认:", false, "");
#endif
    parser.add<int>("data-type", 'd',
                    "数据类型, 值: 1-输入(原始), 2-输入(十六进制), 3-文件(原始, 全部), 4-文件(原始, 单行), 5-文件(十六进制, 单行), 默认:",
                    false, 1, cmdline::oneof<int>(1, 2, 3, 4, 5));
    parser.add<int>("interval", 'i', "按行发送文件数据时, 每行的发送间隔(毫秒), 默认:", false, 500);
    parser.parse_check(argc, argv, "用法", "选项", "显示帮助信息并退出");
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    auto localPort = parser.get<int>("local-port");
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
    auto dataType = parser.get<int>("data-type");
    auto interval = parser.get<int>("interval");
    interval = interval < 0 ? 0 : interval;
    std::string dataTypeDesc, lineIntervalDesc;
    if (1 == dataType)
    {
        dataTypeDesc = "发送输入(原始)";
    }
    else if (2 == dataType)
    {
        dataTypeDesc = "发送输入(十六进制)";
    }
    else if (3 == dataType)
    {
        dataTypeDesc = "发送文件(原始, 全部)";
    }
    else if (4 == dataType)
    {
        dataTypeDesc = "发送文件(原始, 单行)";
        lineIntervalDesc = ", 行发送间隔: " + std::to_string(interval) + "(毫秒)";
    }
    else if (5 == dataType)
    {
        dataTypeDesc = "发送文件(十六进制, 单行)";
        lineIntervalDesc = ", 行发送间隔: " + std::to_string(interval) + "(毫秒)";
    }
    g_client = std::make_shared<nsocket::TcpClient>(localPort);
    /* 设置连接回调 */
    g_client->setConnectCallback([&](const boost::system::error_code& code) {
        auto remoteEndpoint = g_client->getRemoteEndpoint();
        auto serverHost = remoteEndpoint.address().to_string();
        auto serverPort = remoteEndpoint.port();
        if (code)
        {
            auto clientHost = g_localEndpoint.address().to_string();
            auto clientPort = g_localEndpoint.port();
            if (g_connected)
            {
                printf("============================== [%s:%d] 连接 [%s:%d] 断开: %d, %s\n", clientHost.c_str(), clientPort,
                       serverHost.c_str(), serverPort, code.value(), code.message().c_str());
            }
            else
            {
                printf("============================== [%s:%d] 连接 [%s:%d] 失败: %d, %s\n", clientHost.c_str(), clientPort,
                       serverHost.c_str(), serverPort, code.value(), code.message().c_str());
            }
            exit(0);
        }
        else
        {
            g_connected = true;
            g_localEndpoint = g_client->getLocalEndpoint();
            auto clientHost = g_localEndpoint.address().to_string();
            auto clientPort = g_localEndpoint.port();
            printf("============================== [%s:%d] 连接 [%s:%d] 成功\n", clientHost.c_str(), clientPort, serverHost.c_str(),
                   serverPort);
        }
    });
    /* 设置数据回调 */
    g_client->setDataCallback([&](const std::vector<unsigned char>& data) {
        printf("++++++++++ 收到数据, 长度: %zu\n", data.size());
        /* 以十六进制格式打印数据 */
        printf("+++++ [十六进制]\n");
        for (size_t i = 0; i < data.size(); ++i)
        {
            printf("%02X ", data[i]);
        }
        printf("\n");
        /* 以字符串格式打印数据 */
        printf("+++++ [字节流]\n");
        std::string input(data.begin(), data.end());
        printf("%s", input.c_str());
        printf("\n");
    });
    /* 创建线程专门用于网络I/O事件轮询 */
    std::thread th([&, localPort, server, port, sslOn, sslWay, certFmt, certFile, pkFile, pkPwd]() {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
            if (1 == sslOn && 1 == sslWay)
            {
                printf("连接服务器: %s:%d, SSL验证: 单向, 数据类型: %s%s\n", server.c_str(), port, dataTypeDesc.c_str(),
                       lineIntervalDesc.c_str());
            }
            else if (1 == sslOn && 2 == sslWay && !certFile.empty() && !pkFile.empty())
            {
                printf("连接服务器: %s:%d, SSL验证: 双向, 数据类型: %s%s\n", server.c_str(), port, dataTypeDesc.c_str(),
                       lineIntervalDesc.c_str());
            }
            else
            {
                printf("连接服务器: %s:%d, 数据类型: %s%s\n", server.c_str(), port, dataTypeDesc.c_str(), lineIntervalDesc.c_str());
            }
            g_client->run(server, port, sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
        }
        catch (const std::exception& e)
        {
            printf("========== 异常: %s\n", e.what());
        }
        catch (...)
        {
            printf("========== 异常: 未知\n");
        }
        exit(0);
    });
    th.detach();
    /* 主线程 */
    static const int MAX_PAYLOAD = 65495;
    while (1)
    {
        /* 接收输入数据 */
        char input[4096] = {0};
        std::cin.getline(input, sizeof(input));
        if (0 == strlen(input)) /* 输入为空继续等待 */
        {
            continue;
        }
        printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
        /* 数据预处理 */
        if (1 == dataType)
        {
            std::vector<unsigned char> data;
            data.insert(data.end(), input, input + strlen(input));
            sendData(data);
        }
        else if (2 == dataType) /* 把十六进制转为字节流 */
        {
            std::string str = input;
            str = utility::StrTool::toLower(str);
            str = utility::StrTool::replace(str, "0x", "");
            str = utility::StrTool::replace(str, " ", "");
            if (!utility::Digit::isHex(str)) /* 非十六进制 */
            {
                printf("-------------------- 发送失败: 数据格式错误(非十六进制), %s\n", input);
                continue;
            }
            auto bytes = utility::StrTool::fromHex(str);
            if (bytes.empty())
            {
                printf("-------------------- 发送失败: 数据格式错误(非十六进制), %s\n", input);
                continue;
            }
            std::vector<unsigned char> data;
            data.insert(data.end(), bytes.begin(), bytes.end());
            sendData(data);
        }
        else if (3 == dataType || 4 == dataType || 5 == dataType) /* 读取文件内容 */
        {
            auto f = fopen(input, "rb"); /* 打开文件 */
            if (!f)
            {
                printf("-------------------- 发送失败: 文件 %s 不存在\n", input);
                continue;
            }
            if (3 == dataType) /* 发送文件(原始), 全部 */
            {
                auto fileSize = utility::FileInfo(input).size();
                size_t offset = 0;
                while (offset < fileSize)
                {
                    size_t count = MAX_PAYLOAD;
                    auto buf = utility::FileInfo::read(f, offset, count);
                    offset += count;
                    if (buf)
                    {
                        std::vector<unsigned char> data;
                        data.insert(data.end(), buf, buf + count);
                        sendData(data);
                        free(buf);
                    }
                }
            }
            else if (4 == dataType) /* 发送文件(原始), 单行 */
            {
                while (!feof(f))
                {
                    std::string line, bomFlag, endFlag;
                    if (!utility ::FileInfo::readLine(f, line, bomFlag, endFlag))
                    {
                        break;
                    }
                    if (line.empty())
                    {
                        continue;
                    }
                    size_t offset = 0;
                    bool ret = false;
                    while (offset < line.size())
                    {
                        size_t count = MAX_PAYLOAD;
                        if (count > line.size() - offset)
                        {
                            count = line.size() - offset;
                        }
                        auto buf = line.substr(offset, count);
                        offset += count;
                        std::vector<unsigned char> data;
                        data.insert(data.end(), buf.begin(), buf.end());
                        ret = sendData(data);
                    }
                    if (interval > 0 && ret)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                    }
                }
            }
            else if (5 == dataType) /* 发送文件(十六进制), 单行 */
            {
                while (!feof(f))
                {
                    std::string line, bomFlag, endFlag;
                    if (!utility ::FileInfo::readLine(f, line, bomFlag, endFlag))
                    {
                        break;
                    }
                    if (line.empty())
                    {
                        continue;
                    }
                    auto buf = utility::StrTool::toLower(line);
                    buf = utility::StrTool::replace(buf, "0x", "");
                    buf = utility::StrTool::replace(buf, " ", "");
                    if (!utility::Digit::isHex(buf)) /* 非十六进制 */
                    {
                        printf("-------------------- 发送失败: 数据格式错误(非十六进制), %s\n", line.c_str());
                        break;
                    }
                    auto bytes = utility::StrTool::fromHex(buf);
                    if (bytes.empty())
                    {
                        printf("-------------------- 发送失败: 数据格式错误(非十六进制), %s\n", line.c_str());
                        continue;
                    }
                    size_t offset = 0;
                    bool ret = false;
                    while (offset < buf.size())
                    {
                        size_t count = MAX_PAYLOAD - 1;
                        if (count > buf.size() - offset)
                        {
                            count = buf.size() - offset;
                        }
                        auto tmp = buf.substr(offset, count);
                        offset += count;
                        auto bytes = utility::StrTool::fromHex(tmp);
                        std::vector<unsigned char> data;
                        data.insert(data.end(), bytes.begin(), bytes.end());
                        ret = sendData(data);
                    }
                    if (interval > 0 && ret)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                    }
                }
            }
            fclose(f); /* 关闭文件句柄 */
        }
    }
    return 0;
}
