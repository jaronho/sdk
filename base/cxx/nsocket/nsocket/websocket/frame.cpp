#include "frame.h"

namespace nsocket
{
namespace ws
{
int Frame::parse(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const PAYLOAD_CALLBACK& payloadCb,
                 const FINISH_CALLBACK& finishCb)
{
    if (!data || length <= 0)
    {
        return 0;
    }
    int totalUsed = 0;
    while (totalUsed < length)
    {
        const unsigned char* remainData = data + totalUsed;
        int remainLen = length - totalUsed;
        int used = 0;
        switch (m_parseStep)
        {
        case ParseStep::FIN_RSV_OPCODE:
            if ((used = parseFinRsvOpcode(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::MASK_PAYLOAD_LEN:
            if ((used = parseMaskPayloadLen(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::PAYLOAD_LEN_2:
            if ((used = parsePayloadLen(remainData, remainLen, 2)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::PAYLOAD_LEN_8:
            if ((used = parsePayloadLen(remainData, remainLen, 8)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::MASKING_KEY:
            if ((used = parseMaskingKey(remainData, remainLen, headCb, finishCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::PAYLOAD:
            if ((used = parsePayload(remainData, remainLen, payloadCb, finishCb)) <= 0)
            {
                return 0;
            }
            break;
        }
        totalUsed += used;
    }
    return totalUsed;
}

void Frame::createTextFrame(std::vector<unsigned char>& data, const std::string& text, unsigned char maskingKey[4], bool isFin)
{
    Frame f;
    f.fin = isFin ? 1 : 0;
    f.opcode = 0x1;
    if (maskingKey)
    {
        f.mask = 1;
        for (int i = 0; i < 4; ++i)
        {
            f.maskingKey[i] = maskingKey[i];
        }
    }
    else
    {
        f.mask = 0;
    }
    f.payloadLen = text.size();
    f.create(data);
    data.insert(data.end(), text.begin(), text.end());
}

void Frame::createBinaryFrame(std::vector<unsigned char>& data, const std::vector<unsigned char>& bytes, unsigned char maskingKey[4],
                              bool isFin)
{
    Frame f;
    f.fin = isFin ? 1 : 0;
    f.opcode = 0x2;
    if (maskingKey)
    {
        f.mask = 1;
        for (int i = 0; i < 4; ++i)
        {
            f.maskingKey[i] = maskingKey[i];
        }
    }
    else
    {
        f.mask = 0;
    }
    f.payloadLen = bytes.size();
    f.create(data);
    data.insert(data.end(), bytes.begin(), bytes.end());
}

void Frame::createCloseFrame(std::vector<unsigned char>& data, const CloseCode& code, unsigned char maskingKey[4])
{
    Frame f;
    f.fin = 1;
    f.opcode = 0x8;
    if (maskingKey)
    {
        f.mask = 1;
        for (int i = 0; i < 4; ++i)
        {
            f.maskingKey[i] = maskingKey[i];
        }
    }
    else
    {
        f.mask = 0;
    }
    f.payloadLen = 2;
    f.create(data);
    /* 负载写入状态码 */
    for (int i = 2 - 1; i >= 0; --i)
    {
        unsigned char ch = ((int)code >> (8 * i)) % 256;
        data.push_back(ch);
    }
}

void Frame::createPingFrame(std::vector<unsigned char>& data, unsigned char maskingKey[4])
{
    Frame f;
    f.fin = 1;
    f.opcode = 0x9;
    if (maskingKey)
    {
        f.mask = 1;
        for (int i = 0; i < 4; ++i)
        {
            f.maskingKey[i] = maskingKey[i];
        }
    }
    else
    {
        f.mask = 0;
    }
    f.create(data);
}

void Frame::createPongFrame(std::vector<unsigned char>& data, unsigned char maskingKey[4])
{
    Frame f;
    f.fin = 1;
    f.opcode = 0xA;
    if (maskingKey)
    {
        f.mask = 1;
        for (int i = 0; i < 4; ++i)
        {
            f.maskingKey[i] = maskingKey[i];
        }
    }
    else
    {
        f.mask = 0;
    }
    f.create(data);
}

void Frame::create(std::vector<unsigned char>& data)
{
    data.clear();
    /* 第1个字节: FIN + RSV + OPCODE */
    unsigned char byte1 = 0;
    byte1 += fin << 7;
    byte1 += rsv[0] << 6;
    byte1 += rsv[1] << 5;
    byte1 += rsv[2] << 4;
    byte1 += opcode;
    data.push_back(byte1);
    /* 第2个字节: MASK + PAYLOAD_LEN */
    unsigned char byte2 = 0;
    byte2 += mask << 7;
    int extByteCount = 0;
    if (payloadLen <= 125) /* 本字节足以存放负载长度 */
    {
        byte2 += payloadLen;
    }
    else if (payloadLen <= 0xFFFF) /* 需要扩展2个字节用于存放负载长度 */
    {
        byte2 += 126;
        extByteCount = 2;
    }
    else /* 需要扩展8个字节用于存放负载长度 */
    {
        byte2 += 127;
        extByteCount = 8;
    }
    data.push_back(byte2);
    /* 扩展负载长度(大端字节序) */
    for (int i = extByteCount - 1; i >= 0; --i)
    {
        unsigned char ch = ((unsigned long long)payloadLen >> (8 * i)) % 256;
        data.push_back(ch);
    }
    if (1 == mask) /* 有掩码时才需要写入 */
    {
        /* 掩码(4个字节): MASKING_KEY */
        for (int i = 0; i < 4; ++i)
        {
            data.push_back(maskingKey[i]);
        }
    }
}

void Frame::reset()
{
    fin = 0;
    memset(rsv, 0, sizeof(rsv));
    opcode = 0;
    mask = 0;
    payloadLen = 0;
    memset(maskingKey, 0, sizeof(maskingKey));
    m_parseStep = ParseStep::FIN_RSV_OPCODE;
    m_payloadReceived = 0;
    m_tmpBytes.clear();
}

int Frame::parseFinRsvOpcode(const unsigned char* data, int length)
{
    unsigned char byte = data[0]; /* 只解析1个字节 */
    fin = byte >> 7;
    rsv[0] = (byte >> 6) & 0x1;
    rsv[1] = (byte >> 5) & 0x1;
    rsv[2] = (byte >> 4) & 0x1;
    opcode = byte & 0xF;
    m_parseStep = ParseStep::MASK_PAYLOAD_LEN;
    return 1;
}

int Frame::parseMaskPayloadLen(const unsigned char* data, int length)
{
    unsigned char byte = data[0]; /* 只解析1个字节 */
    mask = byte >> 7;
    unsigned char len = byte & 0x7F;
    if (len <= 125) /* 立即得到负载长度 */
    {
        payloadLen = len;
        m_parseStep = ParseStep::MASKING_KEY;
    }
    else if (126 == len) /* 需要解析后面的2个字节作为负载长度 */
    {
        payloadLen = 0;
        m_parseStep = ParseStep::PAYLOAD_LEN_2;
    }
    else if (127 == len) /* 需要解析后面的8个字节作为负载长度 */
    {
        payloadLen = 0;
        m_parseStep = ParseStep::PAYLOAD_LEN_8;
    }
    else /* 协议错误 */
    {
        return 0;
    }
    m_payloadReceived = 0;
    m_tmpBytes.clear();
    return 1;
}

int Frame::parsePayloadLen(const unsigned char* data, int length, int needByteCount)
{
    int used = 0;
    for (; used < length; ++used)
    {
        m_tmpBytes.push_back(data[used]);
        if (needByteCount == m_tmpBytes.size()) /* 字节数已到 */
        {
            for (size_t i = 0; i < m_tmpBytes.size(); ++i) /* 计算负载长度 */
            {
                payloadLen += m_tmpBytes[i] << (8 * (needByteCount - 1 - (int)i)); /* 大端字节序 */
            }
            m_tmpBytes.clear();
            m_parseStep = ParseStep::MASKING_KEY;
            return (used + 1);
        }
    }
    return used;
}

int Frame::parseMaskingKey(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const FINISH_CALLBACK& finishCb)
{
    int used = 0;
    for (; used < length; ++used)
    {
        m_tmpBytes.push_back(data[used]);
        if (4 == m_tmpBytes.size()) /* 只解析4个字节 */
        {
            for (size_t i = 0; i < m_tmpBytes.size(); ++i)
            {
                maskingKey[i] = m_tmpBytes[i];
            }
            m_tmpBytes.clear();
            m_parseStep = ParseStep::PAYLOAD;
            if (headCb)
            {
                headCb();
            }
            if (0 == payloadLen)
            {
                if (finishCb)
                {
                    finishCb();
                }
                reset();
            }
            return (used + 1);
        }
    }
    return used;
}

int Frame::parsePayload(const unsigned char* data, int length, const PAYLOAD_CALLBACK& payloadCb, const FINISH_CALLBACK& finishCb)
{
    int used = length;
    if (m_payloadReceived + used > payloadLen)
    {
        used = payloadLen - m_payloadReceived;
    }
    if (payloadCb)
    {
        if (0 == mask)
        {
            payloadCb(m_payloadReceived, data, used);
        }
        else
        {
            std::vector<unsigned char> payload;
            for (int i = 0; i < used; ++i)
            {
                unsigned char ch = data[i] ^ maskingKey[(m_payloadReceived + i) % 4]; /* 对负载数据需要使用掩码进行异或解码出正确数据 */
                payload.push_back(ch);
            }
            payloadCb(m_payloadReceived, payload.data(), used);
        }
    }
    m_payloadReceived += used;
    if (m_payloadReceived == payloadLen) /* 数据都已接收完毕, 或已经是最后一个帧数据 */
    {
        if (finishCb)
        {
            finishCb();
        }
        reset();
    }
    return used;
}
} // namespace ws
} // namespace nsocket
