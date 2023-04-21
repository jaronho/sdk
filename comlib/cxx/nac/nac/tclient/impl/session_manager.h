#pragma once
#include <map>

#include "../../wrapper/session_wrapper.h"
#include "logger/logger_manager.h"
#include "protocol_adapter.h"

namespace nac
{
namespace tcli
{
/**
 * @brief 会话管理器
 */
class SessionManager final : public SessionWrapper
{
public:
    /**
     * @brief 消息接收者
     * @param bizCode 业务码
     * @param seqId 序列ID
     * @param data 数据
     */
    using MsgReceiver = std::function<void(int32_t bizCode, int64_t seqId, const std::string& data)>;

public:
    /**
     * @brief 设置数据通道
     * @param dataChannel 数据层
     */
    void setDataChannel(const std::shared_ptr<DataChannel>& dataChannel);

    /**
     * @brief 设置协议适配器
     * @param adapter 适配器
     */
    void setProtocolAdapter(const std::shared_ptr<ProtocolAdapter>& adapter);

    /**
     * @brief 设置消息接收者
     * @param receiver 接收者
     */
    void setMsgReceiver(const MsgReceiver& receiver);

private:
    /**
     * @brief 获取日志器
     * @return 日志器
     */
    logger::Logger myLogger() override;

    /**
     * @brief 获取定时器执行线程
     * @return 定时器执行线程
     */
    std::shared_ptr<threading::Executor> myTimerExecutor() override;

    /**
     * @brief 发送消息(内部实现)
     * @param bizCode 业务码
     * @param seqId 序列ID
     * @param data 数据
     * @param callback 发送回调
     * @return 序列ID, -1表示失败
     */
    int64_t sendImpl(int32_t bizCode, int64_t seqId, const std::string& data, const SendCallback& callback) override;

    /**
     * @brief 响应数据包
     * @param pkt 数据包
     */
    void onProcessPacket(const std::shared_ptr<Packet>& pkt);

private:
    std::vector<threading::ScopedSignalConnection> m_connections; /* 信号连接 */
    std::weak_ptr<DataChannel> m_wpDataChannel; /* 数据通道 */
    std::weak_ptr<ProtocolAdapter> m_wpProtocolAdapter; /* 协议适配器 */
    MsgReceiver m_msgReceiver = nullptr; /* 消息接收者 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};
} // namespace tcli
} // namespace nac
