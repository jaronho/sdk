#include "payload.h"

#include <stdexcept>
#include <string>

namespace nsocket
{
Payload::Payload(unsigned int maxBodyLen, bool bigEndium) : m_bodyMaxLen(maxBodyLen), m_bigEndium(bigEndium), m_head(0)
{
    if (maxBodyLen < 4)
    {
        throw std::exception(std::logic_error("arg 'maxBodyLen' = " + std::to_string(maxBodyLen) + " is invalid"));
    }
}

void Payload::reset()
{
    m_recvBuffer.clear();
    m_head = 0;
    m_body.clear();
}

void Payload::unpack(const std::vector<unsigned char>& data, const BODY_CALLBACK& bodyCb, const ERROR_CALLBACK& errorCb)
{
    m_recvBuffer.insert(m_recvBuffer.end(), data.begin(), data.end());
    while (m_recvBuffer.size() > 0)
    {
        if (ParseStep::head == m_parseStep) /* 开始新的包 */
        {
            if (m_recvBuffer.size() >= 4)
            {
                /* 解析包头(包头占4个字节, 用于存放包体长度) */
                unsigned int head = 0;
                if (m_bigEndium) /* 大端存储 */
                {
                    head += m_recvBuffer[0] << 24;
                    head += m_recvBuffer[1] << 16;
                    head += m_recvBuffer[2] << 8;
                    head += m_recvBuffer[3];
                }
                else /* 小端存储 */
                {
                    head += m_recvBuffer[0];
                    head += m_recvBuffer[1] << 8;
                    head += m_recvBuffer[2] << 16;
                    head += m_recvBuffer[3] << 24;
                }
                if (head > m_bodyMaxLen) /* 超过包体允许的最大长度表示出错 */
                {
                    m_recvBuffer.clear();
                    if (errorCb)
                    {
                        errorCb(data);
                    }
                    return;
                }
                m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + 4); /* 去除包头 */
                if (0 == head)
                {
                    if (bodyCb)
                    {
                        bodyCb({});
                    }
                    m_parseStep = ParseStep::head;
                }
                else
                {
                    m_parseStep = ParseStep::body;
                    m_head = head;
                }
            }
            else
            {
                return;
            }
        }
        else if (ParseStep::body == m_parseStep) /* 组装当前包体 */
        {
            unsigned int needBodyLen = m_head - m_body.size(); /* 当前包体还需要的数据长度 */
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
                m_head = 0;
                m_body.clear();
            }
        }
    }
}

void Payload::pack(const unsigned char* body, unsigned int bodyLen, std::vector<unsigned char>& data, bool bigEndium)
{
    data.clear();
    /* 组装包头(包头占4个字节, 用于存放包体长度, 使用大端存储模式) */
    unsigned int head = bodyLen;
    if (bigEndium) /* 大端存储 */
    {
        data.emplace_back((head >> 24) & 0xFF);
        data.emplace_back((head >> 16) & 0xFF);
        data.emplace_back((head >> 8) & 0xFF);
        data.emplace_back((head >> 0) & 0xFF);
    }
    else /* 小端存储 */
    {
        data.emplace_back((head >> 0) & 0xFF);
        data.emplace_back((head >> 8) & 0xFF);
        data.emplace_back((head >> 16) & 0xFF);
        data.emplace_back((head >> 24) & 0xFF);
    }
    /* 组装包体 */
    if (body && bodyLen > 0)
    {
        data.insert(data.end(), body, body + bodyLen);
    }
}

void Payload::pack(const std::vector<unsigned char>& body, std::vector<unsigned char>& data, bool bigEndium)
{
    pack(body.data(), body.size(), data, bigEndium);
}
} // namespace nsocket
