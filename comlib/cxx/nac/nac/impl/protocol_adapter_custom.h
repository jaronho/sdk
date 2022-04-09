#pragma once
#include "logger/logger_manager.h"
#include "nsocket/payload.h"
#include "protocol_adapter.hpp"

namespace nac
{
/**
 * @brief JSON协议适配器
 */
class ProtocolAdapterCustom final : public ProtocolAdapter
{
public:
    ProtocolAdapterCustom();

    /**
     * @brief 发送数据包
     * @param pkt 数据包
     * @param callback 发送回调
     * @return true-数据发送中, false-失败
     */
    bool sendPacket(const std::shared_ptr<Packet>& pkt, const DataChannel::SendCallback& callback) override;

private:
    /**
     * @brief 响应连接状态变化
     * @param isConnected 是否已连接
     */
    void onConnectStatusChanged(bool isConnected) override;

    /**
     * @brief 响应收到数据
     * @param data 数据
     * @return true-数据处理成功, false-数据处理失败
     */
    bool onRecvData(const std::vector<unsigned char>& data) override;

private:
    std::mutex m_mutexPayload;
    std::shared_ptr<nsocket::Payload> m_payload; /* 负载数据 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};
} // namespace nac
