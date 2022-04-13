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
     * @param reuseAddr 是否允许复用端口(选填), 默认不复用
     * @param bz 数据缓冲区大小(字节, 选填)
     */
    Server(const std::string& name, size_t threadCount, const std::string& host, unsigned int port, bool reuseAddr = false,
           size_t bz = 1024);

    /**
     * @brief 是否有效
     * @param errorMsg 错误消息(选填)
     * @return true-有效, false-无效
     */
    bool isValid(std::string* errorMsg = nullptr) const;

    /**
     * @brief 是否运行中
     * @return true-运行中, false-非运行中
     */
    bool isRunning() const;

    /**
     * @brief 设置路由未找到回调
     * @param cb 回调
     */
    void setRouterNotFoundCallback(const std::function<void(const REQUEST_PTR& req)>& cb);

    /**
     * @brief 添加路由
     * @param methods 方法列表, 为空时表示支持所有方法(注意: 正常有且支持1种), 例如: {Method::GET}, {Method::POST}
     * @param uriList 服务URI列表, 例如: {"/", "/index","/index.htm", "/index.html"}
     * @param router 路由
     * @return 返回已添加过的URI列表
     */
    std::vector<std::string> addRouter(const std::vector<Method>& methods, const std::vector<std::string>& uriList,
                                       std::shared_ptr<Router> router);

    /**
     * @brief 运行(非阻塞)
     * @param sslContext TLS上下文(选填), 为空表示不启用TLS
     * @return true-运行中, false-运行失败(服务对象无效导致)
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    bool run(const std::shared_ptr<boost::asio::ssl::context>& sslContext = nullptr);
#else
    bool run();
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
