#pragma once
#include "impl/protocol_adapter.h"
#include "logger/logger_manager.h"
#include "nsocket/payload.h"

namespace nac
{
namespace tcli
{
/**
 * @brief 自定义数据包
 */
class PacketCustom final : public Packet
{
public:
    /**
     * @brief 包大小=包头 + 包体(业务数据)
     */
    size_t size() const override
    {
        return (headSize() + data.size());
    }

    /**
     * @brief 包头大小(版本号长度内存空间 + 包体长度内存空间 + 业务码长度内存空间 + 序列ID长度内存空间)
     */
    static uint32_t headSize()
    {
        return (sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t) + sizeof(int64_t));
    }
};

/**
 * @brief 自定义协议适配器
 */
class ProtocolAdapterCustom final : public ProtocolAdapter
{
public:
    /**
     * @brief 构造函数
     * @param version 版本号
     */
    ProtocolAdapterCustom(int32_t version = 0);

    /**
     * @brief 创建数据包
     * @param bizCode 业务码
     * @param seqId 序列ID
     * @param data 业务数据
     * @return 数据包
     */
    std::shared_ptr<Packet> createPacket(int32_t bizCode, int64_t seqId, const std::string& data) override;

private:
    /**
     * @brief 响应连接状态变化
     * @param code 状态码, boost::system::errc::success-连接成功, 其他-连接失败或断开
     */
    void onConnectStatusChanged(const boost::system::error_code& code) override;

    /**
     * @brief 处理数据包转字节流
     * @param pkt 数据包
     * @param buffer [输出]字节流
     * @return true-成功, false-失败
     */
    bool onPacketToBuffer(const std::shared_ptr<Packet>& pkt, std::vector<unsigned char>& buffer) override;

    /**
     * @brief 响应收到数据
     * @param data 数据
     * @return true-数据处理成功, false-数据处理失败
     */
    bool onRecvData(const std::vector<unsigned char>& data) override;

private:
    std::mutex m_mutex;
    std::shared_ptr<nsocket::Payload> m_payload; /* 负载数据 */
    std::shared_ptr<PacketCustom> m_pkt; /* 数据包 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};
} // namespace tcli
} // namespace nac
