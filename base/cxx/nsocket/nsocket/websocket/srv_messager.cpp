#include "srv_messager.h"

namespace nsocket
{
namespace ws
{
void SrvMessager_batch::onMessageBegin(const std::shared_ptr<Session>& session)
{
    if (beginCb)
    {
        beginCb(session);
    }
}

void SrvMessager_batch::onMessagePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen)
{
    if (payloadCb)
    {
        payloadCb(session, offset, data, dataLen);
    }
}

void SrvMessager_batch::onMessageEnd(const std::shared_ptr<Session>& session)
{
    if (endCb)
    {
        endCb(session);
    }
}

void SrvMessager_simple::onMessageBegin(const std::shared_ptr<Session>& session)
{
    uint64_t id = session->getId();
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_messageMap.end() == m_messageMap.find(id))
    {
        m_messageMap.insert(std::make_pair(id, std::make_shared<std::string>()));
    }
}

void SrvMessager_simple::onMessagePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen)
{
    if (!data || dataLen <= 0)
    {
        return;
    }
    std::shared_ptr<std::string> msg = nullptr;
    {
        /* 限定锁区间, 避免阻塞其他消息, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_messageMap.find(session->getId());
        if (m_messageMap.end() != iter)
        {
            msg = iter->second;
        }
    }
    if (msg)
    {
        msg->insert(msg->end(), data, data + dataLen);
    }
}

void SrvMessager_simple::onMessageEnd(const std::shared_ptr<Session>& session)
{
    std::shared_ptr<std::string> msg = nullptr;
    {
        /* 限定锁区间, 避免阻塞其他消息, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_messageMap.find(session->getId());
        if (m_messageMap.end() != iter)
        {
            msg = iter->second;
            m_messageMap.erase(iter);
        }
    }
    if (msg)
    {
        if (onMessage)
        {
            onMessage(session, session->isMsgText(), *msg.get());
        }
    }
}
} // namespace ws
} // namespace nsocket
