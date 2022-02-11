#include <iostream>
#include <thread>

#include "../rpc/rpc_client.h"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is TCP client                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   broker address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   broker port, default: 4335                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-cf]                  specify certificate file. e.g. client.crt                                      **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("** [-id]                  specify client id will use broker, e.g. client_1, client_2                     **\n");
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string brokerHost;
    int brokerPort = 0;
    std::string certFile;
    std::string privateKeyFile;
    std::string privateKeyFilePwd;
    std::string id;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-s")) /* 代理地址 */
        {
            ++i;
            if (i < argc)
            {
                brokerHost = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-p")) /* 代理端口 */
        {
            ++i;
            if (i < argc)
            {
                brokerPort = atoi(argv[i]);
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
        else if (0 == strcmp(key, "-id")) /* 客户端ID */
        {
            ++i;
            if (i < argc)
            {
                id = argv[i];
                ++i;
            }
        }
        else
        {
            ++i;
        }
    }
    if (brokerHost.empty())
    {
        brokerHost = "127.0.0.1";
    }
    if (brokerPort <= 0)
    {
        brokerPort = 4335;
    }
    if (id.empty())
    {
        printf("id must not be empty\n");
        return 0;
    }
    printf("connect to broker: %s:%d\n", brokerHost.c_str(), brokerPort);
#if (1 == ENABLE_NSOCKET_OPENSSL)
    rpc::Client client(id, brokerHost, brokerPort, certFile, privateKeyFile, privateKeyFilePwd);
#else
    rpc::Client client(id, brokerHost, brokerPort);
#endif
    client.setRegHandler([&](const rpc::ErrorCode& code) { printf("register to broker %s\n", rpc::error_desc(code).c_str()); });
    client.setCallHandler([&](const std::string& callId, int proc, const std::vector<unsigned char>& data) {
        printf("++++++++++++++++++++ [%s] call [%d], data length: %d\n", callId.c_str(), proc, (int)data.size());
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
        return data;
    });
    /* 创建线程专门用于网络I/O事件轮询 */
    std::thread th([&]() { client.run(); });
    th.detach();
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
        size_t len = strlen(str);
        std::string replyer;
        int proc = 0;
        std::vector<unsigned char> data;
        size_t pos = 0;
        for (size_t i = 0; i < len; ++i)
        {
            if (':' == str[i])
            {
                pos = i;
                break;
            }
        }
        if (pos > 0)
        {
            replyer.insert(replyer.end(), str, str + pos);
            data.insert(data.end(), str + pos + 1, str + len);
        }
        std::vector<unsigned char> replyData;
        auto code = client.call(replyer, proc, data, replyData, std::chrono::milliseconds(1000));
        std::string result;
        result.insert(result.begin(), replyData.begin(), replyData.end());
        printf("-------------------- call [%s].%d %s, return: %s\n", replyer.c_str(), proc, rpc::error_desc(code).c_str(), result.c_str());
    }
    return 0;
}
