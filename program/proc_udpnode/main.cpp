#include <atomic>
#include <chrono>
#include <thread>

#include "nsocket/udp/udp_node.h"
#include "utility/cmdline/cmdline.h"
#include "utility/digit/digit.h"
#include "utility/filesystem/file_info.h"
#include "utility/strtool/strtool.h"

std::shared_ptr<nsocket::UdpNode> g_node = nullptr; /* 节点 */
std::atomic_bool g_opened = {false}; /* 是否已打开 */

/**
 * @brief 发送数据
 */
bool sendData(const std::string& host, unsigned int port, const std::vector<unsigned char>& data)
{
    if (g_node)
    {
        /* 发送数据 */
        std::size_t length;
        auto code = g_node->send(host, port, data, length);
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
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("UDP节点");
    parser.add<std::string>("local-addr", 'L', "本地地址, 默认:", false, "127.0.0.1");
    parser.add<int>("local-port", 'l', "本地端口(0表示自动分配), 默认:", false, 0, cmdline::range(1, 65535));
    parser.add<std::string>("remote-addr", 'R', "远端地址, 默认:", false, "127.0.0.1");
    parser.add<int>("remote-port", 'r', "远端端口, 默认:", false, 0, cmdline::range(1, 65535));
    parser.add<int>("data-type", 'd',
                    "数据类型, 值: 1-输入(原始), 2-输入(十六进制), 3-文件(原始, 全部), 4-文件(原始, 单行), 5-文件(十六进制, 单行), 默认:",
                    false, 1, cmdline::oneof<int>(1, 2, 3, 4, 5));
    parser.add<int>("interval", 'i', "按行发送文件数据时, 每行的发送间隔(毫秒), 默认:", false, 500);
    parser.parse_check(argc, argv, "用法", "选项", "显示帮助信息并退出");
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    auto localAddr = parser.get<std::string>("local-addr");
    auto localPort = parser.get<int>("local-port");
    auto remoteAddr = parser.get<std::string>("remote-addr");
    auto remotePort = parser.get<int>("remote-port");
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
    printf("本地地址: %s:%d, 远端地址: %s:%d, 数据类型: %s%s\n", localAddr.c_str(), localPort, remoteAddr.c_str(), remotePort,
           dataTypeDesc.c_str(), lineIntervalDesc.c_str());
    g_node = std::make_shared<nsocket::UdpNode>();
    /* 设置打开回调 */
    g_node->setOpenCallback([&](const boost::system::error_code& code) {
        auto localEndpoint = g_node->getLocalEndpoint();
        auto clientHost = localEndpoint.address().to_string();
        auto clientPort = localEndpoint.port();
        if (code)
        {
            if (g_opened)
            {
                printf("============================== [%s:%d] 关闭: %d, %s\n", clientHost.c_str(), clientPort, code.value(),
                       code.message().c_str());
            }
            else
            {
                printf("============================== [%s:%d] 打开失败: %d, %s\n", clientHost.c_str(), clientPort, code.value(),
                       code.message().c_str());
            }
            exit(0);
        }
        else
        {
            g_opened = true;
            printf("============================== [%s:%d] 打开成功\n", clientHost.c_str(), clientPort);
        }
    });
    /* 设置数据回调 */
    g_node->setDataCallback(
        [&](const boost::asio::ip::udp::endpoint& point, const boost::system::error_code& code, const std::vector<unsigned char>& data) {
            auto host = point.address().to_string();
            auto port = point.port();
            if (code)
            {
                printf("++++++++++ 接收[%s:%d]数据, 失败: %d, %s\n", host.c_str(), port, code.value(), code.message().c_str());
            }
            else
            {
                printf("++++++++++ 收到[%s:%d]数据, 长度: %zu\n", host.c_str(), port, data.size());
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
            }
        });
    /* 创建线程专门用于网络I/O事件轮询 */
    std::thread th([&, localAddr, localPort]() {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
            g_node->run(localAddr, localPort);
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
            sendData(remoteAddr, remotePort, data);
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
            sendData(remoteAddr, remotePort, data);
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
                    auto buf = utility::FileInfo::read(f, offset, count, false);
                    offset += count;
                    if (buf)
                    {
                        std::vector<unsigned char> data;
                        data.insert(data.end(), buf, buf + count);
                        sendData(remoteAddr, remotePort, data);
                        free(buf);
                    }
                }
            }
            else if (4 == dataType) /* 发送文件(原始), 单行 */
            {
                while (!feof(f))
                {
                    std::string bomFlag, endFlag;
                    auto line = utility::FileInfo::readLine(f, bomFlag, endFlag);
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
                        ret = sendData(remoteAddr, remotePort, data);
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
                    std::string bomFlag, endFlag;
                    auto buf = utility::FileInfo::readLine(f, bomFlag, endFlag);
                    if (buf.empty())
                    {
                        continue;
                    }
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
                    bool ret = false;
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
                        ret = sendData(remoteAddr, remotePort, data);
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
