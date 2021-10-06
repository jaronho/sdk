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
    /* 设置连接回调 */
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
    /* 设置接收数据回调 */
    client->setRecvDataCallback([&](const std::vector<unsigned char>& data) {
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
    printf("connect to server: %s:%d\n", serverHost.c_str(), serverPort);
    std::thread th([&]() {
#if (1 == ENABLE_SOCKET_OPENSSL)
        std::string certFile = "client.crt";
        std::string privateKeyFilePwd = "qq123456";
        std::string privateKeyFile = "client.key";
        /* 设置SSL上下文对象 */
        auto sslContext = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23_client);
        sslContext->use_certificate_file(certFile, boost::asio::ssl::context::pem);
        /* 注意: 需要先调用`set_password_callback`再调用`use_private_key_file` */
        sslContext->set_password_callback(
            [privateKeyFilePwd](std::size_t maxLength, boost::asio::ssl::context::password_purpose passwordPurpose) -> std::string {
                return privateKeyFilePwd;
            });
        sslContext->use_private_key_file(privateKeyFile, boost::asio::ssl::context::pem);
        sslContext->set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::verify_fail_if_no_peer_cert);
        sslContext->set_verify_callback([](bool preverified, boost::asio::ssl::verify_context& ctx) -> bool {
            char subject_name[256];
            X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
            X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
            return true; /* 注意: 这里要返回true */
        });
        client->run(serverHost, serverPort, sslContext);
#else
        client->run(serverHost, serverPort);
#endif
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
