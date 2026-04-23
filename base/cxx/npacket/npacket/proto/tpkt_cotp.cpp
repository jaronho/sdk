#include "tpkt_cotp.h"

namespace npacket
{
uint32_t TpktCotpParser::getProtocol() const noexcept
{
    return ApplicationProtocol::TPKT_COTP;
}

ParseResult TpktCotpParser::parse(size_t flag, size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                  const ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen)
{
    consumeLen = 0;
    if (header && TransportProtocol::TCP != header->getProtocol())
    {
        return ParseResult::FAILURE;
    }
    if (payloadLen < 7) /* 最小长度检查: TPKT(4) + COTP最小(3) */
    {
        return ParseResult::FAILURE;
    }
    const uint8_t* buffer = payload;
    uint32_t bufferLen = payloadLen;
    TpktInfo tpktInfo;
    uint32_t tpktLen = 0;
    /* 解析TPKT */
    if (!parseTpktInfo(buffer, bufferLen, tpktInfo, tpktLen))
    {
        return ParseResult::FAILURE;
    }
    /* TPKT长度应等于整个payload长度(单帧场景) */
    if (tpktInfo.length != payloadLen)
    {
        /* 可能是多TPKT包合并, 按tpktInfo.length处理当前包, 剩余数据留给下次解析 */
    }
    buffer += tpktLen;
    bufferLen -= tpktLen;
    /* 解析COTP */
    CotpInfo cotpInfo;
    uint32_t cotpLen = 0;
    if (!parseCotpInfo(buffer, bufferLen, cotpInfo, cotpLen))
    {
        return ParseResult::FAILURE;
    }
    buffer += cotpLen;
    bufferLen -= cotpLen;
    consumeLen = tpktInfo.length;
    /* 回调通知 */
    if (m_dataCallback)
    {
        m_dataCallback(flag, num, ntp, totalLen, header, tpktInfo, cotpInfo, buffer, bufferLen);
    }
    return ParseResult::SUCCESS;
}

void TpktCotpParser::setDataCallback(const DATA_CALLBACK& callback)
{
    m_dataCallback = callback;
}

bool TpktCotpParser::parseTpktInfo(const uint8_t* data, uint32_t dataLen, TpktInfo& info, uint32_t& tpktLen)
{
    memset(&info, 0, sizeof(TpktInfo));
    tpktLen = 0;
    if (dataLen < 4) /* TPKT(4个字节) */
    {
        return false;
    }
    uint8_t version = data[0];
    uint8_t reserved = data[1];
    uint16_t length = (data[2] << 8) | data[3];
    if (3 != version) /* 版本校验: RFC1006规定版本为3 */
    {
        return false;
    }
    if (length < 4) /* 长度校验: 至少包含TPKT头本身 */
    {
        return false;
    }
    info.version = version;
    info.reserved = reserved;
    info.length = length;
    tpktLen = 4;
    return true;
}

bool TpktCotpParser::parseCotpInfo(const uint8_t* data, uint32_t dataLen, CotpInfo& info, uint32_t& cotpLen)
{
    memset(&info, 0, sizeof(CotpInfo));
    info.type = CotpInfo::UNKNOWN;
    cotpLen = 0;
    if (dataLen < 3) /* COTP(3个字节或18个字节) */
    {
        return false;
    }
    uint8_t length = data[0];
    if (2 != length && 17 != length) /* 长度校验 */
    {
        return false;
    }
    uint8_t pduType = ((data[1] & 0xF0) >> 4) | ((data[1] & 0x0F) << 4); /* PDU类型在字节1的高4位和低4位 */
    switch ((CotpPduType)pduType)
    {
    case CotpPduType::ED:
    case CotpPduType::EA:
    case CotpPduType::UD:
    case CotpPduType::RJ:
    case CotpPduType::AK:
    case CotpPduType::ER:
    case CotpPduType::DR:
    case CotpPduType::DC:
    case CotpPduType::CC:
    case CotpPduType::CR:
    case CotpPduType::DT:
        break;
    default:
        return false;
    }
    if (2 == length && dataLen >= 3) /* 短头: 3字节(用于DT数据传输) */
    {
        info.type = CotpInfo::SHORT;
        info.sinfo.length = length;
        info.sinfo.pduType = pduType;
        info.sinfo.tpduNumber = (data[2] & 0x7F);
        info.sinfo.lastDataUnit = (data[2] & 0x80) >> 7;
        cotpLen = 3;
    }
    else if (17 == length && dataLen >= 18) /* 长头: 18字节(用于CR/CC连接建立) */
    {
        info.type = CotpInfo::LONG;
        info.linfo.length = length;
        info.linfo.pduType = pduType;
        info.linfo.dstRef = (data[2] << 8) | data[3];
        info.linfo.srcRef = (data[4] << 8) | data[5];
        info.linfo.nClass = data[6] >> 4;
        info.linfo.extendedFormats = (data[6] & 0x02) >> 1;
        info.linfo.noExplicitFlowControl = (data[6] & 0x01);
        info.linfo.parameterCode = data[7];
        info.linfo.parameterLength = data[8];
        memcpy(info.linfo.reserved, &data[9], sizeof(info.linfo.reserved)); /* 剩余[9]-[17]字节内容 */
        cotpLen = 18;
    }
    else
    {
        return false;
    }
    return true;
}
} // namespace npacket
