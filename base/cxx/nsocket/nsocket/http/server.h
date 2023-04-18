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
     * @param reuseAddr 是否允许复用端口, 默认不复用
     * @param bz 数据缓冲区大小(字节)
     * @param handshakeTimeout 握手超时时间, 单位: 毫秒
     */
    Server(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, bool reuseAddr = false, size_t bz = 4096,
           size_t handshakeTimeout = 3000);

    virtual ~Server();

    /**
     * @brief 设置路由未找到回调
     * @param cb 回调
     */
    void setRouterNotFoundCallback(const std::function<RESPONSE_PTR(uint64_t cid, const REQUEST_PTR& req)>& cb);

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
     * @param sslOn 是否开启SSL, true-是, false-否
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt (证书/私钥)文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件, 例如: client.crt
     * @param pkFile 私钥文件, 例如; client.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     * @return true-运行中, false-运行失败(服务对象无效导致)
     */
    bool run(bool sslOn = false, int sslWay = 1, int certFmt = 2, const std::string& certFile = "", const std::string& pkFile = "",
             const std::string& pkPwd = "");

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 是否有效
     * @param errorMsg 错误消息
     * @return true-有效, false-无效
     */
    bool isValid(std::string* errorMsg = nullptr);

    /**
     * @brief 是否运行中
     * @return true-运行中, false-非运行中
     */
    bool isRunning();

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
    void handleConnectionClose(uint64_t cid);

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

    /**
     * @brief 发送响应
     */
    void sendResponse(const std::weak_ptr<TcpConnection>& wpConn, std::shared_ptr<Response> resp);

private:
    const std::string m_name; /* 服务名 */
    const size_t m_threadCount; /* 线程个数 */
    const std::string m_host; /* 主机地址 */
    const uint16_t m_port; /* 端口 */
    const bool m_reuseAddr; /* /* 是否允许复用端口 */
    const size_t m_bufferSize; /* 缓冲区大小 */
    const size_t m_handshakeTimeout; /* SSL握手超时(单位: 秒) */
    std::mutex m_mutexTcpServer;
    std::shared_ptr<TcpServer> m_tcpServer = nullptr; /* TCP服务器 */
    std::mutex m_mutexSessionMap;
    std::unordered_map<uint64_t, std::shared_ptr<Session>> m_sessionMap; /* 会话表 */
    std::function<RESPONSE_PTR(uint64_t cid, const REQUEST_PTR& req)> m_routerNotFoundCb = nullptr; /* 路由未找到回调 */
    std::unordered_map<std::string, std::shared_ptr<Router>> m_routerMap; /* 路由表 */
};
} // namespace http
} // namespace nsocket
