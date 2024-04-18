#pragma once
#include <atomic>
#include <boost/asio/io_context.hpp>
#include <memory>
#include <mutex>
#include <vector>

#include "../core/socket_udp.h"

namespace nsocket
{
/**
 * @brief UDP数据回调
 * @param point 远端端点
 * @param code 错误码
 * @param data 数据
 */
using UDP_DATA_CALLBACK = std::function<void(const boost::asio::ip::udp::endpoint& point, const boost::system::error_code& code,
                                             const std::vector<unsigned char>& data)>;

/**
 * @brief UDP处理器, 注意: 调用close后实例不可再使用, 需要重新创建
 */
class UdpHandler : public std::enable_shared_from_this<UdpHandler>
{
public:
    /**
     * @brief 构造函数
     * @param socket 套接字
     * @param bz 数据缓冲区大小(字节, 选填)
     */
    UdpHandler(const std::shared_ptr<SocketUdpBase>& socket, size_t bz = 65536);

    ~UdpHandler();

    /**
     * @brief 获取处理器ID
     * @return 处理器ID
     */
    uint64_t getId() const;

    /**
     * @brief 获取数据缓冲区大小
     * @return 缓冲区大小(字节)
     */
    size_t getBufferSize() const;

    /**
     * @brief 调整数据缓冲区大小
     * @param bz 缓冲区大小(字节)
     */
    void resizeBuffer(size_t bz);

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
     * @brief 打开
     * @param point 本地端点
     * @param broadcast 是否广播, true-是, false-否
     */
    void open(const boost::asio::ip::udp::endpoint& point, bool broadcast);

    /**
     * @brief 发送数据(同步)
     * @param point 远端端点
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void send(const boost::asio::ip::udp::endpoint& point, const std::vector<unsigned char>& data, const UDP_SEND_CALLBACK& onSendCb);

    /**
     * @brief 关闭
     */
    void close();

    /**
     * @brief 是否已打开
     * @return true-是, false-否
     */
    bool isOpened() const;

    /**
     * @brief 获取本端端点
     * @return 本端端点
     */
    boost::asio::ip::udp::endpoint getLocalEndpoint();

    /**
     * @brief 设置非阻塞(说明: 需要打开成功后才调用)
     * @param nonBlock true-非阻塞, false-阻塞
     * @return true-设置成功, false-设置失败
     */
    bool setNonBlock(bool nonBlock);

private:
    /**
     * @brief 接收数据(只需要调用一次, 内部递归调用)
     */
    void recv();

    /**
     * @brief 关闭(内部实现)
     */
    void closeImpl();

private:
    uint64_t m_id; /* ID */
    std::mutex m_mutex;
    std::shared_ptr<SocketUdpBase> m_socketUdpBase = nullptr; /* 套接字 */
    std::atomic_bool m_isOpened = {false}; /* 是否已打开 */
    std::vector<unsigned char> m_recvBuf; /* 接收缓冲区 */
    UDP_OPEN_CALLBACK m_onOpenCallback = nullptr; /* 打开回调 */
    UDP_DATA_CALLBACK m_onDataCallback = nullptr; /* 数据回调 */
};
} // namespace nsocket
