#include "payload.h"

#include <stdexcept>
#include <string>

namespace nsocket
{
Payload::Payload(unsigned int headLen) : m_headLen(headLen), m_bodyLen(0)
{
    if (headLen < 4)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] arg 'headLen' = " + std::to_string(headLen) + " is invalid");
    }
}

unsigned int Payload::getHeadLen() const
{
    return m_headLen;
}

void Payload::reset()
{
    m_recvBuffer.clear();
    m_parseStep = ParseStep::head;
    m_bodyLen = 0;
    m_body.clear();
}

void Payload::unpack(const std::vector<unsigned char>& data, const HEAD_CALLBACK& headCb, const BODY_CALLBACK& bodyCb)
{
    m_recvBuffer.insert(m_recvBuffer.end(), data.begin(), data.end());
    while (m_recvBuffer.size() > 0)
    {
        if (ParseStep::head == m_parseStep) /* 解析包头 */
        {
            if (m_recvBuffer.size() >= m_headLen)
            {
                std::vector<unsigned char> head(m_recvBuffer.begin(), m_recvBuffer.begin() + m_headLen);
                m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + m_headLen); /* 去除包头 */
                if (headCb)
                {
                    m_bodyLen = headCb(head);
                }
                else
                {
                    throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                                           + "] arg 'headCb' is invalid");
                }
                if (m_bodyLen < 0) /* 出错 */
                {
                    reset();
                    return;
                }
                else if (0 == m_bodyLen) /* 无包体内容 */
                {
                    if (bodyCb)
                    {
                        bodyCb({});
                    }
                }
                else /* 设置下一步解析包体内容 */
                {
                    m_parseStep = ParseStep::body;
                }
            }
            else
            {
                return;
            }
        }
        else if (ParseStep::body == m_parseStep) /* 解析包体 */
        {
            unsigned int needBodyLen = m_bodyLen - m_body.size(); /* 当前包体还需要的数据长度 */
            if (needBodyLen > m_recvBuffer.size()) /* 包体未接收完毕 */
            {
                m_body.insert(m_body.end(), m_recvBuffer.begin(), m_recvBuffer.end());
                m_recvBuffer.clear();
            }
            else /* 包体已接收完毕 */
            {
                m_body.insert(m_body.end(), m_recvBuffer.begin(), m_recvBuffer.begin() + needBodyLen);
                m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + needBodyLen);
                /* 处理包体 */
                if (bodyCb)
                {
                    bodyCb(m_body);
                }
                /* 重置包数据 */
                m_parseStep = ParseStep::head;
                m_bodyLen = 0;
                m_body.clear();
            }
        }
    }
}
} // namespace nsocket
