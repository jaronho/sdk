#include <iostream>

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
    server->run();
    return 0;
}
