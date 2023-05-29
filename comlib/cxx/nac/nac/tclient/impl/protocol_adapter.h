#pragma once
#include <functional>

#include "data_channel.h"
#include "threading/signal/scoped_signal_connection.h"

namespace nac
{
namespace tcli
{
/**
 * @brief 数据包版本不匹配回调
 * @param localVersion 本地版本号
 * @param pktVersion 数据包版本号
 */
using PACKET_VERSION_MISMATCH_CALLBACK = std::function<void(int32_t localVersion, int32_t pktVersion)>;

/**
 * @brief 设置数据包长度异常回调
 * @param maxLength 最大长度
 * @param pktLength 数据包长度
 */
using PACKET_LENGTH_ABNORMAL_CALLBACK = std::function<void(int32_t maxLength, int32_t pktLength)>;

/**
 * @brief 数据包基类
 */
class Packet
{
public:
    /**
     * @brief 包大小
     */
    virtual size_t size() const = 0;

    int32_t version = 0; /* 版本号(4个字节) */
    int32_t bizCode = 0; /* 业务码(4个字节) */
    int64_t seqId = 0; /* 序列ID(8个字节) */
    std::string data; /* 包体(业务数据) */
};

/**
 * @brief 协议适配器基类
 */
class ProtocolAdapter : public std::enable_shared_from_this<ProtocolAdapter>
{
public:
    /**
     * @brief 设置数据通道
     * @param dataChannel 数据通道
     */
    void setDataChannel(const std::shared_ptr<DataChannel>& dataChannel);

    /**
     * @brief 设置数据包版本不匹配回调
     * @param callback 回调
     */
    void setPacketVersionMismatchCallback(const PACKET_VERSION_MISMATCH_CALLBACK& callback);

    /**
     * @brief 设置数据包长度异常回调
     * @param callback 回调
     */
    void setPacketLengthAbnormalCallback(const PACKET_LENGTH_ABNORMAL_CALLBACK& callback);

    /**
     * @brief 发送数据包
     * @param pkt 数据包
     * @param callback 发送回调
     * @return true-数据发送中, false-失败
     */
    bool sendPacket(const std::shared_ptr<Packet>& pkt, const nsocket::TCP_SEND_CALLBACK& callback);

    /**
     * @brief 同步信号: 收到数据包
     * @param pkt 数据包
     */
    threading::BasicSignal<void(const std::shared_ptr<Packet>& pkt)> sigRecvPacket;

    /**
     * @brief 创建数据包
     * @param bizCode 业务码
     * @param seqId 序列ID
     * @param data 业务数据
     * @return 数据包
     */
    virtual std::shared_ptr<Packet> createPacket(int32_t bizCode, int64_t seqId, const std::string& data) = 0;

protected:
    /**
     * @brief 响应连接状态变化
     * @param code 状态码, boost::system::errc::success-连接成功, 其他-连接失败或断开
     */
    virtual void onConnectStatusChanged(const boost::system::error_code& code) = 0;

    /**
     * @brief 处理数据包转字节流
     * @param pkt 数据包
     * @param buffer [输出]字节流
     * @return true-成功, false-失败
     */
    virtual bool onPacketToBuffer(const std::shared_ptr<Packet>& pkt, std::vector<unsigned char>& buffer) = 0;

    /**
     * @brief 响应收到数据
     * @param data 数据
     * @return true-数据处理成功, false-数据处理失败
     */
    virtual bool onRecvData(const std::vector<unsigned char>& data) = 0;

    /**
     * @brief 响应数据包版本不匹配
     * @param localVersion 本地版本号
     * @param pktVersion 数据包版本号
     */
    void onPacketVersionMismatch(int32_t localVersion, int32_t pktVersion);

    /**
     * @brief 响应数据包长度异常
     * @param maxLength 最大长度
     * @param pktLength 数据包长度
     */
    void onPacketLengthAbnormal(int32_t maxLength, int32_t pktLength);

protected:
    std::weak_ptr<DataChannel> m_wpDataChannel; /* 数据通道 */

private:
    std::vector<threading::ScopedSignalConnection> m_connections; /* 信号连接 */
    PACKET_VERSION_MISMATCH_CALLBACK m_packetVersionMismatchCb = nullptr; /* 数据包版本不匹配回调 */
    PACKET_LENGTH_ABNORMAL_CALLBACK m_packetLengthAbnormalCb = nullptr; /* 数据包长度异常回调 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};
} // namespace tcli
} // namespace nac
