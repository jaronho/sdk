#pragma once
#include <chrono>

#include "logger/logger_manager.h"
#include "nsocket/tcp/tcp_client.h"
#include "threading/signal/basic_signal.h"
#include "threading/thread_proxy.hpp"

namespace nac
{
namespace tcli
{
/**
 * @brief 网络数据通道
 */
class DataChannel final : public std::enable_shared_from_this<DataChannel>
{
public:
    /**
     * @brief 获取报文处理线程
     */
    std::weak_ptr<threading::Executor> getPktExecutor();

    /**
     * @brief 连接(异步)
     * @param localPort 本地端口, 0-使用自动随机分配的端口
     * @param address 服务器地址
     * @param port 服务器端口
     * @param sslOn 是否开启SSL验证
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt (证书/私钥)文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件(选填), 例如: client.crt
     * @param pkFile 私钥文件(选填), 例如: client.key
     * @param pkPwd 私钥文件密码(选填), 例如: qq123456
     * @return true-请求连接中, false-失败
     */
    bool connect(unsigned short localPort, const std::string& address, unsigned short port, bool sslOn = false, int sslWay = 1,
                 int certFmt = 2, const std::string& certFile = "", const std::string& pkFile = "", const std::string& pkPwd = "");

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 连接是否已打开
     * @return true-已打开, false-未打开
     */
    bool isOpened() const;

    /**
     * @brief 获取本端端点
     * @return 本端端点
     */
    boost::asio::ip::tcp::endpoint getLocalEndpoint() const;

    /**
     * @brief 发送数据(异步)
     * @param data 数据
     * @param callback 回调
     * @return true-数据发送中, false-失败
     */
    bool sendData(const std::vector<unsigned char>& data, const nsocket::TCP_SEND_CALLBACK& callback);

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
    threading::ExecutorPtr m_tcpExecutor = threading::ThreadProxy::createAsioExecutor("nac::tcli::loop", 1); /* TCP报文收发线程 */
    threading::ExecutorPtr m_pktExecutor = threading::ThreadProxy::createAsioExecutor("nac::tcli::pkt", 1); /* 报文处理线程 */
    std::shared_ptr<nsocket::TcpClient> m_tcpClient; /* TCP客户端 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};
} // namespace tcli
} // namespace nac
