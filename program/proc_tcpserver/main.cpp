#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "nsocket/tcp/tcp_server.h"
#include "utility/cmdline/cmdline.h"
#include "utility/filesystem/file_info.h"

std::recursive_mutex g_mutex;
std::unordered_map<boost::asio::ip::tcp::endpoint, std::weak_ptr<nsocket::TcpConnection>> g_clientMap; /* 客户端映射表 */
std::shared_ptr<nsocket::TcpServer> g_server = nullptr; /* 服务器 */

int main(int argc, char* argv[])
{
    /* 命令参数 */
    cmdline::parser parser;
    parser.set_program_name("TCP服务端");
    parser.add<std::string>("address", 'a', "服务器地址", false, "127.0.0.1");
    parser.add<int>("port", 'p', "服务器端口", false, 4444);
#if (1 == ENABLE_NSOCKET_OPENSSL)
    parser.add<int>("cert_format", 'f', "证书文件格式, 值: 0-DER, 1-PEM, 默认1", false, 1);
    parser.add<std::string>("cert_file", 'c', "证书文件名, 例如: server.crt", false, "");
    parser.add<std::string>("key_file", 'k', "私钥文件名, 例如: server.key", false, "");
    parser.add<std::string>("key_pwd", 's', "私钥文件密码, 例如: 123456", false, "");
    parser.add<int>("ssl_way", 'w', "SSL验证, 值: 1-单向验证, 2-双向验证, 默认1", false, 1);
#endif
    parser.add<int>("reply", 'r', "应答方式, 值: 0-不应答, 1-原数据返回, 默认0", false, 0);
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
    auto reply = parser.get<int>("reply");
    reply = reply < 0 ? 0 : (reply > 1 ? 1 : reply);
    std::string replyDesc;
    if (0 == reply)
    {
        replyDesc = "不应答";
    }
    else if (1 == reply)
    {
        replyDesc = "原数据返回";
    }
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("服务器: %s:%d, SSL验证: %s, 应答: %s\n", address.c_str(), port, 1 == way ? "单向验证" : "双向验证", replyDesc.c_str());
#else
    printf("服务器: %s:%d, 应答: %s\n", address.c_str(), port, replyDesc.c_str());
#endif
    g_server = std::make_shared<nsocket::TcpServer>("tcp_server", 10, address, port);
    if (!g_server->isValid())
    {
        printf("服务器启动失败, 请检查地址和端口是否可用\n");
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
            std::lock_guard<std::recursive_mutex> locker(g_mutex);
            auto iter = g_clientMap.find(point);
            if (g_clientMap.end() == iter)
            {
                g_clientMap.insert(std::make_pair(point, wpConn));
            }
        }
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
                printf("++++++++++ 收到 [%lld][%s:%d] 数据, 长度: %d\n", conn->getId(), clientHost.c_str(), clientPort, (int)data.size());
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
                                printf("---------- 回复 [%lld][%s:%d] 成功, 长度: %d\n", conn->getId(), clientHost.c_str(), clientPort,
                                       (int)length);
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
            std::lock_guard<std::recursive_mutex> locker(g_mutex);
            auto iter = g_clientMap.find(point);
            if (g_clientMap.end() != iter)
            {
                g_clientMap.erase(iter);
            }
        });
    /* 创建线程专门用于网络I/O事件轮询 */
    std::thread th([&, certFile, privateKeyFile, privateKeyFilePwd, way]() {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
#if (1 == ENABLE_NSOCKET_OPENSSL)
            if (certFile.empty())
            {
                g_server->run();
            }
            else
            {
                std::shared_ptr<boost::asio::ssl::context> sslContext;
                if (1 == way) /* 单向SSL */
                {
                    sslContext = nsocket::TcpServer::getSsl1WayContext(certFormat ? boost::asio::ssl::context::file_format::pem
                                                                                  : boost::asio::ssl::context::file_format::asn1,
                                                                       certFile, privateKeyFile, privateKeyFilePwd, true);
                }
                else /* 双向SSL */
                {
                    sslContext = nsocket::TcpServer::getSsl2WayContext(certFormat ? boost::asio::ssl::context::file_format::pem
                                                                                  : boost::asio::ssl::context::file_format::asn1,
                                                                       certFile, privateKeyFile, privateKeyFilePwd, true);
                }
                g_server->run(sslContext);
            }
#else
            g_server->run();
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
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
