#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "../tcp/tcp_server.h"

namespace nsocket
{
namespace ws
{
/**
 * @brief WebSocket服务器
 */
class Server final : public std::enable_shared_from_this<Server>
{
public:
    /**
     * @brief 构造函数
     * @param host 主机地址
     * @param port 端口
     */
    Server(const std::string& host, unsigned int port);

    /**
     * @brief 运行
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    void run(const std::shared_ptr<boost::asio::ssl::context>& sslContext = nullptr);
#else
    void run();
#endif

private:
    /**
     * @brief 处理新连接
     */
    void handleNewConnection(const std::weak_ptr<TcpSession>& wpSession);

    /**
     * @brief 处理连接数据
     */
    void handleConnectionData(const std::weak_ptr<TcpSession>& wpSession, const std::vector<unsigned char>& data);

    /**
     * @brief 处理连接断开
     */
    void handleConnectionClose(int64_t sid);

private:
    std::shared_ptr<TcpServer> m_tcpServer; /* TCP服务器 */
};
} // namespace ws
} // namespace nsocket
