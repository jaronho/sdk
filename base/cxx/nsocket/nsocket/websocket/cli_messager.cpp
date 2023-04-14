#include "cli_messager.h"

namespace nsocket
{
namespace ws
{
void CliMessager_batch::onMessageBegin(bool isText)
{
    if (beginCb)
    {
        beginCb(isText);
    }
}

void CliMessager_batch::onMessagePayload(bool isText, size_t offset, const unsigned char* data, int dataLen)
{
    if (payloadCb)
    {
        payloadCb(isText, offset, data, dataLen);
    }
}

void CliMessager_batch::onMessageEnd(bool isText)
{
    if (endCb)
    {
        endCb(isText);
    }
}

void CliMessager_simple::onMessageBegin(bool isText)
{
    m_msg.clear();
}

void CliMessager_simple::onMessagePayload(bool isText, size_t offset, const unsigned char* data, int dataLen)
{
    if (!data || dataLen <= 0)
    {
        return;
    }
    m_msg.insert(m_msg.end(), data, data + dataLen);
}

void CliMessager_simple::onMessageEnd(bool isText)
{
    if (onMessage)
    {
        onMessage(isText, m_msg);
    }
}
} // namespace ws
} // namespace nsocket
