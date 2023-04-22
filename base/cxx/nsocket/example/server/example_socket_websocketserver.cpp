#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>

#include "../../nsocket/websocket/server.h"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is WebSocket server                                                                              **\n");
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
    nsocket::ws::Server server("ws_server", 10, serverHost, serverPort);
    server.setConnectingCallback([&](const std::weak_ptr<nsocket::ws::Session>& wpSession) {
        const auto session = wpSession.lock();
        if (session)
        {
            printf("============================== client [%lld] on connecting, URI: %s\n", session->getId(), session->getUri().c_str());
        }
        return nullptr;
    });
    server.setOpenCallback([&](const std::weak_ptr<nsocket::ws::Session>& wpSession) {
        const auto session = wpSession.lock();
        if (session)
        {
            printf("============================== client [%lld] on open, [%s:%d], URI: %s\n", session->getId(),
                   session->getClientHost().c_str(), session->getClientPort(), session->getUri().c_str());
        }
    });
    server.setPingCallback([&](const std::weak_ptr<nsocket::ws::Session>& wpSession) {
        const auto session = wpSession.lock();
        if (session)
        {
            printf("==================== on ping\n");
            session->sendPong();
        }
    });
    server.setPongCallback([&](const std::weak_ptr<nsocket::ws::Session>& wpSession) {
        const auto session = wpSession.lock();
        if (session)
        {
            printf("==================== on pong\n");
        }
    });
    auto msger = std::make_shared<nsocket::ws::SrvMessager_simple>();
    msger->onMessage = [&](const std::weak_ptr<nsocket::ws::Session>& wpSession, bool isText, const std::string& msg) {
        const auto session = wpSession.lock();
        if (session)
        {
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
        }
    };
    server.setMessager(msger);
    server.setCloseCallback([&](uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
        printf("------------------------------ client [%lld] on closed\n", cid);
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
        std::string errDesc;
        if (server.run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd, &errDesc))
        {
            while (1)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        else
        {
            printf("server run fail: %s\n", errDesc.c_str());
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
