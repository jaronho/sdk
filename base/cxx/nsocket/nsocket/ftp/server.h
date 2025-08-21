#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "../tcp/tcp_server.h"
#include "session.h"

namespace nsocket
{
namespace ftp
{
/**
 * @brief FTP服务器(注意: 需要实例化为共享指针否则会报错)
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
     * @param handshakeTimeout 握手超时时间
     */
    Server(const std::string& name, size_t threadCount, const std::string& host, uint16_t port = 21, bool reuseAddr = false,
           size_t bz = 4096, const std::chrono::steady_clock::duration& handshakeTimeout = std::chrono::seconds(3));

    /**
     * @brief 析构函数, 注意: 不能在回调所在线程中销毁服务端对象(会有线程中销毁所在线程的问题, 将导致程序崩溃)
     */
    virtual ~Server();

    /**
     * @brief 设置根路径
     * @param rootPath 根路径
     */
    void setRootPath(const std::string& rootPath);

    /**
     * @brief 运行(非阻塞)
     * @param sslOn 是否开启SSL, true-是, false-否
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt (证书/私钥)文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件, 例如: client.crt
     * @param pkFile 私钥文件, 例如; client.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     * @param errDesc [输出]错误描述
     * @return true-运行中, false-运行失败
     */
    bool run(bool sslOn = false, int sslWay = 1, int certFmt = 2, const std::string& certFile = "", const std::string& pkFile = "",
             const std::string& pkPwd = "", std::string* errDesc = nullptr);

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 是否运行中
     * @return true-运行中, false-非运行中
     */
    bool isRunning();

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

private:
    std::shared_ptr<TcpServer> m_tcpServer = nullptr; /* TCP服务器 */
    const uint32_t m_dataBufferSize; /* 数据连接接收缓冲区大小 */
    std::mutex m_mutexRootPath;
    std::string m_rootPath; /* 根路径 */
    std::mutex m_mutexSessionMap;
    std::unordered_map<uint64_t, std::shared_ptr<Session>> m_sessionMap; /* 会话表 */
};
} // namespace ftp
} // namespace nsocket
