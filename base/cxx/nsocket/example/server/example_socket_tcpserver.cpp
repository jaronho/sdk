#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "../../nsocket/tcp/tcp_server.h"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is TCP server                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4444                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-tls]                 specify enable ssl [0-disable, 1-enable]. default: 0                           **\n");
    printf("** [-way]                 specify ssl way verify [1, 2], default: 1                                      **\n");
    printf("** [-pem]                 specify file format [1-DER, 2-PEM]. default: 2                                 **\n");
    printf("** [-cf]                  specify certificate file. e.g. client.crt, ca.crt                              **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    int sslOn = 0;
    int sslWay = 1;
    int certFmt = 2;
    std::string certFile;
    std::string pkFile;
    std::string pkPwd;
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
        else if (0 == strcmp(key, "-tls")) /* 是否启用TLS */
        {
            ++i;
            if (i < argc)
            {
                sslOn = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-way")) /* SSL校验 */
        {
            ++i;
            if (i < argc)
            {
                sslWay = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pem")) /* 文件格式 */
        {
            ++i;
            if (i < argc)
            {
                certFmt = atoi(argv[i]);
                ++i;
            }
        }
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
                pkFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* 私钥文件密码 */
        {
            ++i;
            if (i < argc)
            {
                pkPwd = argv[i];
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
    if (serverPort <= 0 || serverPort > 65535)
    {
        serverPort = 4444;
    }
    if (sslOn < 0)
    {
        sslOn = 0;
    }
    else if (sslOn > 1)
    {
        sslOn = 1;
    }
    if (sslWay < 1)
    {
        sslWay = 1;
    }
    else if (sslWay > 2)
    {
        sslWay = 2;
    }
    if (certFmt < 1)
    {
        certFmt = 1;
    }
    else if (certFmt > 2)
    {
        certFmt = 2;
    }
    auto server = std::make_shared<nsocket::TcpServer>("tcp_server", 10, serverHost, serverPort);
    if (!server->isValid())
    {
        printf("server invalid, please check host[%s] or port[%d]\n", serverHost.c_str(), serverPort);
        return 0;
    }
    /* 设置新连接回调 */
    server->setNewConnectionCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn) {
        const auto conn = wpConn.lock();
        if (conn)
        {
            auto point = conn->getRemoteEndpoint();
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("============================== on new connection [%lld] [%s:%d]\n", conn->getId(), clientHost.c_str(), clientPort);
        }
    });
    /* 设置握手成功回调 */
    server->setHandshakeOkCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn) {
        const auto conn = wpConn.lock();
        if (conn)
        {
            auto point = conn->getRemoteEndpoint();
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("==================== on handshake ok [%lld] [%s:%d]\n", conn->getId(), clientHost.c_str(), clientPort);
        }
    });
    /* 设置握手失败回调 */
    server->setHandshakeFailCallback([&](uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("-------------------- on handshake fail [%lld] [%s:%d], %d, %s\n", cid, clientHost.c_str(), clientPort, code.value(),
               code.message().c_str());
    });
    /* 设置连接数据回调 */
    server->setConnectionDataCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn, const std::vector<unsigned char>& data) {
        const auto conn = wpConn.lock();
        if (conn)
        {
            auto point = conn->getRemoteEndpoint();
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("++++++++++ on recv data [%lld] [%s:%d], length: %d\n", conn->getId(), clientHost.c_str(), clientPort, (int)data.size());
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
            if (0 == str.compare("error"))
            {
                conn->close();
            }
            else
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
                            printf("---------- on send [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort, code.value(),
                                   code.message().c_str());
                        }
                        else
                        {
                            printf("---------- on send [%s:%d] ok, length: %d\n", clientHost.c_str(), clientPort, (int)length);
                        }
                    }
                });
            }
        }
    });
    /* 设置连接关闭回调 */
    server->setConnectionCloseCallback(
        [&](uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            if (code)
            {
                printf("-------------------- on connection closed [%lld] [%s:%d] fail, %d, %s\n", cid, clientHost.c_str(), clientPort,
                       code.value(), code.message().c_str());
            }
            else
            {
                printf("-------------------- on connection closed [%lld] [%s:%d]\n", cid, clientHost.c_str(), clientPort);
            }
        });
    /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
    try
    {
        if (1 == sslOn && !certFile.empty() && !pkFile.empty())
        {
            printf("server: %s:%d, ssl way: %d, certFile: %s, pkFile: %s\n", serverHost.c_str(), serverPort, sslWay, certFile.c_str(),
                   pkFile.c_str());
        }
        else
        {
            printf("server: %s:%d\n", serverHost.c_str(), serverPort);
        }
        server->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
        /* 主线程 */
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    catch (const std::exception& e)
    {
        printf("========== execption: %s\n", e.what());
    }
    catch (...)
    {
        printf("========== execption: unknown\n");
    }
    return 0;
}
