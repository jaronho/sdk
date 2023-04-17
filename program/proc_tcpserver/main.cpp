#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "nsocket/tcp/tcp_server.h"
#include "utility/cmdline/cmdline.h"

std::shared_ptr<nsocket::TcpServer> g_server = nullptr; /* 服务器 */

int main(int argc, char* argv[])
{
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("TCP服务端");
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
    g_server = std::make_shared<nsocket::TcpServer>("tcp_server", 10, server, port);
    if (!g_server->isValid())
    {
        printf("启动服务器失败, 请检查地址[%s]和端口[%d]是否可用\n", server.c_str(), port);
        return 0;
    }
    /* 设置新连接回调 */
    g_server->setNewConnectionCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn) {
        const auto conn = wpConn.lock();
        if (conn)
        {
            auto point = conn->getRemoteEndpoint();
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("============================== 新连接 [%lld][%s:%d]\n", conn->getId(), clientHost.c_str(), clientPort);
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
        g_server->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
        /* 主线程 */
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
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
