#pragma once
#include "nsocket/payload.h"
#include "nsocket/tcp/tcp_server.h"
#include "rpc_msg.hpp"

namespace rpc
{
/**
 * @brief RPC代理服务
 */
class Broker
{
private:
    class Client; /* 客户端连接 */

public:
    /**
     * @brief 构造函数
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    Broker(const std::string& name, size_t threadCount, const std::string& serverHost, int serverPort, const std::string& certFile = "",
           const std::string& privateKeyFile = "", const std::string& privateKeyFilePwd = "");
#else
    Broker(const std::string& name, size_t threadCount, const std::string& serverHost, int serverPort);
#endif

    /**
     * @brief 运行
     */
    void run();

private:
    /**
     * @brief 处理新连接
     */
    void handleNewConnection(const std::weak_ptr<nsocket::TcpConnection>& wpConn);

    /**
     * @brief 处理接收到连接数据
     */
    void handleRecvConnectionData(const std::weak_ptr<nsocket::TcpConnection>& wpConn, const std::vector<unsigned char>& data);

    /**
     * @brief 处理连接关闭
     */
    void handleConnectionClose(const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code);

    /**
     * @brief 处理客户端消息
     */
    void handleClientMsg(const std::shared_ptr<Client> client, const MsgType& type, utilitiy::ByteArray& ba);

private:
    std::shared_ptr<nsocket::TcpServer> m_tcpServer; /* 服务器 */
    std::string m_certFile;
    std::string m_privateKeyFile;
    std::string m_privateKeyFilePwd;
    std::mutex m_mutex;
    std::map<boost::asio::ip::tcp::endpoint, std::shared_ptr<Client>> m_clientMap; /* 已连接的客户端表 */
};
} // namespace rpc
