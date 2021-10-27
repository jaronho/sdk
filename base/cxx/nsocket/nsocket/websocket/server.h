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
 * @brief WebSocket������
 */
class Server final : public std::enable_shared_from_this<Server>
{
public:
    /**
     * @brief ���캯��
     * @param host ������ַ
     * @param port �˿�
     */
    Server(const std::string& host, unsigned int port);

    /**
     * @brief ����
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    void run(const std::shared_ptr<boost::asio::ssl::context>& sslContext = nullptr);
#else
    void run();
#endif

private:
    /**
     * @brief ����������
     */
    void handleNewConnection(const std::weak_ptr<TcpSession>& wpSession);

    /**
     * @brief ������������
     */
    void handleConnectionData(const std::weak_ptr<TcpSession>& wpSession, const std::vector<unsigned char>& data);

    /**
     * @brief �������ӶϿ�
     */
    void handleConnectionClose(int64_t sid);

private:
    std::shared_ptr<TcpServer> m_tcpServer; /* TCP������ */
};
} // namespace ws
} // namespace nsocket
