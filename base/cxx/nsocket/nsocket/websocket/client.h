#pragma once
#include <atomic>
#include <mutex>
#include <string>

#include "../tcp/tcp_client.h"
#include "cli_messager.h"
#include "frame.h"
#include "request.h"
#include "response.h"

namespace nsocket
{
namespace ws
{
/**
 * @brief 连接中回调
 * @return 客户端发给服务端的请求
 */
using WS_CLI_CONNECTING_CALLBACK = std::function<std::shared_ptr<Request>()>;

/**
 * @brief 连接打开回调
 */
using WS_CLI_OPEN_CALLBACK = std::function<void()>;

/**
 * @brief ping回调
 */
using WS_CLI_PING_CALLBACK = std::function<void()>;

/**
 * @brief pong回调
 */
using WS_CLI_PONG_CALLBACK = std::function<void()>;

/**
 * @brief 连接关闭回调
 * @param code 错误码
 */
using WS_CLI_CLOSE_CALLBACK = std::function<void(const boost::system::error_code& code)>;

/**
 * @brief WebSocket客户端(注意: 需要实例化为共享指针否则会报错)
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
     * @brief 设置连接中回调
     * @param cb 连接中回调
     */
    void setConnectingCallback(const WS_CLI_CONNECTING_CALLBACK& cb);

    /**
     * @brief 设置连接打开回调
     * @param cb 连接打开回调
     */
    void setOpenCallback(const WS_CLI_OPEN_CALLBACK& cb);

    /**
     * @brief 设置ping回调
     * @param cb ping回调
     */
    void setPingCallback(const WS_CLI_PING_CALLBACK& cb);

    /**
     * @brief 设置pong回调
     * @param cb pong回调
     */
    void setPongCallback(const WS_CLI_PONG_CALLBACK& cb);

    /**
     * @brief 设置消息接收者
     * @param msger 消息接收者
     */
    void setMessager(const std::shared_ptr<CliMessager>& msger);

    /**
     * @brief 设置连接关闭回调
     * @param cb 连接关闭回调
     */
    void setCloseCallback(const WS_CLI_CLOSE_CALLBACK& cb);

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
     * @param hostPortPath 远端主机端口路径, 例如: ws://127.0.0.1:4444/echo
     * @param defaultPort 默认远端端口, 当hostPortPath没有包含端口时则使用默认端口
     * @param sslOn 是否开启SSL, true-是, false-否
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt 证书文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件, 例如: client.crt
     * @param pkFile 私钥文件, 例如; client.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     */
    void run(const std::string& hostPortPath, uint16_t defaultPort = 80, bool sslOn = false, int sslWay = 1, int certFmt = 2,
             const std::string& certFile = "", const std::string& pkFile = "", const std::string& pkPwd = "");

    /**
     * @brief 发送文本
     * @param text 文本数据
     * @param isFin 是否最后一个帧
     * @param onSendCb 发送回调
     */
    void sendText(const std::string& text, bool isFin = true, const TCP_SEND_CALLBACK& onSendCb = nullptr);

    /**
     * @brief 发送二进制
     * @param text 二进制数据
     * @param isFin 是否最后一个帧
     * @param onSendCb 发送回调
     */
    void sendBytes(const std::vector<unsigned char>& bytes, bool isFin = true, const TCP_SEND_CALLBACK& onSendCb = nullptr);

    /**
     * @brief 发送ping命令
     * @param onSendCb 发送回调
     */
    void sendPing(const TCP_SEND_CALLBACK& onSendCb = nullptr);

    /**
     * @brief 发送pong命令
     * @param onSendCb 发送回调
     */
    void sendPong(const TCP_SEND_CALLBACK& onSendCb = nullptr);

    /**
     * @brief 发送关闭命令
     * @param code 关闭命令码
     * @param onSendCb 发送回调
     */
    void sendClose(const CloseCode& code = CloseCode::close_normal, const TCP_SEND_CALLBACK& onSendCb = nullptr);

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

    /**
     * @brief 获取远端URI
     * @return 远端URI
     */
    std::string getUri() const;

private:
    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 处理连接结果
     */
    void handleConnect(const boost::system::error_code& code);

    /**
     * @brief 处理数据
     */
    void handleData(const std::vector<unsigned char>& data);

    /**
     * @brief 处理响应
     */
    void handleResponse();

    /**
     * @brief 处理帧头
     */
    void handleFrameHead();

    /**
     * @brief 处理帧负载
     */
    void handleFramePayload(size_t offset, const unsigned char* data, int dataLen);

    /**
     * @brief 处理帧结束
     */
    void handleFrameFinish();

private:
    const uint16_t m_localPort; /* 本地端口, 0表示使用自动分配 */
    const size_t m_bufferSize; /* 数据缓冲区大小(字节) */
    std::mutex m_mutexTcpClient;
    std::shared_ptr<TcpClient> m_tcpClient = nullptr; /* TCP客户端 */
    std::string m_hostPort; /* 远端主机端口 */
    std::string m_uri; /* 远端URI */
    std::string m_secWebSocketKey; /* 安全密钥 */
    std::shared_ptr<Response> m_resp = nullptr; /* 响应 */
    std::shared_ptr<Frame> m_frame = nullptr; /* 数据帧 */
    bool m_isMsgText = false; /* 当前消息是否为文本 */
    WS_CLI_CONNECTING_CALLBACK m_onConnectingCallback = nullptr; /* 连接中回调 */
    WS_CLI_OPEN_CALLBACK m_onOpenCallback = nullptr; /* 连接打开回调 */
    WS_CLI_PING_CALLBACK m_onPingCallback = nullptr; /* ping回调 */
    WS_CLI_PONG_CALLBACK m_onPongCallback = nullptr; /* pong回调 */
    std::shared_ptr<CliMessager> m_messager = nullptr; /* 消息接收者 */
    WS_CLI_CLOSE_CALLBACK m_onCloseCallback = nullptr; /* 连接关闭回调 */
    std::atomic<int> m_nonBlock = {-1}; /* 是否非阻塞: <0-默认, 0-阻塞, 1-非阻塞 */
    std::atomic<int> m_sendBufferSize = {-1}; /* 发送缓冲区大小(字节), <=0-默认, >0-指定大小 */
    std::atomic<int> m_recvBufferSize = {-1}; /* 接收缓冲区大小(字节), <=0-默认, >0-指定大小 */
    std::atomic<int> m_enableNagle = {-1}; /* 是否禁用Nagle算法, <0-默认, 0-禁用, 1-启用 */
};
} // namespace ws
} // namespace nsocket
