#pragma once
#include <chrono>

#include "logger/logger_manager.h"
#include "nsocket/tcp/tcp_client.h"
#include "threading/signal/basic_signal.h"
#include "threading/thread_proxy.hpp"

namespace nac
{
/**
 * @brief 网络数据通道
 */
class DataChannel final : public std::enable_shared_from_this<DataChannel>
{
public:
    /**
     * @brief 数据发送回调
     * @param ok 是否成功
     * @param length 数据已发送长度
     */
    using SendCallback = std::function<void(bool ok, size_t length)>;

public:
    /**
     * @brief 设置报文处理线程
     * @param wpHandleExecutor 报文处理线程
     */
    void setHandleExecutor(const std::weak_ptr<threading::Executor>& wpHandleExecutor);

    /**
     * @brief 连接(异步)
     * @param address 服务器地址
     * @param port 服务器端口
     * @param certFile 证书文件(选填), 例如: client.crt
     * @param privateKeyFile 私钥文件(选填), 例如: client.key
     * @param privateKeyFilePwd 私钥文件密码(选填), 例如: qq123456
     * @return true-请求连接中, false-失败
     */
    bool connect(const std::string& address, unsigned short port, const std::string& certFile = "", const std::string& privateKeyFile = "",
                 const std::string& privateKeyFilePwd = "");

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 连接是否已打开
     * @return true-已打开, false-未打开
     */
    bool isOpened();

    /**
     * @brief 发送数据(异步)
     * @param data 数据
     * @param callback 回调
     * @return true-数据发送中, false-失败
     */
    bool sendData(const std::vector<unsigned char>& data, const SendCallback& callback);

    /**
     * @brief 同步信号: 连接状态变化
     * @param isConnected 是否已连接
     */
    threading::BasicSignal<void(bool isConnected)> sigConnectStatus;

    /**
     * @brief 同步信号: 收到数据
     * @param data 数据
     * @return true-数据处理成功, false-数据处理失败
     */
    threading::BasicSignal<bool(const std::vector<unsigned char>& data)> sigRecvData;

    /**
     * @brief 同步信号: 更新接收时间
     * @param tp 时间点
     */
    threading::BasicSignal<void(std::chrono::steady_clock::time_point tp)> sigUpdateRecvTime;

    /**
     * @brief 同步信号: 更新发送时间
     * @param tp 时间点
     */
    threading::BasicSignal<void(std::chrono::steady_clock::time_point tp)> sigUpdateSendTime;

private:
    /**
     * @brief 断开连接
     */
    void disconnectImpl();

    /**
     * @brief 响应连接结果
     */
    void onConnected(const boost::system::error_code& code);

    /**
     * @brief 响应数据接收
     */
    void onRecvData(const std::vector<unsigned char>& data);

private:
    threading::ExecutorPtr m_tcpExecutor = threading::ThreadProxy::createAsioExecutor("nac::loop", 1); /* TCP报文收发线程 */
    std::weak_ptr<threading::Executor> m_wpHandleExecutor; /* 报文处理线程 */
    std::shared_ptr<nsocket::TcpClient> m_tcpClient; /* TCP客户端 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};
} // namespace nac
