#include <iostream>
#include <thread>

#include "../../socket/tcp/tcp_client.h"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is TCP client                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4335                                                     **\n");
#if (1 == ENABLE_SOCKET_OPENSSL)
    printf("** [-cf]                  specify certificate file. e.g. client.crt                                      **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
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
#if (1 == ENABLE_SOCKET_OPENSSL)
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
    auto client = std::make_shared<nsocket::TcpClient>();
    /* �������ӻص� */
    client->setConnectCallback([&](const boost::system::error_code& code) {
        if (code)
        {
            printf("============================== on connect fail, %d, %s\n", code.value(), code.message().c_str());
            exit(0);
        }
        else
        {
            printf("============================== on connect ok\n");
        }
    });
    /* ���ý������ݻص� */
    client->setRecvDataCallback([&](const std::vector<unsigned char>& data) {
        printf("++++++++++++++++++++ on recv data, length: %d\n", (int)data.size());
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
    });
    /* �����߳�ר����������I/O�¼���ѯ */
    printf("connect to server: %s:%d\n", serverHost.c_str(), serverPort);
    std::thread th([&, certFile, privateKeyFile, privateKeyFilePwd]() {
        /* ע��: ��������쳣����, ��Ϊ�����벻��ʱ�����쳣 */
        try
        {
#if (1 == ENABLE_SOCKET_OPENSSL)
            auto sslContext = nsocket::TcpClient::getSslContext(certFile, privateKeyFile, privateKeyFilePwd);
            client->run(serverHost, serverPort, sslContext);
#else
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
    });
    th.detach();
    /* ���߳� */
    while (1)
    {
        /* �����������ݲ����� */
        char str[1024] = {0};
        std::cin.getline(str, sizeof(str));
        if (0 == strlen(str)) /* ����Ϊ�ռ����ȴ� */
        {
            continue;
        }
        printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
        std::vector<unsigned char> data;
        data.insert(data.end(), str, str + strlen(str));
        client->send(data, [&](const boost::system::error_code& code, std::size_t length) {
            if (code)
            {
                printf("-------------------- on send fail, %d, %s\n", code.value(), code.message().c_str());
            }
            else
            {
                printf("++++++++++++++++++++ on send ok, length: %d\n", (int)length);
            }
        });
    }
    return 0;
}
