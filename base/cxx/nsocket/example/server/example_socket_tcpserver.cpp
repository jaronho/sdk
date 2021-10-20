#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "../../nsocket/tcp/tcp_server.h"

std::recursive_mutex g_mutex;
std::unordered_map<boost::asio::ip::tcp::endpoint, nsocket::TCP_CONN_SEND_HANDLER> g_clientMap; /* 客户端映射表 */

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is TCP server                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4335                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-cf]                  specify certificate file. e.g. server.crt                                      **\n");
    printf("** [-pkf]                 specify private key file, e.g. server.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    std::string certFile;
    std::string privateKeyFile;
    std::string privateKeyFilePwd;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-s")) /* 服务器地址 */
        {
            ++i;
            if (i < argc)
            {
                serverHost = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-p")) /* 服务器端口 */
        {
            ++i;
            if (i < argc)
            {
                serverPort = atoi(argv[i]);
                ++i;
            }
        }
#if (1 == ENABLE_NSOCKET_OPENSSL)
        else if (0 == strcmp(key, "-cf")) /* 证书文件 */
        {
            ++i;
            if (i < argc)
            {
                certFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkf")) /* 私钥文件 */
        {
            ++i;
            if (i < argc)
            {
                privateKeyFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* 私钥文件密码 */
        {
            ++i;
            if (i < argc)
            {
                privateKeyFilePwd = argv[i];
                ++i;
            }
        }
#endif
        else
        {
            ++i;
        }
    }
    if (serverHost.empty())
    {
        serverHost = "127.0.0.1";
    }
    if (serverPort <= 0)
    {
        serverPort = 4335;
    }
    printf("server: %s:%d\n", serverHost.c_str(), serverPort);
    auto server = std::make_shared<nsocket::TcpServer>(serverHost, serverPort);
    /* 设置新连接回调 */
    server->setNewConnectionCallback([&](int64_t sid, const boost::asio::ip::tcp::endpoint& point,
                                         const nsocket::TCP_CONN_SEND_HANDLER& sendHandler,
                                         const nsocket::TCP_CONN_CLOSE_HANDLER& closeHandler) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("============================== on new connection [%lld] [%s:%d]\n", sid, clientHost.c_str(), clientPort);
        std::lock_guard<std::recursive_mutex> locker(g_mutex);
        auto iter = g_clientMap.find(point);
        if (g_clientMap.end() == iter)
        {
            g_clientMap.insert(std::make_pair(point, sendHandler));
        }
    });
    /* 设置连接数据回调 */
    server->setConnectionDataCallback([&](int64_t sid, const boost::asio::ip::tcp::endpoint& point, const std::vector<unsigned char>& data,
                                          const nsocket::TCP_CONN_SEND_HANDLER& sendHandler,
                                          const nsocket::TCP_CONN_CLOSE_HANDLER& closeHandler) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("++++++++++ on recv data [%lld] [%s:%d], length: %d\n", sid, clientHost.c_str(), clientPort, (int)data.size());
        /* 以十六进制格式打印数据 */
        printf("+++++ [hex format]\n");
        for (size_t i = 0; i < data.size(); ++i)
        {
            printf("%02X ", data[i]);
        }
        printf("\n");
        /* 以字符串格式打印数据 */
        printf("+++++ [string format]\n");
        std::string str(data.begin(), data.end());
        printf("%s", str.c_str());
        printf("\n");
        /* 把收到的数据原封不动返回给客户端 */
        sendHandler(data, [&](const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code, std::size_t length) {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            if (code)
            {
                printf("---------- on send [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort, code.value(), code.message().c_str());
            }
            else
            {
                printf("---------- on send [%s:%d] ok, length: %d\n", clientHost.c_str(), clientPort, (int)length);
            }
        });
    });
    /* 设置连接关闭回调 */
    server->setConnectionCloseCallback(
        [&](int64_t sid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            if (code)
            {
                printf("-------------------- on connection closed [%lld] [%s:%d] fail, %d, %s\n", sid, clientHost.c_str(), clientPort,
                       code.value(), code.message().c_str());
            }
            else
            {
                printf("-------------------- on connection closed [%lld] [%s:%d]\n", sid, clientHost.c_str(), clientPort);
            }
            std::lock_guard<std::recursive_mutex> locker(g_mutex);
            auto iter = g_clientMap.find(point);
            if (g_clientMap.end() != iter)
            {
                g_clientMap.erase(iter);
            }
        });
    /* 创建线程专门用于网络I/O事件轮询 */
    std::thread th([&, certFile, privateKeyFile, privateKeyFilePwd]() {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
#if (1 == ENABLE_NSOCKET_OPENSSL)
            auto sslContext = nsocket::TcpServer::getSslContext(certFile, privateKeyFile, privateKeyFilePwd);
            server->run(sslContext);
#else
            server->run();
#endif
        }
        catch (const std::exception& e)
        {
            printf("========== execption: %s\n", e.what());
        }
        catch (...)
        {
            printf("========== execption: unknown\n");
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
