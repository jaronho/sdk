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
    m_parseStep = ParseStep::head;
    m_head = 0;
    m_type = 0;
    m_seq = 0;
    m_body.clear();
}

void Payload::unpack(const std::vector<unsigned char>& data, const BODY_CALLBACK& bodyCb, const ERROR_CALLBACK& errorCb)
{
    m_recvBuffer.insert(m_recvBuffer.end(), data.begin(), data.end());
    while (m_recvBuffer.size() > 0)
    {
        if (ParseStep::head == m_parseStep) /* 解析包头 */
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
        else if (ParseStep::body == m_parseStep) /* 解析包体 */
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
                m_type = 0;
                m_seq = 0;
                m_body.clear();
            }
        }
    }
}

void Payload::unpack(const std::vector<unsigned char>& data, const TYPE_BODY_CALLBACK& typeBodyCb, const ERROR_CALLBACK& errorCb)
{
    m_recvBuffer.insert(m_recvBuffer.end(), data.begin(), data.end());
    while (m_recvBuffer.size() > 0)
    {
        if (ParseStep::head == m_parseStep) /* 解析包头 */
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
                m_parseStep = ParseStep::type;
                m_head = head;
            }
            else
            {
                return;
            }
        }
        else if (ParseStep::type == m_parseStep) /* 解析类型 */
        {
            if (m_recvBuffer.size() >= 4)
            {
                /* 解析类型(类型占4个字节) */
                unsigned int type = 0;
                if (m_bigEndium) /* 大端存储 */
                {
                    type += m_recvBuffer[0] << 24;
                    type += m_recvBuffer[1] << 16;
                    type += m_recvBuffer[2] << 8;
                    type += m_recvBuffer[3];
                }
                else /* 小端存储 */
                {
                    type += m_recvBuffer[0];
                    type += m_recvBuffer[1] << 8;
                    type += m_recvBuffer[2] << 16;
                    type += m_recvBuffer[3] << 24;
                }
                m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + 4); /* 去除类型 */
                if (0 == m_head)
                {
                    if (typeBodyCb)
                    {
                        typeBodyCb(type, {});
                    }
                    m_parseStep = ParseStep::head;
                }
                else
                {
                    m_parseStep = ParseStep::body;
                    m_type = type;
                }
            }
            else
            {
                return;
            }
        }
        else if (ParseStep::body == m_parseStep) /* 解析包体 */
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
                if (typeBodyCb)
                {
                    typeBodyCb(m_type, m_body);
                }
                /* 重置包数据 */
                m_parseStep = ParseStep::head;
                m_head = 0;
                m_type = 0;
                m_seq = 0;
                m_body.clear();
            }
        }
    }
}

