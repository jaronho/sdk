#pragma once
#include "nsocket/payload.h"
#include "nsocket/tcp/tcp_server.h"
#include "rpc_msg.hpp"

namespace rpc
{
/**
 * @brief RPC代理服务
 */
class Broker final : public std::enable_shared_from_this<Broker>
{
private:
    class Client; /* 客户端连接 */
    class Session; /* 调用会话 */
    friend class Session;

public:
    /**
     * @brief 构造函数
     * @param name 代理服务运行线程名
     * @param threadCount 运行线程数
     * @param serverHost 服务器地址
     * @param serverPort 服务器端口
     * @param sslOn 是否开启SSL, true-是, false-否
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt (证书/私钥)文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件, 例如: client.crt
     * @param privateKeyFile 私钥文件, 例如: client.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: 123456
     */
    Broker(const std::string& name, size_t threadCount, const std::string& serverHost, int serverPort, bool sslOn = false, int sslWay = 1,
           int certFmt = 2, const std::string& certFile = "", const std::string& privateKeyFile = "",
           const std::string& privateKeyFilePwd = "");

    /**
     * @brief 是否有效
     * @return true-有效, false-无效
     */
    bool isValid() const;

    /**
     * @brief 是否运行中
     * @return true-运行中, false-非运行中
     */
    bool isRunning() const;

    /**
     * @brief 运行(非阻塞)
     * @return true-运行中, false-运行失败(服务对象无效导致)
     */
    bool run();

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
    void handleClientMsg(const std::shared_ptr<Client>& client, const MsgType& type, utility::ByteArray& ba);

    /**
     * @brief 会话超时
     */
    void onSessionTimeout(int64_t seqId);

private:
    std::shared_ptr<nsocket::TcpServer> m_tcpServer; /* 服务器 */
    bool m_sslOn = false;
    int m_sslWay = 1;
    int m_certFmt = 2;
    std::string m_certFile;
    std::string m_privateKeyFile;
    std::string m_privateKeyFilePwd;
    std::mutex m_mutexClientMap;
    std::map<boost::asio::ip::tcp::endpoint, std::shared_ptr<Client>> m_clientMap; /* 已连接的客户端表 */
    std::mutex m_mutexSessionMap;
    std::map<int64_t, std::shared_ptr<Session>> m_sessionMap; /* 会话表 */
};
} // namespace rpc
