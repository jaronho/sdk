#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>

#include "../tcp/tcp_client.h"

namespace nsocket
{
namespace ftp
{
enum class ReplyCode
{
    ready = 220,
    user_ok = 331,
    login_ok = 230,
    data_conn_open = 150,
    transfer_complete = 226,
    command_ok = 200,
    path_created = 257,
    closing = 221,
    bad_command = 500,
    file_unavailable = 550
};

/**
 * @brief FTP客户端(注意: 需要实例化为共享指针否则会报错)
 */
class Client final : public std::enable_shared_from_this<Client>
{
public:
    /**
     * @brief 构造函数
     * @param localPort 本地端口, 0表示使用自动分配
     * @param bz 数据缓冲区大小(字节)
     */
    Client(uint16_t localPort = 0, size_t bz = 4096);

    virtual ~Client();

    /**
     * @brief 设置非阻塞(运行前调用才有效)
     * @param nonBlock true-非阻塞, false-阻塞
     */
    void setNonBlock(bool nonBlock);

    /**
     * @brief 设置发送缓冲区大小(运行前调用才有效)
     * @param bufferSize 发送缓冲区大小
     */
    void setSendBufferSize(int bufferSize);

    /**
     * @brief 设置接收缓冲区大小(运行前调用才有效)
     * @param bufferSize 接收缓冲区大小
     */
    void setRecvBufferSize(int bufferSize);

    /**
     * @brief 设置是否启用Nagle算法(运行前调用才有效)
     * @param enable true-启用, false-关闭
     */
    void setNagleEnable(bool enable);

    /**
     * @brief 运行(进入循环, 阻塞和占用调用线程)
     * @param host 远端主机
     * @param port 远端端口
     * @param sslOn 是否开启SSL, true-是, false-否
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt 证书文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件, 例如: client.crt
     * @param pkFile 私钥文件, 例如; client.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     */
    void run(const std::string& host, uint16_t port = 21, bool sslOn = false, int sslWay = 1, int certFmt = 2,
             const std::string& certFile = "", const std::string& pkFile = "", const std::string& pkPwd = "");

    /**
     * @brief 是否在运行
     * @param true-是, false-否
     */
    bool isRunning();

    /**
     * @brief 是否非阻塞模式
     * @return true-非阻塞, false-阻塞
     */
    bool isNonBlock();

    /**
     * @brief 获取发送缓冲区大小
     * @return 发送缓冲区大小
     */
    int getSendBufferSize();

    /**
     * @brief 获取接收缓冲区大小
     * @return 接收缓冲区大小
     */
    int getRecvBufferSize();

    /**
     * @brief 获取是否启用Nagle算法
     * @return true-启用, false-不启用
     */
    bool isNagleEnable();

    /**
     * @brief 获取本端端点
     * @return 本端端点
     */
    boost::asio::ip::tcp::endpoint getLocalEndpoint();

    /**
     * @brief 获取远端端点
     * @return 远端端点
     */
    boost::asio::ip::tcp::endpoint getRemoteEndpoint();

    bool login(const std::string& user, const std::string& password);

private:
    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 处理连接结果
     * @param code 错误码
     */
    void handleConnect(const boost::system::error_code& code);

    /**
     * @brief 处理数据
     * @param data 数据
     */
    void handleData(const std::vector<unsigned char>& data);

    /**
     * @brief 等待响应
     * @param expectedCode 期待应答码
     * @param response [输出]响应内容
     * @param timeout 超时时间(单位: 毫秒)
     */
    bool waitForReply(const ReplyCode& expectedCode, std::string* response, size_t timeout = 5000);

    /**
     * @brief response
     * @param cmd 命令 
     * @param expectedCode 期待应答码
     * @param response 超时时间(单位: 毫秒)
     */
    bool sendCommand(const std::string& cmd, const ReplyCode& expectedCode, std::string* response);

private:
    const uint16_t m_localPort; /* 本地端口, 0表示使用自动分配 */
    const size_t m_bufferSize; /* 数据缓冲区大小(字节) */
    std::mutex m_mutexTcpClient;
    std::shared_ptr<TcpClient> m_tcpClient = nullptr; /* TCP客户端 */
    std::atomic<int> m_nonBlock = {-1}; /* 是否非阻塞: <0-默认, 0-阻塞, 1-非阻塞 */
    std::atomic<int> m_sendBufferSize = {-1}; /* 发送缓冲区大小(字节), <=0-默认, >0-指定大小 */
    std::atomic<int> m_recvBufferSize = {-1}; /* 接收缓冲区大小(字节), <=0-默认, >0-指定大小 */
    std::atomic<int> m_enableNagle = {-1}; /* 是否禁用Nagle算法, <0-默认, 0-禁用, 1-启用 */
    std::mutex m_mutexCmdBuffer;
    std::string m_cmdBuffer; /* 命令缓冲区 */
    std::mutex m_mutexReply;
    std::condition_variable m_cvReply; /* 命令响应条件变量 */
    std::queue<std::pair<ReplyCode, std::string>> m_replyQueue; /* 响应队列 */
};
} // namespace ftp
} // namespace nsocket
