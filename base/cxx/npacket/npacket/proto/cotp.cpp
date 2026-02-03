#include "cotp.h"

namespace npacket
{
uint32_t CotpParser::getProtocol() const noexcept
{
    return ApplicationProtocol::COTP;
}

ParseResult CotpParser::parse(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                              const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen)
{
    consumeLen = 0;
    if (header && TransportProtocol::TCP != header->getProtocol())
    {
        return ParseResult::FAILURE;
    }
    if (payloadLen < 7) /* COTP包最小7个字节 */
    {
        return ParseResult::FAILURE;
    }
    const uint8_t* buffer = payload;
    uint32_t bufferLen = payloadLen;
    /* 尝试解析TPKT */
    TpktInfo tpktInfo;
    tpktInfo.version = buffer[0];
    tpktInfo.reserverd = buffer[1];
    tpktInfo.length = (buffer[2] << 8) + buffer[3];
    if (tpktInfo.length != payloadLen)
    {
        return ParseResult::FAILURE;
    }
    buffer = payload + 4;
    bufferLen -= 4;
    /* 尝试解析COTP */
    uint8_t cotpLength = buffer[0];
    if (cotpLength < 2 || 5 + cotpLength < payloadLen)
    {
        return ParseResult::FAILURE;
    }
    PduType pduType = (PduType)((buffer[1] << 4) | (buffer[1] >> 4));
    switch (pduType)
    {
    case PduType::ED:
    case PduType::EA:
    case PduType::UD:
    case PduType::RJ:
    case PduType::AK:
    case PduType::ER:
    case PduType::DR:
    case PduType::DC:
    case PduType::CC:
    case PduType::CR:
    case PduType::DT:
        break;
    default: /* PDU类型标识错误 */
        return ParseResult::FAILURE;
    }
    std::shared_ptr<CotpInfo> cotpInfo = nullptr;
    if (2 == cotpLength)
    {
        auto cotpInfo3 = std::make_shared<CotpInfo3>();
        cotpInfo3->length = cotpLength;
        cotpInfo3->pduType = pduType;
        cotpInfo3->tpduNumber = (buffer[2] & 0x7F);
        cotpInfo3->lastDataUnit = (buffer[2] & 0x80) >> 7;
        cotpInfo = cotpInfo3;
    }
    else if (17 == cotpLength)
    {
        auto cotpInfo18 = std::make_shared<CotpInfo18>();
        cotpInfo18->length = cotpLength;
        cotpInfo18->pduType = pduType;
        cotpInfo18->dstRef = (buffer[2] << 8) | buffer[3];
        cotpInfo18->srcRef = (buffer[4] << 8) | buffer[5];
        cotpInfo18->nClass = buffer[6] >> 4;
        cotpInfo18->extendedFormats = (buffer[6] & 0x2) >> 1;
        cotpInfo18->noExplicitFlowControl = (buffer[6] & 0x1);
        cotpInfo18->parameterCode = buffer[7];
        cotpInfo18->parameterLength = buffer[8];
        cotpInfo = cotpInfo18;
    }
    else
    {
        return ParseResult::FAILURE;
    }
    buffer = buffer + 1 + cotpInfo->length;
    bufferLen -= (1 + cotpInfo->length);
    /* 解析COTP负载 */
    if (bufferLen > 0)
    {
        if (0x32 == buffer[0]) /* S7COMM */
        {
            parseS7Comm(ntp, totalLen, header, tpktInfo, cotpInfo, buffer, bufferLen);
        }
    }
    consumeLen = tpktInfo.length;
    return ParseResult::SUCCESS;
}

bool CotpParser::parseS7Comm(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                             const TpktInfo& tpktInfo, const std::shared_ptr<CotpInfo>& cotpInfo, const uint8_t* payload,
                             uint32_t payloadLen)
{
    const uint8_t* buffer = payload;
    uint32_t bufferLen = payloadLen;
    S7CommInfo s7Info;
    /* 解析头部 */
    s7Info.header.protocolId = buffer[0];
    s7Info.header.rosctr = (RosctrType)buffer[1];
    s7Info.header.redundancyIdentification = (buffer[2] << 8) | buffer[3];
    s7Info.header.protocolDataUnitReference = (buffer[4] << 8) | buffer[5];
    s7Info.header.parameterLength = (buffer[6] << 8) | buffer[7];
    s7Info.header.dataLength = (buffer[8] << 8) | buffer[9];
    if (RosctrType::ACK_DATA == s7Info.header.rosctr)
    {
        s7Info.header.errorClass = buffer[10];
        s7Info.header.errorCode = buffer[11];
        buffer = buffer + 12;
        bufferLen -= 12;
    }
    else
    {
        buffer = buffer + 10;
        bufferLen -= 10;
    }
    if (bufferLen != s7Info.header.parameterLength + s7Info.header.dataLength)
    {
        return false;
    }
    /* TODO: 解析参数 */
    /* TODO: 解析数据 */
    return true;
}
} // namespace npacket