void Payload::unpack(const std::vector<unsigned char>& data, const TYPE_SEQ_BODY_CALLBACK& typeSeqBodyCb, const ERROR_CALLBACK& errorCb)
{
    m_recvBuffer.insert(m_recvBuffer.end(), data.begin(), data.end());
    while (m_recvBuffer.size() > 0)
    {
        if (ParseStep::head == m_parseStep) /* 解析包头 */
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
                m_parseStep = ParseStep::type;
                m_head = head;
            }
            else
            {
                return;
            }
        }
        else if (ParseStep::type == m_parseStep) /* 解析类型 */
        {
            if (m_recvBuffer.size() >= 4)
            {
                /* 解析类型(类型占4个字节) */
                unsigned int type = 0;
                if (m_bigEndium) /* 大端存储 */
                {
                    type += m_recvBuffer[0] << 24;
                    type += m_recvBuffer[1] << 16;
                    type += m_recvBuffer[2] << 8;
                    type += m_recvBuffer[3];
                }
                else /* 小端存储 */
                {
                    type += m_recvBuffer[0];
                    type += m_recvBuffer[1] << 8;
                    type += m_recvBuffer[2] << 16;
                    type += m_recvBuffer[3] << 24;
                }
                m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + 4); /* 去除类型 */
                m_parseStep = ParseStep::seq;
                m_type = type;
            }
            else
            {
                return;
            }
        }
        else if (ParseStep::seq == m_parseStep) /* 解析序列ID */
        {
            if (m_recvBuffer.size() >= 8)
            {
                /* 解析序列ID(序列ID占8个字节) */
                uint64_t seq = 0;
                if (m_bigEndium) /* 大端存储 */
                {
                    seq += (uint64_t)m_recvBuffer[0] << 56;
                    seq += (uint64_t)m_recvBuffer[1] << 48;
                    seq += (uint64_t)m_recvBuffer[2] << 40;
                    seq += (uint64_t)m_recvBuffer[3] << 32;
                    seq += (uint64_t)m_recvBuffer[4] << 24;
                    seq += (uint64_t)m_recvBuffer[5] << 16;
                    seq += (uint64_t)m_recvBuffer[6] << 8;
                    seq += (uint64_t)m_recvBuffer[7];
                }
                else /* 小端存储 */
                {
                    seq += (uint64_t)m_recvBuffer[0];
                    seq += (uint64_t)m_recvBuffer[1] << 8;
                    seq += (uint64_t)m_recvBuffer[2] << 16;
                    seq += (uint64_t)m_recvBuffer[3] << 24;
                    seq += (uint64_t)m_recvBuffer[4] << 32;
                    seq += (uint64_t)m_recvBuffer[5] << 40;
                    seq += (uint64_t)m_recvBuffer[6] << 48;
                    seq += (uint64_t)m_recvBuffer[7] << 56;
                }
                m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + 8); /* 去除序列ID */
                if (0 == m_head)
                {
                    if (typeSeqBodyCb)
                    {
                        typeSeqBodyCb(m_type, seq, {});
                    }
                    m_parseStep = ParseStep::head;
                }
                else
                {
                    m_parseStep = ParseStep::body;
                    m_seq = seq;
                }
            }
            else
            {
                return;
            }
        }
        else if (ParseStep::body == m_parseStep) /* 解析包体 */
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
                if (typeSeqBodyCb)
                {
                    typeSeqBodyCb(m_type, m_seq, m_body);
                }
                /* 重置包数据 */
                m_parseStep = ParseStep::head;
                m_head = 0;
                m_type = 0;
                m_seq = 0;
                m_body.clear();
            }
        }
    }
}

