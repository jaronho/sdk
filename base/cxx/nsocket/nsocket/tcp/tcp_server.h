#pragma once
#include <boost/asio/ip/address.hpp>
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
using TCP_CONN_NEW_CALLBACK = std::function<void(const std::weak_ptr<TcpConnection>& wpConn)>;

/**
 * @brief TCP数据回调
 * @param wpConn 连接
 * @param data 数据
 */
using TCP_CONN_DATA_CALLBACK = std::function<void(const std::weak_ptr<TcpConnection>& wpConn, const std::vector<unsigned char>& data)>;

/**
 * @brief TCP连接关闭回调
 * @param cid 连接ID
 * @param point 端点
 * @param code 错误码
 */
using TCP_CONN_CLOSE_CALLBACK =
    std::function<void(int64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)>;

/**
 * @brief 上下文线程池
 */
class io_context_pool
{
public:
    /**
     * @brief 构造函数
     * @param name 池名称
     * @param poolSize 池大小(线程个数)
     */
    explicit io_context_pool(const std::string& name, size_t poolSize = 1);

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
     * @brief 获取IO上下文
     * @return IO上下文
     */
    boost::asio::io_context& getContext();

private:
    std::mutex m_mutex;
    std::vector<std::shared_ptr<boost::asio::io_context>> m_contexts; /* 上下文列表 */
    std::vector<std::shared_ptr<boost::asio::io_context::work>> m_workers; /* 工作列表 */
    std::vector<std::shared_ptr<std::thread>> m_threads; /* 线程列表 */
    size_t m_index = 0; /* 当前上下文索引 */
    std::string m_name; /* 名称 */
};

/**
 * @brief TCP服务端(注意: 需要实例化为共享指针否则会报错, 2.停止后需要重新实例化, 不可复用之前的实例)
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
     * @param reuseAddr 是否允许复用端口(选填), 默认不复用
     * @param bz 数据缓冲区大小(字节, 选填)
     */
    TcpServer(const std::string& name, size_t threadCount, const std::string& host, unsigned int port, bool reuseAddr = false,
              size_t bz = 1024);

    virtual ~TcpServer();

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
     * @brief 设置新连接回调
     * @param onNewCb 新连接回调
     */
    void setNewConnectionCallback(const TCP_CONN_NEW_CALLBACK& onNewCb);

    /**
     * @brief 设置数据回调, 注意: 严禁在回调里执行死循环或长时间循环逻辑, 否则会阻塞该连接线程
     * @param onDataCb 数据回调
     */
    void setConnectionDataCallback(const TCP_CONN_DATA_CALLBACK& onDataCb);

    /**
     * @brief 设置连接关闭回调
     * @param onCloseCb 连接关闭回调
     */
    void setConnectionCloseCallback(const TCP_CONN_CLOSE_CALLBACK& onCloseCb);

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

    /**
     * @brief 停止
     */
    void stop();

#if (1 == ENABLE_NSOCKET_OPENSSL)
    /**
     * @brief 获取SSL(单向向验证)上下文(当证书文件或私钥文件为空时返回空)
     * @param certFile 证书文件, 例如: server.crt
     * @param privateKeyFile 私钥文件, 例如: server.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: qq123456
     * @param allowSelfSigned 是否允许自签证书通过(选填), 默认允许
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> getSsl1WayContext(const std::string& certFile, const std::string& privateKeyFile,
                                                                        const std::string& privateKeyFilePwd, bool allowSelfSigned = true);

    /**
     * @brief 获取SSL(双向验证)上下文(当证书文件或私钥文件为空时返回空)
     * @param certFile 证书文件, 例如: server.crt
     * @param privateKeyFile 私钥文件, 例如: server.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: qq123456
     * @param allowSelfSigned 是否允许自签证书通过(选填), 默认允许
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> getSsl2WayContext(const std::string& certFile, const std::string& privateKeyFile,
                                                                        const std::string& privateKeyFilePwd, bool allowSelfSigned = true);
#endif

private:
    /**
     * @brief 接收客户端连接请求
     */
    void doAccept();

    /**
     * @brief 处理客户端新连接
     */
    void handleNewConnection(boost::asio::ip::tcp::socket socket);

private:
    std::shared_ptr<io_context_pool> m_contextPool; /* 上下文线程池 */
    std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor; /* 接收器 */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    std::shared_ptr<boost::asio::ssl::context> m_sslContext; /* TLS上下文 */
#endif
    size_t m_bufferSize; /* 数据接收缓冲区大小 */
    std::mutex m_mutex;
    std::unordered_map<int64_t, std::shared_ptr<TcpConnection>> m_connectionMap; /* 连接表 */
    TCP_CONN_NEW_CALLBACK m_onNewConnectionCallback; /* 新连接回调 */
    TCP_CONN_DATA_CALLBACK m_onConnectionDataCallback; /* 连接数据回调 */
    TCP_CONN_CLOSE_CALLBACK m_onConnectionCloseCallback; /* 连接关闭回调 */
    std::string m_host; /* 主机 */
    std::atomic_bool m_running = {false}; /* 是否运行中 */
    std::string m_errorMsg; /* 错误消息 */
};
} // namespace nsocket
