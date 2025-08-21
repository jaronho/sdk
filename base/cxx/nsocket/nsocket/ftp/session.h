#pragma once
#include <fstream>
#include <memory>
#include <mutex>
#include <string>

#include "../tcp/tcp_connection.h"
#include "type_def.h"

namespace nsocket
{
namespace ftp
{
/**
 * @brief Ftp会话
 */
class Session : public std::enable_shared_from_this<Session>
{
    friend class Server;

public:
    Session(const std::weak_ptr<TcpConnection>& wpConn, const std::string& rootPath, size_t bz = 4096);
    ~Session();

    /**
     * @brief 获取会话ID
     * @return 会话ID
     */
    uint64_t getId() const;

    /**
     * @brief 获取客户端主机
     * @return 客户端主机
     */
    std::string getClientHost() const;

    /**
     * @brief 获取客户端端口
     * @return 客户端端口
     */
    int getClientPort() const;

    /**
     * @brief 响应收到命令
     * @param data 收到的命令数据
     */
    void onCommandRecv(const std::vector<unsigned char>& data);

    /**
     * @brief 开始处理会话
     */
    void start();

private:
    /**
     * @brief 发送响应
     * @param code 响应码
     * @param msg 响应消息
     */
    void sendReply(const ReplyCode& code, const std::string& msg);

    /**
     * @brief 处理命令
     * @param cmdLine 命令行
     */
    void handleCommand(const std::string& cmdLine);

    void handleUser(const std::string& arg);
    void handlePass(const std::string& arg);
    void handleQuit();
    void handlePort(const std::string& arg);
    void handlePasv();
    void handleType(const std::string& arg);
    void handleRetr(const std::string& arg);
    void handleStor(const std::string& arg);
    void handleList(const std::string& arg);
    void handleNlst(const std::string& arg);
    void handleCwd(const std::string& arg);
    void handleCdup();
    void handlePwd();
    void handleMkd(const std::string& arg);
    void handleRmd(const std::string& arg);
    void handleDele(const std::string& arg);
    void handleRnfr(const std::string& arg);
    void handleRnto(const std::string& arg);
    void handleSyst();
    void handleNoop();

    /**
     * @brief 设置数据连接
     */
    void setupDataConnection();

    /**
     * @brief 关闭数据连接
     */
    void closeDataConnection();

    /**
     * @brief 响应数据连接数据接收
     * @param data 数据
     */
    void onDataConnectionRecv(const std::vector<unsigned char>& data);

    /**
     * @brief 发送文件数据
     * @param path 文件路径
     */
    void sendFileData(const std::string& path);

    /**
     * @brief 接收文件数据
     * @param path 文件路径
     */
    void recvFileData(const std::string& path);

private:
    std::weak_ptr<TcpConnection> m_wpConn; /* 命令连接 */
    const uint32_t m_dataBufferSize; /* 数据连接接收缓冲区大小 */
    std::shared_ptr<boost::asio::ip::tcp::acceptor> m_dataAcceptor = nullptr; /* 数据连接接收器(被动模式才有) */
    std::shared_ptr<TcpConnection> m_dataConn = nullptr; /* 数据连接 */
    std::string m_cmdBuffer; /* 命令缓冲区 */
    std::string m_rootPath; /* 根路径 */
    std::string m_currentPath; /* 当前工作路径 */
};
} // namespace ftp
} // namespace nsocket
