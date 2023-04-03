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

/**
 * @brief 发送数据
 */
void sendData(const std::vector<unsigned char>& data)
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
        }
    }
    else
    {
        printf("-------------------- 发送失败: 客户端为空\n");
    }
}

int main(int argc, char* argv[])
{
    /* 命令参数 */
    cmdline::parser parser;
    parser.set_program_name("TCP客户端");
    parser.add<std::string>("address", 'a', "服务器地址", false, "127.0.0.1");
    parser.add<int>("port", 'p', "服务器端口", false, 4444);
#if (1 == ENABLE_NSOCKET_OPENSSL)
    parser.add<int>("cert_format", 'f', "证书文件格式, 值: 0-DER, 1-PEM, 默认1", false, 1);
    parser.add<std::string>("cert_file", 'c', "证书文件名, 例如: server.crt", false, "");
    parser.add<std::string>("key_file", 'k', "私钥文件名, 例如: server.key", false, "");
    parser.add<std::string>("key_pwd", 's', "私钥文件密码, 例如: 123456", false, "");
    parser.add<int>("ssl_way", 'w', "SSL验证, 值: 1-单向验证, 2-双向验证, 默认1", false, 1);
#endif
    parser.add<int>("data_type", 'd',
                    "数据类型, 值: 1-发送输入(原始数据), 2-发送输入(十六进制), 3-发送文件(原始数据, 全部), 4-发送文件(原始数据, 单行), "
                    "5-发送文件(十六进制, 单行), 默认1",
                    false, 1);
    parser.add<int>("line_interval", 'i', "按单行发送文件数据时, 每行的发送间隔(毫秒), 默认0", false, 0);
    parser.parse_check(argc, argv);
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    auto address = parser.get<std::string>("address");
    auto port = parser.get<int>("port");
    port = (port > 0 && port < 65536) ? port : 4444;
#if (1 == ENABLE_NSOCKET_OPENSSL)
    auto certFormat = parser.get<int>("cert_format");
    certFormat = certFormat < 0 ? 0 : (certFormat > 1 ? 1 : certFormat);
    auto certFile = parser.get<std::string>("cert_file");
    auto privateKeyFile = parser.get<std::string>("key_file");
    auto privateKeyFilePwd = parser.get<std::string>("key_pwd");
    auto way = parser.get<int>("ssl_way");
    way = way < 1 ? 1 : (way > 2 ? 2 : way);
#endif
    auto dataType = parser.get<int>("data_type");
    dataType = dataType < 1 ? 1 : (dataType > 5 ? 1 : dataType);
    auto lineInterval = parser.get<int>("line_interval");
    lineInterval = lineInterval < 0 ? 0 : lineInterval;
    std::string dataTypeDesc, lineIntervalDesc;
    if (1 == dataType)
    {
        dataTypeDesc = "发送输入(原始数据)";
    }
    else if (2 == dataType)
    {
        dataTypeDesc = "发送输入(十六进制)";
    }
    else if (3 == dataType)
    {
        dataTypeDesc = "发送文件(原始数据, 全部)";
    }
    else if (4 == dataType)
    {
        dataTypeDesc = "发送文件(原始数据, 单行)";
        lineIntervalDesc = ", 行发送间隔: " + std::to_string(lineInterval) + "(毫秒)";
    }
    else if (5 == dataType)
    {
        dataTypeDesc = "发送文件(十六进制, 单行)";
        lineIntervalDesc = ", 行发送间隔: " + std::to_string(lineInterval) + "(毫秒)";
    }
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("服务器: %s:%d, SSL验证: %s, 数据类型: %s%s\n", address.c_str(), port, 1 == way ? "单向验证" : "双向验证", dataTypeDesc.c_str(),
           lineIntervalDesc.c_str());
#else
    printf("服务器: %s:%d, 数据类型: %s%s\n", address.c_str(), port, dataTypeDesc.c_str(), lineIntervalDesc.c_str());
#endif
    g_client = std::make_shared<nsocket::TcpClient>();
    /* 设置连接回调 */
    g_client->setConnectCallback([&](const boost::system::error_code& code) {
        auto localEndpoint = g_client->getLocalEndpoint();
        auto clientHost = localEndpoint.address().to_string();
        auto clientPort = localEndpoint.port();
        auto remoteEndpoint = g_client->getRemoteEndpoint();
        auto serverHost = remoteEndpoint.address().to_string();
        auto serverPort = remoteEndpoint.port();
        if (code)
        {
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
    printf("连接服务器 ...\n");
    std::thread th([&, certFile, privateKeyFile, privateKeyFilePwd, way]() {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
#if (1 == ENABLE_NSOCKET_OPENSSL)
            if (certFile.empty())
            {
                g_client->run(address, port);
            }
            else
            {
                std::shared_ptr<boost::asio::ssl::context> sslContext;
                if (1 == way || privateKeyFile.empty()) /* 单向SSL */
                {
                    sslContext = nsocket::TcpClient::getSsl1WayContext(
                        certFormat ? boost::asio::ssl::context::file_format::pem : boost::asio::ssl::context::file_format::asn1, certFile);
                }
                else /* 双向SSL */
                {
                    sslContext = nsocket::TcpClient::getSsl2WayContext(certFormat ? boost::asio::ssl::context::file_format::pem
                                                                                  : boost::asio::ssl::context::file_format::asn1,
                                                                       certFile, privateKeyFile, privateKeyFilePwd);
                }
                g_client->run(address, port, sslContext);
            }
#else
            g_client->run(address, port);
#endif
        }
        catch (const std::exception& e)
        {
            printf("========== 异常: %s\n", e.what());
        }
        catch (...)
        {
            printf("========== 异常: 未知\n");
        }
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
            if (3 == dataType) /* 发送文件(原始数据), 全部 */
            {
                auto fileSize = utility::FileInfo(input).size();
                size_t offset = 0;
                while (offset < fileSize)
                {
                    size_t count = MAX_PAYLOAD;
                    auto buf = utility::FileInfo::read(f, offset, count, false);
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
            else if (4 == dataType) /* 发送文件(原始数据), 单行 */
            {
                while (!feof(f))
                {
                    std::string bomFlag, endFlag;
                    auto line = utility::FileInfo::readLine(f, bomFlag, endFlag);
                    size_t offset = 0;
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
                        sendData(data);
                    }
                    if (lineInterval > 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(lineInterval));
                    }
                }
            }
            else if (5 == dataType) /* 发送文件(十六进制), 单行 */
            {
                while (!feof(f))
                {
                    std::string bomFlag, endFlag;
                    auto buf = utility::FileInfo::readLine(f, bomFlag, endFlag);
                    auto line = utility::StrTool::toLower(buf);
                    line = utility::StrTool::replace(line, "0x", "");
                    line = utility::StrTool::replace(line, " ", "");
                    if (!utility::Digit::isHex(line)) /* 非十六进制 */
                    {
                        printf("-------------------- 发送失败: 数据格式错误(非十六进制), %s\n", buf.c_str());
                        break;
                    }
                    auto bytes = utility::StrTool::fromHex(line);
                    if (bytes.empty())
                    {
                        printf("-------------------- 发送失败: 数据格式错误(非十六进制), %s\n", buf.c_str());
                        continue;
                    }
                    size_t offset = 0;
                    while (offset < line.size())
                    {
                        size_t count = MAX_PAYLOAD - 1;
                        if (count > line.size() - offset)
                        {
                            count = line.size() - offset;
                        }
                        auto buf = line.substr(offset, count);
                        offset += count;
                        auto bytes = utility::StrTool::fromHex(buf);
                        std::vector<unsigned char> data;
                        data.insert(data.end(), bytes.begin(), bytes.end());
                        sendData(data);
                    }
                    if (lineInterval > 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(lineInterval));
                    }
                }
            }
            fclose(f); /* 关闭文件句柄 */
        }
    }
    return 0;
}
