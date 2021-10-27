#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "../tcp/tcp_server.h"
#include "request.h"
#include "response.h"
#include "session.h"

namespace nsocket
{
namespace ws
{
using WS_REQUEST_CALLBACK = std::function<std::shared_ptr<Response>(const SESSION_PTR& session)>;
using WS_OPEN_CALLBACK = std::function<void(const SESSION_PTR& session)>;
using WS_CLOSE_CALLBACK = std::function<void(const SESSION_PTR& session)>;

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

    void setRequestCallback(const WS_REQUEST_CALLBACK& cb);

    void setOpenCallback(const WS_OPEN_CALLBACK& cb);

    void setCloseCallback(const WS_CLOSE_CALLBACK& cb);

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
    void handleConnectionClose(int64_t sid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code);

    /**
     * @brief 处理请求
     */
    void handleRequest(const std::shared_ptr<Session>& session);

private:
    std::shared_ptr<TcpServer> m_tcpServer; /* TCP服务器 */
    std::mutex m_mutex;
    std::unordered_map<int64_t, std::shared_ptr<Session>> m_sessionMap; /* 会话表 */
    WS_REQUEST_CALLBACK m_onRequestCallback;
    WS_OPEN_CALLBACK m_onOpenCallback;
    WS_CLOSE_CALLBACK m_onCloseCallback;
};
} // namespace ws
} // namespace nsocket
