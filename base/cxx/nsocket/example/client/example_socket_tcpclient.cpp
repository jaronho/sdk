#include <iostream>
#include <thread>

#include "../../nsocket/tcp/tcp_client.h"
#include "../net_packet.hpp"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is TCP client                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4335                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-cf]                  specify certificate file. e.g. client.crt                                      **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("** [-id]                  specify client id will use broker, e.g. client_1, client_2                     **\n");
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    std::string certFile;
    std::string privateKeyFile;
    std::string privateKeyFilePwd;
    std::string id;
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
        else if (0 == strcmp(key, "-id")) /* 客户端ID, 当非空时, 将使用代理进行进程间通信(需要基于协议发送数据) */
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
    if (serverHost.empty())
    {
        serverHost = "127.0.0.1";
    }
    if (serverPort <= 0)
    {
        serverPort = 4335;
    }
    std::vector<unsigned char> recvBuffer; /* 客户端接收缓冲区 */
    unsigned int pktBodyLen = 0; /* 需要接收的客户端包体总长度 */
    std::vector<unsigned char> pktBody; /* 已接收的客户端包体内容 */
    auto client = std::make_shared<nsocket::TcpClient>();
    auto handleSend = [&, id](const std::vector<unsigned char>& data) {
        if (id.empty())
        {
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
        else /* 使用broker服务器 */
        {
            std::vector<unsigned char> buffer;
            /* 组装包头: 包头4个字节, 存放包体长度(大端) */
            size_t pktBodyLen = data.size();
            buffer.emplace_back((pktBodyLen >> 24) & 0xFF);
            buffer.emplace_back((pktBodyLen >> 16) & 0xFF);
            buffer.emplace_back((pktBodyLen >> 8) & 0xFF);
            buffer.emplace_back((pktBodyLen >> 0) & 0xFF);
            buffer.insert(buffer.end(), data.begin(), data.end());
            client->send(buffer, [&](const boost::system::error_code& code, std::size_t length) {
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
    };
    auto handleRecv = [&, id](const std::vector<unsigned char>& data) {
        if (id.empty())
        {
            return;
        }
        recvBuffer.insert(recvBuffer.end(), data.begin(), data.end());
        while (recvBuffer.size() > 0)
        {
            if (0 == pktBodyLen) /* 解析包头 */
            {
                if (recvBuffer.size() >= 4)
                {
                    /* 包头4个字节, 存放包体长度(大端) */
                    unsigned int pktHead = 0;
                    pktHead += (unsigned int)recvBuffer[0] << 24;
                    pktHead += (unsigned int)recvBuffer[1] << 16;
                    pktHead += (unsigned int)recvBuffer[2] << 8;
                    pktHead += (unsigned int)recvBuffer[3];
                    if (pktHead > 10 * 1024 * 1024) /* 包体长度大于10M认为是错误的 */
                    {
                        printf("+ protocol illegal\n");
                        recvBuffer.clear();
                        break;
                    }
                    recvBuffer.erase(recvBuffer.begin(), recvBuffer.begin() + 4);
                    pktBodyLen = pktHead;
                }
                else
                {
                    break;
                }
            }
            else /* 解析包体 */
            {
                unsigned int needBodyLen = pktBodyLen - pktBody.size();
                if (needBodyLen > recvBuffer.size()) /* 包不完整 */
                {
                    pktBody.insert(pktBody.end(), recvBuffer.begin(), recvBuffer.end());
                    recvBuffer.clear();
                }
                else /* 包完整 */
                {
                    pktBody.insert(pktBody.end(), recvBuffer.begin(), recvBuffer.begin() + needBodyLen);
                    recvBuffer.erase(recvBuffer.begin(), recvBuffer.begin() + needBodyLen);
                    /* 包处理 */
                    if (pktBody.size() >= 4)
                    {
                        /* 解析消息类型(小端) */
                        unsigned int msgType = 0;
                        msgType += pktBody[0];
                        msgType += pktBody[1];
                        msgType += pktBody[2];
                        msgType += pktBody[3];
                        pktBody.erase(pktBody.begin(), pktBody.begin() + 4);
                        switch (msgType)
                        {
                        case MSG_NOTIFY_RECV_DATA: {
                            notify_recv_data resp;
                            resp.decode(pktBody);
                            printf("+ src id: %s\n", resp.src_id.c_str());
                            printf("+ data: %s\n", std::string(resp.data.begin(), resp.data.end()).c_str());
                        }
                        break;
                        default:
                            printf("+ can't handle unknown net msg type [%u]\n", msgType);
                            break;
                        }
                    }
                    /* 重置包 */
                    pktBodyLen = 0;
                    pktBody.clear();
                }
            }
        }
    };
    /* 设置连接回调 */
    client->setConnectCallback([&, id](const boost::system::error_code& code) {
        if (code)
        {
            printf("============================== on connect fail, %d, %s\n", code.value(), code.message().c_str());
            exit(0);
        }
        else
        {
            printf("============================== on connect ok\n");
            if (!id.empty()) /* 需要向服务器设置本客户端的ID */
            {
                req_set_self_id req;
                req.self_id = id;
                handleSend(req.encode());
            }
        }
    });
    /* 设置接收数据回调 */
    client->setRecvDataCallback([&, id](const std::vector<unsigned char>& data) {
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
        handleRecv(data);
    });
    /* 创建线程专门用于网络I/O事件轮询 */
    printf("connect to server: %s:%d\n", serverHost.c_str(), serverPort);
    std::thread th([&, certFile, privateKeyFile, privateKeyFilePwd]() {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
#if (1 == ENABLE_NSOCKET_OPENSSL)
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
        if (id.empty())
        {
            std::vector<unsigned char> data;
            data.insert(data.end(), str, str + len);
            handleSend(data);
        }
        else
        {
            std::string targetId;
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
            if (0 == pos)
            {
                data.insert(data.end(), str, str + len);
            }
            else
            {
                targetId.insert(targetId.end(), str, str + pos);
                data.insert(data.end(), str + pos + 1, str + len);
            }
            req_send_data req;
            req.target_id = targetId;
            req.data = data;
            handleSend(req.encode());
        }
    }
    return 0;
}
