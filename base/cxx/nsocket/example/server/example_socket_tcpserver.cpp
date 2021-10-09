#include <chrono>
#include <iostream>
#include <thread>

#include "../../nsocket/tcp/tcp_server.h"

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
    auto server = std::make_shared<nsocket::TcpServer>(serverHost, serverPort);
    /* ���������ӻص� */
    server->setNewConnectionCallback([&](const boost::asio::ip::tcp::endpoint& point, const nsocket::TCP_CONN_SEND_HANDLER& sendHandler) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("============================== on new connection [%s:%d]\n", clientHost.c_str(), clientPort);
    });
    /* ���ý����������ݻص� */
    server->setRecvConnectionDataCallback([&](const boost::asio::ip::tcp::endpoint& point, const std::vector<unsigned char>& data,
                                              const nsocket::TCP_CONN_SEND_HANDLER& sendHandler) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("++++++++++ on recv data [%s:%d], length: %d\n", clientHost.c_str(), clientPort, (int)data.size());
        /* ��ʮ�����Ƹ�ʽ��ӡ���� */
        printf("+++++ [hex format]\n");
        for (size_t i = 0; i < data.size(); ++i)
        {
            printf("%02X ", data[i]);
        }
        printf("\n");
        /* ���ַ�����ʽ��ӡ���� */
        printf("+++++ [string format]\n");
        std::string str(data.begin(), data.end());
        printf("%s", str.c_str());
        printf("\n");
        /* ���յ�������ԭ�ⲻ�����ظ��ͻ��� */
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
    /* �������ӹرջص� */
    server->setConnectionCloseCallback([&](const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        if (code)
        {
            printf("-------------------- on connection closed [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort, code.value(),
                   code.message().c_str());
        }
        else
        {
            printf("-------------------- on connection closed [%s:%d]\n", clientHost.c_str(), clientPort);
        }
    });
    /* �����߳�ר����������I/O�¼���ѯ */
    std::thread th([&, certFile, privateKeyFile, privateKeyFilePwd]() {
        /* ע��: ��������쳣����, ��Ϊ�����벻��ʱ�����쳣 */
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
    /* ���߳� */
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
