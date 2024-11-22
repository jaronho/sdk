#pragma once
#include <boost/asio/ip/address.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "tcp_connection.h"

namespace nsocket
{
/**
 * @brief TCP新连接回调
 * @param wpConn 连接
 */
using TCP_SRV_CONN_NEW_CALLBACK = std::function<void(const std::weak_ptr<TcpConnection>& wpConn)>;

/**
 * @brief TLS握手成功回调
 * @param wpConn 连接
 */
using TLS_SRV_HANDSHAKE_OK_CALLBACK = std::function<void(const std::weak_ptr<TcpConnection>& wpConn)>;

/**
 * @brief TLS握手失败回调
 * @param cid 连接ID
 * @param point 远端端点
 * @param code 错误码
 */
using TLS_SRV_HANDSHAKE_FAIL_CALLBACK =
    std::function<void(uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)>;

/**
 * @brief TCP数据回调
 * @param wpConn 连接
 * @param data 数据
 */
using TCP_SRV_CONN_DATA_CALLBACK = std::function<void(const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data)>;

/**
 * @brief TCP连接关闭回调
 * @param cid 连接ID
 * @param point 远端端点
 * @param code 错误码
 */
using TCP_SRV_CONN_CLOSE_CALLBACK =
    std::function<void(uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)>;

/**
 * @brief 上下文线程池
 */
class io_context_pool
{
public:
    /**
     * @brief 构造函数
     * @param name 池名称
     * @param connPoolSize 连接池大小(线程个数)
     */
    explicit io_context_pool(const std::string& name, size_t connPoolSize = 1);

    virtual ~io_context_pool();

    /**
     * @brief 开始
     */
    void start();

    /**
     * @brief 等待退出
     */
    void join();

    /**
     * @brief 获取处理I/O的上下文
     * @return 上下文
     */
    boost::asio::io_context& getIoContext();

    /**
     * @brief 获取处理连接的上下文
     * @return 上下文
     */
    boost::asio::io_context& getConnContext();

private:
    const std::string m_name; /* 名称 */
    std::mutex m_mutex;
    std::shared_ptr<boost::asio::io_context> m_ioContext = nullptr; /* 用于处理I/O的上下文 */
    std::shared_ptr<boost::asio::io_context::work> m_ioWorker = nullptr; /* 用于处理I/O的工作 */
    std::shared_ptr<std::thread> m_ioThread = nullptr; /* 用于处理I/O的线程 */
    std::vector<std::shared_ptr<boost::asio::io_context>> m_connContexts; /* 用于处理连接的上下文 */
    std::vector<std::shared_ptr<boost::asio::io_context::work>> m_connWorkers; /* 用于处理连接的工作 */
    std::vector<std::shared_ptr<std::thread>> m_connThreads; /* 用于处理连接的线程 */
    size_t m_connIndex = 0; /* 当前处理连接的上下文索引 */
};

/**
 * @brief TCP服务端(注意: 1.需要实例化为共享指针否则会报错, 2.停止后需要重新实例化, 不可复用之前的实例)
 */
class TcpServer final : public std::enable_shared_from_this<TcpServer>
{
public:
    /**
     * @brief 构造函数
     * @param name 服务器名称
     * @param threadCount 线程个数
     * @param host 主机
     * @param port 端口
     * @param reuseAddr 是否允许复用端口, 默认不复用
     * @param bz 数据缓冲区大小(字节)
     * @param handshakeTimeout 握手超时时间
     */
    TcpServer(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, bool reuseAddr = false, size_t bz = 4096,
              const std::chrono::steady_clock::duration& handshakeTimeout = std::chrono::seconds(3));

    /**
     * @brief 析构函数, 注意: 不能在回调所在线程中销毁服务端对象(会有线程中销毁所在线程的问题, 将导致程序崩溃)
     */
    virtual ~TcpServer();

    /**
     * @brief 设置新连接回调
     * @param onNewCb 新连接回调
     */
    void setNewConnectionCallback(const TCP_SRV_CONN_NEW_CALLBACK& onNewCb);

    /**
     * @brief 设置握手成功回调
     * @param onHandshakeOkCb 握手成功回调
     */
    void setHandshakeOkCallback(const TLS_SRV_HANDSHAKE_OK_CALLBACK& onHandshakeOkCb);

