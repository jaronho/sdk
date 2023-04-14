#include <iostream>
#include <thread>

#include "../../nsocket/websocket/client.h"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is WebSocket client                                                                              **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-lp]                  client local port (0. auto random), default: 0                                 **\n");
    printf("** [-s]                   server address, default: ws://127.0.0.1                                        **\n");
    printf("** [-p]                   server default port, default: 4444                                             **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-pem]                 specify file format [0-DER, 1-PEM]. default: 1                                 **\n");
    printf("** [-cf]                  specify certificate file. e.g. client.crt, ca.crt                              **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
    printf("** [-w]                   specify ssl way verify [1, 2], default: 1                                      **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    int localPort = 0;
    std::string serverHost;
    int defaultPort = 0;
    int tls = 0;
    int pem = 1;
    std::string certFile;
    std::string privateKeyFile;
    std::string privateKeyFilePwd;
    int way = 1;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-lp")) /* 本机端口 */
        {
            ++i;
            if (i < argc)
            {
                localPort = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-s")) /* 服务器地址 */
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
                defaultPort = atoi(argv[i]);
                ++i;
            }
        }
#if (1 == ENABLE_NSOCKET_OPENSSL)
        else if (0 == strcmp(key, "-pem")) /* 文件格式 */
        {
            ++i;
            if (i < argc)
            {
                pem = atoi(argv[i]);
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
        else if (0 == strcmp(key, "-w")) /* SSL校验 */
        {
            ++i;
            if (i < argc)
            {
                way = atoi(argv[i]);
                ++i;
            }
        }
#endif
        else
        {
            ++i;
        }
    }
    if (localPort < 0 || localPort > 65535)
    {
        localPort = 0;
    }
    if (defaultPort <= 0 || defaultPort > 65535)
    {
        defaultPort = 4444;
    }
    if (serverHost.empty())
    {
        serverHost = "ws://127.0.0.1:" + std::to_string(defaultPort);
    }
    else
    {
        if (std::string::npos != serverHost.find("ws://"))
        {
            tls = 0;
        }
        else if (std::string::npos != serverHost.find("wss://"))
        {
            tls = 1;
        }
        else
        {
            printf("server host [%s] format error\n", serverHost.c_str());
            return 0;
        }
    }
    if (pem < 0)
    {
        pem = 0;
    }
    else if (pem > 1)
    {
        pem = 1;
    }
    if (way < 1)
    {
        way = 1;
    }
    else if (way > 2)
    {
        way = 2;
    }
    auto client = std::make_shared<nsocket::ws::Client>();
    client->setConnectingCallback([&, serverHost, defaultPort]() {
        auto localEndpoint = client->getLocalEndpoint();
        auto clientHost = localEndpoint.address().to_string();
        auto clientPort = localEndpoint.port();
        printf("============================== [%s:%d] on connect [%s] ok\n", clientHost.c_str(), clientPort, serverHost.c_str());
        return nullptr;
    });
    client->setOpenCallback([&]() {
        printf("========================= on open\n");
        std::thread th([&] {
            while (1)
            {
                client->sendPing();
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        });
        th.detach();
    });
    client->setPingCallback([&]() {
        printf("++++++++++++++++++++ on ping\n");
        client->sendPong();
    });
    client->setPongCallback([&]() { printf("++++++++++++++++++++ on pong\n"); });
    auto msger = std::make_shared<nsocket::ws::CliMessager_simple>();
    msger->onMessage = [&](bool isText, const std::string& msg) {
        if (isText)
        {
            printf("++++++++++++++++++++ on message(Text), length: %zu\n", msg.size());
            printf("%s", msg.c_str());
        }
        else
        {
            printf("++++++++++++++++++++ on message(Binary), length: %zu\n", msg.size());
        }
        printf("\n");
    };
    client->setMessager(msger);
    client->setCloseCallback([&](const boost::system::error_code& code) {
        if (code)
        {
            printf("------------------------------ on close, %d, %s\n", code.value(), code.message().c_str());
        }
        else
        {
            printf("------------------------------ on close\n");
        }
    });
    /* 创建线程专门用于网络I/O事件轮询 */
    std::thread th([&, localPort, serverHost, defaultPort, tls, pem, certFile, privateKeyFile, privateKeyFilePwd, way]() {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
            client->setLocalPort(localPort);
#if (1 == ENABLE_NSOCKET_OPENSSL)
            if (0 == tls)
            {
                printf("connect to server: %s\n", serverHost.c_str());
                client->run(serverHost, defaultPort);
            }
            else
            {
                std::shared_ptr<boost::asio::ssl::context> sslContext;
                if (1 == way)
                {
                    sslContext = nsocket::TcpClient::getSsl1WayContext();
                    printf("connect to server: %s, ssl way: 1\n", serverHost.c_str());
                }
                else
                {
                    sslContext = nsocket::TcpClient::getSsl2WayContext(pem ? boost::asio::ssl::context::file_format::pem
                                                                           : boost::asio::ssl::context::file_format::asn1,
                                                                       certFile, privateKeyFile, privateKeyFilePwd);
                    if (sslContext)
                    {
                        printf("connect to server: %s, ssl way: 2%s%s\n", serverHost.c_str(),
                               certFile.empty() ? "" : (", certFile: " + certFile).c_str(),
                               certFile.empty() ? "" : (privateKeyFile.empty() ? "" : (", privateKeyFile: " + privateKeyFile).c_str()));
                    }
                    else
                    {
                        printf("connect to server: %s\n", serverHost.c_str());
                    }
                }
                client->run(serverHost, defaultPort, sslContext);
            }
#else
            printf("connect to server: %s\n", serverHost.c_str());
            client->run(serverHost, defaultPort);
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
        exit(0);
    });
    th.detach();
    try
    {
        /* 主线程 */
        while (1)
        {
            /* 接收输入数据并发送 */
            char str[1024] = {0};
            std::cin.getline(str, sizeof(str));
            if (0 == strlen(str)) /* 输入为空继续等待 */
            {
                continue;
            }
            printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
            if (0 == strcmp(str, "quit") || 0 == strcmp(str, "close"))
            {
                client->sendClose();
                break;
            }
            client->sendText(str);
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
