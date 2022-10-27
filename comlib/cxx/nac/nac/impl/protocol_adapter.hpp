#pragma once
#include "data_channel.h"
#include "threading/signal/scoped_signal_connection.h"

namespace nac
{
/**
 * @brief 协议适配器基类
 */
class ProtocolAdapter : public std::enable_shared_from_this<ProtocolAdapter>
{
public:
    /**
     * @brief 数据包
     */
    struct Packet
    {
        /**
         * @brief 包大小=包头(业务数据长度内存空间 + 业务码长度内存空间 + 序列ID长度内存空间) + 包体(业务数据)
         */
        size_t size() const
        {
            return sizeof((int)data.size()) + sizeof(bizCode) + sizeof(seqId) + data.size();
        }

        int32_t bizCode = 0; /* 业务码(4个字节) */
        int64_t seqId = 0; /* 序列ID(8个字节) */
        std::string data; /* 业务数据 */
    };

public:
    /**
     * @brief 设置数据通道
     * @param dataChannel 数据通道
     */
    void setDataChannel(const std::shared_ptr<DataChannel>& dataChannel)
    {
        m_connections.clear();
        if (dataChannel)
        {
            const std::weak_ptr<ProtocolAdapter> wpSelf = shared_from_this();
            m_connections.emplace_back(dataChannel->sigConnectStatus.connect([wpSelf](bool isConnected) -> void {
                const auto self = wpSelf.lock();
                if (self)
                {
                    self->onConnectStatusChanged(isConnected);
                }
            }));
            m_connections.emplace_back(dataChannel->sigRecvData.connect([wpSelf](const std::vector<unsigned char>& data) -> bool {
                const auto self = wpSelf.lock();
                if (self)
                {
                    return self->onRecvData(data);
                }
                return false;
            }));
        }
        m_wpDataChannel = dataChannel;
    }

    /**
     * @brief 发送数据包
     * @param pkt 数据包
     * @param callback 发送回调
     * @return true-数据发送中, false-失败
     */
    virtual bool sendPacket(const std::shared_ptr<Packet>& pkt, const DataChannel::SendCallback& callback) = 0;

    /**
     * @brief 同步信号: 收到数据包
     * @param pkt 数据包
     */
    threading::BasicSignal<void(const std::shared_ptr<Packet>& pkt)> sigRecvPacket;

protected:
    /**
     * @brief 响应连接状态变化
     * @param isConnected 是否已连接
     */
    virtual void onConnectStatusChanged(bool isConnected) = 0;

    /**
     * @brief 响应收到数据
     * @param data 数据
     * @return true-数据处理成功, false-数据处理失败
     */
    virtual bool onRecvData(const std::vector<unsigned char>& data) = 0;

protected:
    std::weak_ptr<DataChannel> m_wpDataChannel; /* 数据通道 */

private:
    std::vector<threading::ScopedSignalConnection> m_connections; /* 信号连接 */
};
} // namespace nac
