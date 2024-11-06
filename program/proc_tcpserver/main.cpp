#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <chrono>
#include <map>
#include <mutex>
#include <thread>

#include "nsocket/tcp/tcp_server.h"
#include "utility/cmdline/cmdline.h"
#include "utility/digit/digit.h"
#include "utility/filesystem/file_info.h"
#include "utility/strtool/strtool.h"

std::shared_ptr<nsocket::TcpServer> g_server = nullptr; /* 服务器 */
std::mutex g_mutexConnList;
std::map<uint64_t, std::weak_ptr<nsocket::TcpConnection>> g_connList; /* 连接列表 */

/**
 * @brief 发送数据
 */
bool sendData(std::vector<unsigned char>& data, int crlf)
{
    std::map<uint64_t, std::weak_ptr<nsocket::TcpConnection>> connList;
    {
        std::lock_guard<std::mutex> locker(g_mutexConnList);
        connList = g_connList;
    }
    if (connList.empty())
    {
        printf("-------------------- 发送失败: 当前没有客户端连接\n");
        return false;
    }
    if (1 == crlf)
    {
        data.push_back('\r');
    }
    else if (2 == crlf)
    {
        data.push_back('\n');
    }
    else if (3 == crlf)
    {
        data.push_back('\r');
        data.push_back('\n');
    }
    for (auto kv : connList)
    {
        auto conn = kv.second.lock();
        if (conn)
        {
            conn->send(data, [&, wpConn = kv.second](const boost::system::error_code& code, std::size_t length) {
                const auto conn = wpConn.lock();
                if (conn)
                {
                    auto point = conn->getRemoteEndpoint();
                    std::string clientHost = point.address().to_string().c_str();
                    int clientPort = (int)point.port();
                    if (code)
                    {
                        printf("---------- 发送 [%lld][%s:%d] 失败: %d, %s\n", conn->getId(), clientHost.c_str(), clientPort, code.value(),
                               code.message().c_str());
                    }
                    else
                    {
                        printf("---------- 发送 [%lld][%s:%d] 成功, 长度: %zu\n", conn->getId(), clientHost.c_str(), clientPort, length);
                    }
                }
            });
        }
    }
    return true;
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
    parser.header("TCP服务端");
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
    parser.add<int>("reply", 'r', "应答数据, 值: 0-无(不应答), 1-原数据, 默认:", false, 0, cmdline::oneof<int>(0, 1));
    parser.add<int>(
        "data-type", 'd',
        "通知数据类型, 值: 1-输入(原始), 2-输入(十六进制), 3-文件(原始, 全部), 4-文件(原始, 单行), 5-文件(十六进制, 单行), 默认:", false, 1,
        cmdline::oneof<int>(1, 2, 3, 4, 5));
    parser.add<int>("crlf", 'e', "通知数据结束符(选填), 值: [0: 无, 1: CR(回车), 2: LF(换行), 3: CRLF(回车换行)], 默认:", false, 0,
                    cmdline::oneof<int>(0, 1, 2, 3));
    parser.add<int>("interval", 'i', "按行发送文件数据时, 每行的发送间隔(毫秒), 默认:", false, 500);
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
        replyDesc = "无(不应答)";
    }
    else if (1 == reply)
    {
        replyDesc = "原数据";
    }
    auto dataType = parser.get<int>("data-type");
    auto crlf = parser.get<int>("crlf");
    auto interval = parser.get<int>("interval");
    interval = interval < 0 ? 0 : interval;
    std::string dataTypeDesc, lineIntervalDesc;
    if (1 == dataType)
    {
        dataTypeDesc = "原始";
    }
    else if (2 == dataType)
    {
        dataTypeDesc = "十六进制";
    }
    else if (3 == dataType)
    {
        dataTypeDesc = "文件(原始, 全部)";
    }
    else if (4 == dataType)
    {
        dataTypeDesc = "文件(原始, 单行)";
        lineIntervalDesc = ", 行发送间隔: " + std::to_string(interval) + "(毫秒)";
    }
    else if (5 == dataType)
    {
        dataTypeDesc = "文件(十六进制, 单行)";
        lineIntervalDesc = ", 行发送间隔: " + std::to_string(interval) + "(毫秒)";
    }
    std::string crlfDesc;
    if (0 == crlf)
    {
        crlfDesc = "无";
    }
    else if (1 == crlf)
    {
        crlfDesc = "CR(回车)";
    }
    else if (2 == crlf)
    {
        crlfDesc = "LF(换行)";
    }
    else if (3 == crlf)
    {
        crlfDesc = "CRLF(回车换行)";
    }
    g_server = std::make_shared<nsocket::TcpServer>("tcp_server", 10, server, port);
    std::string errDesc;
    if (!g_server->isValid(&errDesc))
    {
        printf("启动服务器失败, 请检查地址[%s]和端口[%d]是否可用, %s\n", server.c_str(), port, errDesc.c_str());
        return 0;
    }
    /* 设置新连接回调 */
    g_server->setNewConnectionCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn) {
        const auto conn = wpConn.lock();
        if (conn)
        {
            auto cid = conn->getId();
            auto point = conn->getRemoteEndpoint();
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("============================== 新连接 [%lld][%s:%d]\n", conn->getId(), clientHost.c_str(), clientPort);
            {
                std::lock_guard<std::mutex> locker(g_mutexConnList);
                if (g_connList.end() == g_connList.find(cid))
                {
                    g_connList.insert(std::make_pair(cid, wpConn));
                }
            }
        }
    });
    /* 设置握手成功回调 */
    g_server->setHandshakeOkCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn) {
        const auto conn = wpConn.lock();
        if (conn)
        {
            auto point = conn->getRemoteEndpoint();
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("==================== 校验成功 [%lld] [%s:%d]\n", conn->getId(), clientHost.c_str(), clientPort);
        }
    });
    /* 设置握手失败回调 */
    g_server->setHandshakeFailCallback(
        [&](uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("-------------------- 校验失败 [%lld] [%s:%d], %d, %s\n", cid, clientHost.c_str(), clientPort, code.value(),
                   code.message().c_str());
        });
    /* 设置连接数据回调 */
    g_server->setConnectionDataCallback(
        [&, reply](const std::weak_ptr<nsocket::TcpConnection>& wpConn, const std::vector<unsigned char>& data) {
            const auto conn = wpConn.lock();
            if (conn)
            {
                auto point = conn->getRemoteEndpoint();
                std::string clientHost = point.address().to_string().c_str();
                int clientPort = (int)point.port();
                printf("++++++++++ 收到 [%lld][%s:%d] 数据, 长度: %zu\n", conn->getId(), clientHost.c_str(), clientPort, data.size());
                /* 以十六进制格式打印数据 */
                printf("+++++ [十六进制]\n");
                for (size_t i = 0; i < data.size(); ++i)
                {
                    printf("%02X ", data[i]);
                }
                printf("\n");
                /* 以字符串格式打印数据 */
                printf("+++++ [字节流]\n");
                std::string str(data.begin(), data.end());
                printf("%s", str.c_str());
                printf("\n");
                if (1 == reply) /* 把收到的数据原封不动返回给客户端 */
                {
                    conn->send(data, [&, wpConn](const boost::system::error_code& code, std::size_t length) {
                        const auto conn = wpConn.lock();
                        if (conn)
                        {
                            auto point = conn->getRemoteEndpoint();
                            std::string clientHost = point.address().to_string().c_str();
                            int clientPort = (int)point.port();
                            if (code)
                            {
                                printf("---------- 回复 [%lld][%s:%d] 失败: %d, %s\n", conn->getId(), clientHost.c_str(), clientPort,
                                       code.value(), code.message().c_str());
                            }
                            else
                            {
                                printf("---------- 回复 [%lld][%s:%d] 成功, 长度: %zu\n", conn->getId(), clientHost.c_str(), clientPort,
                                       length);
                            }
                        }
                    });
                }
            }
        });
    /* 设置连接关闭回调 */
    g_server->setConnectionCloseCallback(
        [&](uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            if (code)
            {
                printf("-------------------- 关闭 [%lld][%s:%d] 连接: %d, %s\n", cid, clientHost.c_str(), clientPort, code.value(),
                       code.message().c_str());
            }
            else
            {
                printf("-------------------- 关闭 [%lld][%s:%d] 连接\n", cid, clientHost.c_str(), clientPort);
            }
            {
                std::lock_guard<std::mutex> locker(g_mutexConnList);
                auto iter = g_connList.find(cid);
                if (g_connList.end() == iter)
                {
                    g_connList.erase(iter);
                }
            }
        });
    /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
    try
    {
        if (1 == sslOn && !certFile.empty() && !pkFile.empty())
        {
            printf("启动服务器: %s:%d, SSL验证: %s, 应答数据: %s, 通知数据: 类型-%s%s, 结束符-%s\n", server.c_str(), port,
                   1 == sslWay ? "单向" : "双向", replyDesc.c_str(), dataTypeDesc.c_str(), lineIntervalDesc.c_str(), crlfDesc.c_str());
        }
        else
        {
            printf("启动服务器: %s:%d, 应答数据: %s, 通知数据: 类型-%s%s, 结束符-%s\n", server.c_str(), port, replyDesc.c_str(),
                   dataTypeDesc.c_str(), lineIntervalDesc.c_str(), crlfDesc.c_str());
        }
        g_server->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
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
            printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"); /* 数据预处理 */
            if (1 == dataType)
            {
                std::vector<unsigned char> data;
                data.insert(data.end(), input, input + strlen(input));
                sendData(data, crlf);
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
                sendData(data, crlf);
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
                            sendData(data, crlf);
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
                            ret = sendData(data, crlf);
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
                            ret = sendData(data, crlf);
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
