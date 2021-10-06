#include <chrono>
#include <iostream>
#include <thread>

#include "../../socket/tcp/tcp_server.h"

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
    printf("server: %s:%d\n", serverHost.c_str(), serverPort);
    auto server = std::make_shared<nsocket::TcpServer>(serverHost, serverPort);
    /* 设置新连接回调 */
    server->setNewConnectionCallback([&](const boost::asio::ip::tcp::endpoint& point, const nsocket::TCP_CONN_SEND_HANDLER& sendHandler) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("============================== on new connection [%s:%d]\n", clientHost.c_str(), clientPort);
    });
    /* 设置接收连接数据回调 */
    server->setRecvConnectionDataCallback([&](const boost::asio::ip::tcp::endpoint& point, const std::vector<unsigned char>& data,
                                              const nsocket::TCP_CONN_SEND_HANDLER& sendHandler) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("++++++++++ on recv data [%s:%d], length: %d\n", clientHost.c_str(), clientPort, (int)data.size());
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
        /* 把收到的数据原封不动返回给客户端 */
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
    /* 设置连接关闭回调 */
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
    /* 创建线程专门用于网络I/O事件轮询 */
    std::thread th([&]() {
#if (1 == ENABLE_SOCKET_OPENSSL)
        std::string certFile = "server.crt";
        std::string privateKeyFilePwd = "qq123456";
        std::string privateKeyFile = "server.key";
        /* 设置SSL上下文对象 */
        auto sslContext = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23_server);
        sslContext->use_certificate_file(certFile, boost::asio::ssl::context::pem);
        ///* 注意: 需要先调用`set_password_callback`再调用`use_private_key_file` */
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
        server->run(sslContext);
#else
        server->run();
#endif
    });
    th.detach();
    /* 主线程 */
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
