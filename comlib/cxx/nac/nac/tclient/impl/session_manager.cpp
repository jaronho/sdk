#include "session_manager.h"

#include "algorithm/snowflake/snowflake.h"
#include "threading/timer/steady_timer.h"

namespace nac
{
namespace tcli
{
void SessionManager::setDataChannel(const std::shared_ptr<DataChannel>& dataChannel)
{
    m_wpDataChannel = dataChannel;
}

void SessionManager::setProtocolAdapter(const std::shared_ptr<ProtocolAdapter>& adapter)
{
    m_connections.clear();
    if (adapter)
    {
        const std::weak_ptr<SessionManager> wpSelf = std::static_pointer_cast<SessionManager>(shared_from_this());
        m_connections.emplace_back(adapter->sigRecvPacket.connect([wpSelf](const std::shared_ptr<Packet>& pkt) -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onProcessPacket(pkt);
            }
        }));
    }
    m_wpProtocolAdapter = adapter;
}

void SessionManager::setMsgReceiver(const MsgReceiver& receiver)
{
    m_msgReceiver = receiver;
}

logger::Logger SessionManager::myLogger()
{
    return m_logger;
}

std::shared_ptr<threading::Executor> SessionManager::myTimerExecutor()
{
    const auto dataChannel = m_wpDataChannel.lock();
    return dataChannel ? dataChannel->getPktExecutor().lock() : nullptr;
}

int64_t SessionManager::sendImpl(int32_t bizCode, int64_t seqId, const std::string& data, const SendCallback& callback)
{
    const auto adapter = m_wpProtocolAdapter.lock();
    if (adapter)
    {
        auto pkt = adapter->createPacket(bizCode, seqId, data);
        TRACE_LOG(m_logger, "准备发送数据, bizCode[{}], seqId[{}], length[{}]", bizCode, seqId, pkt->size());
        if (adapter->sendPacket(pkt, callback)) /* 发送成功, 返回seqId */
        {
            return seqId;
        }
    }
    else
    {
        ERROR_LOG(m_logger, "消息发送错误: 协议适配器为空.");
    }
    return -1;
}

void SessionManager::onProcessPacket(const std::shared_ptr<Packet>& pkt)
{
    if (onResp(pkt->bizCode, pkt->seqId, pkt->size(), pkt->data)) /* 请求响应 */
    {
        return;
    }
    /* 主动通知 */
    TRACE_LOG(m_logger, "收到通知数据, bizCode[{}], seqId[{}], length[{}]", pkt->bizCode, pkt->seqId, pkt->size());
    if (m_msgReceiver)
    {
        auto beg = std::chrono::steady_clock::now();
        m_msgReceiver(pkt->bizCode, pkt->seqId, pkt->data);
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg);
        if (elapsed.count() > 0)
        {
            WARN_LOG(m_logger, "通知数据处理完毕, bizCode[{}], seqId[{}], length[{}], 耗时: {} 毫秒.", pkt->bizCode, pkt->seqId,
                     pkt->size(), elapsed.count());
        }
    }
}
} // namespace tcli
} // namespace nac