    /**
     * @brief 设置握手失败回调
     * @param onHandshakeFailCb 握手失败回调
     */
    void setHandshakeFailCallback(const TLS_SRV_HANDSHAKE_FAIL_CALLBACK& onHandshakeFailCb);

    /**
     * @brief 设置数据回调, 注意: 严禁在回调里执行死循环或长时间循环逻辑, 否则会阻塞该连接线程
     * @param onDataCb 数据回调
     */
    void setConnectionDataCallback(const TCP_SRV_CONN_DATA_CALLBACK& onDataCb);

    /**
     * @brief 设置连接关闭回调
     * @param onCloseCb 连接关闭回调
     */
    void setConnectionCloseCallback(const TCP_SRV_CONN_CLOSE_CALLBACK& onCloseCb);

    /**
     * @brief 运行(非阻塞)
     * @param sslOn 是否开启SSL, true-是, false-否
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt (证书/私钥)文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件, 例如: client.crt
     * @param pkFile 私钥文件, 例如; client.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     * @param errorMsg [输出]错误消息
     * @return true-运行中, false-运行失败(服务对象无效导致)
     */
    bool run(bool sslOn = false, int sslWay = 1, int certFmt = 2, const std::string& certFile = "", const std::string& pkFile = "",
             const std::string& pkPwd = "", std::string* errorMsg = nullptr);

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 是否启用SSL
     * @return true-是, false-否
     */
    bool isEnableSSL();

    /**
     * @brief 是否运行中
     * @return true-运行中, false-非运行中
     */
    bool isRunning();

private:
    /**
     * @brief 接收客户端连接请求
     */
    void doAccept();

    /**
     * @brief 处理客户端新连接
     */
    void handleNewConnection(boost::asio::ip::tcp::socket socket);

    /**
     * @brief 处理连接结果
     */
    void handleConnectionResult(const std::shared_ptr<TcpConnection>& conn, const boost::asio::ip::tcp::endpoint& point,
                                const boost::system::error_code& code);

    /**
     * @brief 处理握手
     */
    void handleHandshake(const std::shared_ptr<TcpConnection>& conn, const boost::asio::ip::tcp::endpoint& point);

    /**
     * @brief 处理握手结果
     */
    void handleHandshakeResult(const std::shared_ptr<TcpConnection>& conn, const boost::asio::ip::tcp::endpoint& point,
                               const boost::system::error_code& code);

private:
    const std::shared_ptr<io_context_pool> m_contextPool; /* 上下文线程池 */
    const std::string m_host; /* 主机 */
    const uint16_t m_port; /* 端口 */
    const bool m_reuseAddr; /* 是否复用端口 */
    const uint32_t m_bufferSize; /* 数据接收缓冲区大小 */
    const std::chrono::steady_clock::duration m_handshakeTimeout; /* 握手超时时间 */
    std::mutex m_mutex;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor = nullptr; /* 接收器 */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    std::shared_ptr<boost::asio::ssl::context> m_sslContext = nullptr; /* TLS上下文 */
#endif
    std::unordered_map<uint64_t, std::shared_ptr<TcpConnection>> m_connectionMap; /* 连接表 */
    std::unordered_map<uint64_t, std::shared_ptr<boost::asio::steady_timer>> m_handshakeMap; /* 握手表 */
    TCP_SRV_CONN_NEW_CALLBACK m_onNewConnectionCallback = nullptr; /* 新连接回调 */
    TLS_SRV_HANDSHAKE_OK_CALLBACK m_onHandshakeOkCallback = nullptr; /* 握手成功回调 */
    TLS_SRV_HANDSHAKE_FAIL_CALLBACK m_onHandshakeFailCallback = nullptr; /* 握手成失败回调 */
    TCP_SRV_CONN_DATA_CALLBACK m_onConnectionDataCallback = nullptr; /* 连接数据回调 */
    TCP_SRV_CONN_CLOSE_CALLBACK m_onConnectionCloseCallback = nullptr; /* 连接关闭回调 */
    bool m_running = false; /* 是否运行中 */
};
} // namespace nsocket
