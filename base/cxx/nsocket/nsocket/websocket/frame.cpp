#include "frame.h"

#include <random>
#include <string.h>

namespace nsocket
{
namespace ws
{
/**
 * @brief 创建掩码值
 * @param maskingKey [输出]掩码值
 */
static void createMaskingKey(unsigned char maskingKey[4])
{
    std::uniform_int_distribution<unsigned short> dist(0, 255);
    std::random_device rd;
    for (int i = 0; i < 4; ++i)
    {
        maskingKey[i] = (unsigned char)dist(rd);
    }
}

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
        case ParseStep::fin_rsv_opcode:
            if ((used = parseFinRsvOpcode(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::mask_payload_len:
            if ((used = parseMaskPayloadLen(remainData, remainLen, headCb, finishCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::payload_len_2:
            if ((used = parsePayloadLen(remainData, remainLen, 2, headCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::payload_len_8:
            if ((used = parsePayloadLen(remainData, remainLen, 8, headCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::masking_key:
            if ((used = parseMaskingKey(remainData, remainLen, headCb, finishCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::payload:
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

void Frame::createTextFrame(std::vector<unsigned char>& data, const std::string& text, bool isClient, bool isFin)
{
    Frame f;
    f.fin = isFin ? 1 : 0;
    f.opcode = 0x1;
    if (isClient)
    {
        f.mask = 1;
        createMaskingKey(f.maskingKey);
    }
    else
    {
        f.mask = 0;
    }
    f.payloadLen = text.size();
    f.create(data);
    if (isClient)
    {
        for (size_t i = 0; i < text.size(); ++i)
        {
            data.push_back(text[i] ^ f.maskingKey[i % 4]);
        }
    }
    else
    {
        data.insert(data.end(), text.begin(), text.end());
    }
}

void Frame::createBinaryFrame(std::vector<unsigned char>& data, const std::vector<unsigned char>& bytes, bool isClient, bool isFin)
{
    Frame f;
    f.fin = isFin ? 1 : 0;
    f.opcode = 0x2;
    if (isClient)
    {
        f.mask = 1;
        createMaskingKey(f.maskingKey);
    }
    else
    {
        f.mask = 0;
    }
    f.payloadLen = bytes.size();
    f.create(data);
    if (isClient)
    {
        for (size_t i = 0; i < bytes.size(); ++i)
        {
            data.push_back(bytes[i] ^ f.maskingKey[i % 4]);
        }
    }
    else
    {
        data.insert(data.end(), bytes.begin(), bytes.end());
    }
}

void Frame::createCloseFrame(std::vector<unsigned char>& data, const CloseCode& code, bool isClient)
{
    Frame f;
    f.fin = 1;
    f.opcode = 0x8;
    if (isClient)
    {
        f.mask = 1;
        createMaskingKey(f.maskingKey);
    }
    else
    {
        f.mask = 0;
    }
    f.payloadLen = 2;
    f.create(data);
    /* 负载写入状态码 */
    if (isClient)
    {
        data.push_back(((int)code >> 8) ^ f.maskingKey[0]);
        data.push_back(((int)code % 256) ^ f.maskingKey[1]);
    }
    else
    {
        data.push_back((int)code >> 8);
        data.push_back((int)code % 256);
    }
}

void Frame::createPingFrame(std::vector<unsigned char>& data, bool isClient)
{
    Frame f;
    f.fin = 1;
    f.opcode = 0x9;
    if (isClient)
    {
        f.mask = 1;
        createMaskingKey(f.maskingKey);
    }
    else
    {
        f.mask = 0;
    }
    f.create(data);
}

void Frame::createPongFrame(std::vector<unsigned char>& data, bool isClient)
{
    Frame f;
    f.fin = 1;
    f.opcode = 0xA;
    if (isClient)
    {
        f.mask = 1;
        createMaskingKey(f.maskingKey);
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
        /* 掩码(4个字节): masking_key */
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
    m_parseStep = ParseStep::fin_rsv_opcode;
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
    m_parseStep = ParseStep::mask_payload_len;
    return 1;
}

int Frame::parseMaskPayloadLen(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const FINISH_CALLBACK& finishCb)
{
    unsigned char byte = data[0]; /* 只解析1个字节 */
    mask = byte >> 7;
    unsigned char len = byte & 0x7F;
    if (len <= 125) /* 立即得到负载长度 */
    {
        payloadLen = len;
        if (0 == mask)
        {
            m_parseStep = ParseStep::payload;
            if (headCb)
            {
                headCb();
            }
        }
        else
        {
            m_parseStep = ParseStep::masking_key;
        }
    }
    else if (126 == len) /* 需要解析后面的2个字节作为负载长度 */
    {
        payloadLen = 0;
        m_parseStep = ParseStep::payload_len_2;
    }
    else if (127 == len) /* 需要解析后面的8个字节作为负载长度 */
    {
        payloadLen = 0;
        m_parseStep = ParseStep::payload_len_8;
    }
    else /* 协议错误: 数据量太大 */
    {
        return 0;
    }
    m_payloadReceived = 0;
    m_tmpBytes.clear();
    if (0 == mask && ParseStep::payload == m_parseStep && 0 == payloadLen)
    {
        if (finishCb)
        {
            finishCb();
        }
        reset();
    }
    return 1;
}

int Frame::parsePayloadLen(const unsigned char* data, int length, int needByteCount, const HEAD_CALLBACK& headCb)
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
            if (0 == mask)
            {
                m_parseStep = ParseStep::payload;
                if (headCb)
                {
                    headCb();
                }
            }
            else
            {
                m_parseStep = ParseStep::masking_key;
            }
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
            m_parseStep = ParseStep::payload;
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
