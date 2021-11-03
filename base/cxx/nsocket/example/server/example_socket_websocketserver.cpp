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
        if (0 == strcmp(key, "-s")) /* ��������ַ */
        {
            ++i;
            if (i < argc)
            {
                serverHost = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-p")) /* �������˿� */
        {
            ++i;
            if (i < argc)
            {
                serverPort = atoi(argv[i]);
                ++i;
            }
        }
#if (1 == ENABLE_NSOCKET_OPENSSL)
        else if (0 == strcmp(key, "-cf")) /* ֤���ļ� */
        {
            ++i;
            if (i < argc)
            {
                certFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkf")) /* ˽Կ�ļ� */
        {
            ++i;
            if (i < argc)
            {
                privateKeyFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* ˽Կ�ļ����� */
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
    nsocket::ws::Server server(serverHost, serverPort);
    server.setConnectingCallback([&](const nsocket::ws::SESSION_PTR& session) {
        printf("============================== client [%lld] on connecting\n", session->getId());
        return nullptr;
    });
    server.setOpenCallback([&](const nsocket::ws::SESSION_PTR& session) {
        printf("============================== client [%lld] on open\n", session->getId());
    });
    server.setPingCallback([&](const nsocket::ws::SESSION_PTR& session) {
        printf("==================== on ping\n");
        session->sendPong();
    });
    server.setPongCallback([&](const nsocket::ws::SESSION_PTR& session) { printf("==================== on pong\n"); });
    auto msger = std::make_shared<nsocket::ws::Messager_simple>();
    msger->onMessage = [&](const nsocket::ws::SESSION_PTR& session, const std::string& msg) {
        if (session->isMsgText())
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
    server.setMessager(msger);
    server.setCloseCallback([&](int64_t cid) { printf("------------------------------ client [%lld] on closed\n", cid); });
    try
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        auto sslContext = nsocket::TcpServer::getSslContext(certFile, privateKeyFile, privateKeyFilePwd);
        server.run(sslContext);
#else
        server.run();
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
    return 0;
}
