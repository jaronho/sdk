#include <iostream>
#include <thread>

#include "../../nsocket/tcp/tcp_client.h"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is TCP client                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-lp]                  client local port (0. auto random), default: 0                                 **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4444                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-tls]                 specify enable ssl [0-disable, 1-enable]. default: 0                           **\n");
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
    int serverPort = 0;
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
                tls = atoi(argv[i]);
                ++i;
            }
        }
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
    if (serverHost.empty())
    {
        serverHost = "127.0.0.1";
    }
    if (serverPort <= 0 || serverPort > 65535)
    {
        serverPort = 4444;
    }
    if (tls < 0)
    {
        tls = 0;
    }
    else if (tls > 1)
    {
        tls = 1;
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
    auto client = std::make_shared<nsocket::TcpClient>();
    /* 设置连接回调 */
    client->setConnectCallback([&](const boost::system::error_code& code) {
        auto localEndpoint = client->getLocalEndpoint();
        auto clientHost = localEndpoint.address().to_string();
        auto clientPort = localEndpoint.port();
        auto remoteEndpoint = client->getRemoteEndpoint();
        auto serverHost = remoteEndpoint.address().to_string();
        auto serverPort = remoteEndpoint.port();
        if (code)
        {
            printf("============================== [%s:%d] on connect [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort,
                   serverHost.c_str(), serverPort, code.value(), code.message().c_str());
            exit(0);
        }
        else
        {
            printf("============================== [%s:%d] on connect [%s:%d] ok\n", clientHost.c_str(), clientPort, serverHost.c_str(),
                   serverPort);
        }
    });
    /* 设置数据回调 */
    client->setDataCallback([&](const std::vector<unsigned char>& data) {
        printf("++++++++++++++++++++ on recv data, length: %d\n", (int)data.size());
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
    });
    /* 创建线程专门用于网络I/O事件轮询 */
    std::thread th([&, localPort, serverHost, serverPort, tls, pem, certFile, privateKeyFile, privateKeyFilePwd, way]() {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
            client->setLocalPort(localPort);
#if (1 == ENABLE_NSOCKET_OPENSSL)
            if (0 == tls)
            {
                printf("connect to server: %s:%d\n", serverHost.c_str(), serverPort);
                client->run(serverHost, serverPort);
            }
            else
            {
                std::shared_ptr<boost::asio::ssl::context> sslContext;
                if (1 == way)
                {
                    sslContext = nsocket::TcpClient::getSsl1WayContext();
                    printf("connect to server: %s:%d, ssl way: 1\n", serverHost.c_str(), serverPort);
                }
                else
                {
                    sslContext = nsocket::TcpClient::getSsl2WayContext(pem ? boost::asio::ssl::context::file_format::pem
                                                                           : boost::asio::ssl::context::file_format::asn1,
                                                                       certFile, privateKeyFile, privateKeyFilePwd);
                    if (sslContext)
                    {
                        printf("connect to server: %s:%d, ssl way: 2%s%s\n", serverHost.c_str(), serverPort,
                               certFile.empty() ? "" : (", certFile: " + certFile).c_str(),
                               certFile.empty() ? "" : (privateKeyFile.empty() ? "" : (", privateKeyFile: " + privateKeyFile).c_str()));
                    }
                    else
                    {
                        printf("connect to server: %s:%d\n", serverHost.c_str(), serverPort);
                    }
                }
                client->run(serverHost, serverPort, sslContext);
            }
#else
            printf("connect to server: %s:%d\n", serverHost.c_str(), serverPort);
            client->run(serverHost, serverPort);
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
                client->stop();
                break;
            }
            std::size_t length;
            auto code = client->send(std::vector<unsigned char>(str, str + strlen(str)), length);
            if (code)
            {
                printf("-------------------- on send fail, %d, %s\n", code.value(), code.message().c_str());
            }
            else
            {
                printf("++++++++++++++++++++ on send ok, length: %d\n", (int)length);
            }
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
