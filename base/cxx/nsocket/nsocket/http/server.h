#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "../tcp/tcp_server.h"
#include "request.h"
#include "response.h"
#include "router.h"

namespace nsocket
{
namespace http
{
/**
 * @brief HTTP服务器
 */
class Server final : public std::enable_shared_from_this<Server>
{
public:
    /**
     * @brief 构造函数
     * @param name 服务器名称
     * @param threadCount 线程个数
     * @param host 主机地址
     * @param port 端口
     */
    Server(const std::string& name, size_t threadCount, const std::string& host, unsigned int port);

    /**
     * @brief 设置路由未找到回调
     * @param cb 回调
     */
    void setRouterNotFoundCallback(const std::function<void(const REQUEST_PTR& req)>& cb);

    /**
     * @brief 添加路由
     * @param uri URI
     * @param router 路由
     */
    void addRouter(const std::string& uri, const std::shared_ptr<Router>& router);

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
     * @brief HTTP会话
     */
    struct Session
    {
        std::weak_ptr<TcpConnection> wpConn; /* TCP连接 */
        std::shared_ptr<Request> req; /* 请求 */
    };

private:
    /**
     * @brief 处理新连接
     */
    void handleNewConnection(const std::weak_ptr<TcpConnection>& wpConn);

    /**
     * @brief 处理连接数据
     */
    void handleConnectionData(const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data);

    /**
     * @brief 处理连接断开
     */
    void handleConnectionClose(int64_t cid);

    /**
     * @brief 处理请求头
     */
    void handleReqHead(const std::shared_ptr<Session>& session);

    /**
     * @brief 处理请求内容
     */
    void handleReqContent(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen);

    /**
     * @brief 处理请求结束
     */
    void handleReqFinish(const std::shared_ptr<Session>& session);

private:
    std::shared_ptr<TcpServer> m_tcpServer; /* TCP服务器 */
    std::mutex m_mutex;
    std::unordered_map<int64_t, std::shared_ptr<Session>> m_sessionMap; /* 会话表 */
    std::function<void(const REQUEST_PTR& req)> m_routerNotFoundCb; /* 路由未找到回调 */
    std::unordered_map<std::string, std::shared_ptr<Router>> m_routerMap; /* 路由表 */
};
} // namespace http
} // namespace nsocket
