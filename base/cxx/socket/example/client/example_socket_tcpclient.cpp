#include <iostream>
#include <thread>

#include "../../socket/tcp/tcp_client.h"

int main(int argc, char* argv[])
{
    std::string serverHost;
    int serverPort = 0;
    if (argc >= 3)
    {
        serverHost = argv[1];
        serverPort = atoi(argv[2]);
    }
    if (serverHost.empty() || serverPort <= 0)
    {
        serverHost = "127.0.0.1";
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
    std::thread th([&]() { client->run(serverHost, serverPort); });
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
