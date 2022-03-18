#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>

#include "../../nsocket/websocket/server.h"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is WebSocket server                                                                              **\n");
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
    nsocket::ws::Server server("ws_server", 10, serverHost, serverPort);
    if (!server.isValid())
    {
        printf("server invalid, please check host or port\n");
        return 0;
    }
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
    auto msger = std::make_shared<nsocket::ws::Messager_simple>();
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
            while (1)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    };
    server.setMessager(msger);
    server.setCloseCallback([&](int64_t cid) { printf("------------------------------ client [%lld] on closed\n", cid); });
    try
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        auto sslContext = nsocket::TcpServer::getSsl2WayContext(certFile, privateKeyFile, privateKeyFilePwd, true);
        server.run(sslContext);
#else
        server.run();
#endif
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