void Payload::pack(const unsigned char* body, unsigned int bodyLen, std::vector<unsigned char>& data, bool bigEndium)
{
    data.clear();
    /* 组装包头(包头占4个字节, 用于存放包体长度) */
    if (bigEndium) /* 大端存储 */
    {
        data.emplace_back((bodyLen >> 24) & 0xFF);
        data.emplace_back((bodyLen >> 16) & 0xFF);
        data.emplace_back((bodyLen >> 8) & 0xFF);
        data.emplace_back((bodyLen >> 0) & 0xFF);
    }
    else /* 小端存储 */
    {
        data.emplace_back((bodyLen >> 0) & 0xFF);
        data.emplace_back((bodyLen >> 8) & 0xFF);
        data.emplace_back((bodyLen >> 16) & 0xFF);
        data.emplace_back((bodyLen >> 24) & 0xFF);
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

void Payload::pack(unsigned int type, const unsigned char* body, unsigned int bodyLen, std::vector<unsigned char>& data, bool bigEndium)
{
    data.clear();
    /* 组装包头(包头占4个字节, 用于存放包体长度) */
    if (bigEndium) /* 大端存储 */
    {
        data.emplace_back((bodyLen >> 24) & 0xFF);
        data.emplace_back((bodyLen >> 16) & 0xFF);
        data.emplace_back((bodyLen >> 8) & 0xFF);
        data.emplace_back((bodyLen >> 0) & 0xFF);
    }
    else /* 小端存储 */
    {
        data.emplace_back((bodyLen >> 0) & 0xFF);
        data.emplace_back((bodyLen >> 8) & 0xFF);
        data.emplace_back((bodyLen >> 16) & 0xFF);
        data.emplace_back((bodyLen >> 24) & 0xFF);
    }
    /* 组装类型(类型占4个字节) */
    if (bigEndium) /* 大端存储 */
    {
        data.emplace_back((type >> 24) & 0xFF);
        data.emplace_back((type >> 16) & 0xFF);
        data.emplace_back((type >> 8) & 0xFF);
        data.emplace_back((type >> 0) & 0xFF);
    }
    else /* 小端存储 */
    {
        data.emplace_back((type >> 0) & 0xFF);
        data.emplace_back((type >> 8) & 0xFF);
        data.emplace_back((type >> 16) & 0xFF);
        data.emplace_back((type >> 24) & 0xFF);
    }
    /* 组装包体 */
    if (body && bodyLen > 0)
    {
        data.insert(data.end(), body, body + bodyLen);
    }
}

void Payload::pack(unsigned int type, const std::vector<unsigned char>& body, std::vector<unsigned char>& data, bool bigEndium)
{
    pack(type, body.data(), body.size(), data, bigEndium);
}

void Payload::pack(unsigned int type, uint64_t seq, const unsigned char* body, unsigned int bodyLen, std::vector<unsigned char>& data,
                   bool bigEndium)
{
    data.clear();
    /* 组装包头(包头占4个字节, 用于存放包体长度) */
    if (bigEndium) /* 大端存储 */
    {
        data.emplace_back((bodyLen >> 24) & 0xFF);
        data.emplace_back((bodyLen >> 16) & 0xFF);
        data.emplace_back((bodyLen >> 8) & 0xFF);
        data.emplace_back((bodyLen >> 0) & 0xFF);
    }
    else /* 小端存储 */
    {
        data.emplace_back((bodyLen >> 0) & 0xFF);
        data.emplace_back((bodyLen >> 8) & 0xFF);
        data.emplace_back((bodyLen >> 16) & 0xFF);
        data.emplace_back((bodyLen >> 24) & 0xFF);
    }
    /* 组装类型(类型占4个字节) */
    if (bigEndium) /* 大端存储 */
    {
        data.emplace_back((type >> 24) & 0xFF);
        data.emplace_back((type >> 16) & 0xFF);
        data.emplace_back((type >> 8) & 0xFF);
        data.emplace_back((type >> 0) & 0xFF);
    }
    else /* 小端存储 */
    {
        data.emplace_back((type >> 0) & 0xFF);
        data.emplace_back((type >> 8) & 0xFF);
        data.emplace_back((type >> 16) & 0xFF);
        data.emplace_back((type >> 24) & 0xFF);
    }
    /* 组装序列ID(序列ID占8个字节) */
    if (bigEndium) /* 大端存储 */
    {
        data.emplace_back((seq >> 56) & 0xFF);
        data.emplace_back((seq >> 48) & 0xFF);
        data.emplace_back((seq >> 40) & 0xFF);
        data.emplace_back((seq >> 32) & 0xFF);
        data.emplace_back((seq >> 24) & 0xFF);
        data.emplace_back((seq >> 16) & 0xFF);
        data.emplace_back((seq >> 8) & 0xFF);
        data.emplace_back((seq >> 0) & 0xFF);
    }
    else /* 小端存储 */
    {
        data.emplace_back((seq >> 0) & 0xFF);
        data.emplace_back((seq >> 8) & 0xFF);
        data.emplace_back((seq >> 16) & 0xFF);
        data.emplace_back((seq >> 24) & 0xFF);
        data.emplace_back((seq >> 32) & 0xFF);
        data.emplace_back((seq >> 40) & 0xFF);
        data.emplace_back((seq >> 48) & 0xFF);
        data.emplace_back((seq >> 56) & 0xFF);
    }
    /* 组装包体 */
    if (body && bodyLen > 0)
    {
        data.insert(data.end(), body, body + bodyLen);
    }
}

void Payload::pack(unsigned int type, uint64_t seq, const std::vector<unsigned char>& body, std::vector<unsigned char>& data,
                   bool bigEndium)
{
    pack(type, seq, body.data(), body.size(), data, bigEndium);
}
} // namespace nsocket
