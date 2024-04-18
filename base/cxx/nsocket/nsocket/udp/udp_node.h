#pragma once
#include <boost/asio/io_context.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "udp_handler.h"

namespace nsocket
{
/**
 * @brief UDP节点(注意: 1.需要实例化为共享指针否则会报错, 2.套接字关闭后需要重新实例化, 不可复用之前的实例)
 */
class UdpNode final : public std::enable_shared_from_this<UdpNode>
{
public:
    /**
     * @brief 构造函数
     * @param bz 数据缓冲区大小(字节, 选填)
     */
    UdpNode(size_t bz = 65536);

    virtual ~UdpNode();

    /**
     * @brief 设置打开回调
     * @param onOpenCb 打开回调
     */
    void setOpenCallback(const UDP_OPEN_CALLBACK& onOpenCb);

    /**
     * @brief 设置数据回调
     * @param onDataCb 数据回调
     */
    void setDataCallback(const UDP_DATA_CALLBACK& onDataCb);

    /**
     * @brief 运行(进入循环, 阻塞和占用调用线程)
     * @param host 本地地址
     * @param port 本地端口
     * @param broadcast 是否广播, true-是, false-否
     */
    void run(const std::string& host, unsigned int port = 0, bool broadcast = false);

    /**
     * @brief 发送数据(同步)
     * @param host 远端地址
     * @param port 远端端口
     * @param data 数据
     * @param sentLength [输出]已发送数据的长度
     * @param 错误码
     */
    boost::system::error_code send(const std::string& host, unsigned int port, const std::vector<unsigned char>& data, size_t& sentLength);

    /**
     * @brief 发送数据(异步)
     * @param host 远端地址
     * @param port 远端端口
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void sendAsync(const std::string& host, unsigned int port, const std::vector<unsigned char>& data, const UDP_SEND_CALLBACK& onSendCb);

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 是否在运行
     * @param true-是, false-否
     */
    bool isRunning() const;

    /**
     * @brief 获取本端端点
     * @return 本端端点
     */
    boost::asio::ip::udp::endpoint getLocalEndpoint();

private:
    /**
     * @brief 处理打开结果
     * @param code 错误码
     */
    void handleOpen(const boost::system::error_code& code);

private:
    boost::asio::io_context m_ioContext; /* IO上下文 */
    std::mutex m_mutex;
    std::shared_ptr<UdpHandler> m_udpHandler = nullptr; /* UDP处理器 */
    size_t m_bufferSize; /* 数据接收缓冲区大小 */
    UDP_OPEN_CALLBACK m_onOpenCallback = nullptr; /* 打开回调 */
    UDP_DATA_CALLBACK m_onDataCallback = nullptr; /* 数据回调 */
    enum class RunStatus
    {
        idle, /* 空闲 */
        running, /* 运行中 */
        stop /* 停止 */
    };
    std::atomic<RunStatus> m_runStatus = {RunStatus::idle}; /* 运行状态 */
};
} // namespace nsocket
