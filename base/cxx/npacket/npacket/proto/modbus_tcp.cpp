#include "modbus_tcp.h"

#include <algorithm>

namespace npacket
{
ModbusTcpParser::ModbusTcpParser(const std::vector<uint16_t>& portList)
    : m_modbusPortList(portList.empty() ? std::vector<uint16_t>{502} : portList)
{
}

uint32_t ModbusTcpParser::getProtocol() const noexcept
{
    return ApplicationProtocol::MODBUS_TCP;
}

ParseResult ModbusTcpParser::parse(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                   const ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen)
{
    consumeLen = 0;
    if (!header || TransportProtocol::TCP != header->getProtocol())
    {
        return ParseResult::FAILURE;
    }
    auto tcpHeader = (const TcpHeader*)(header);
    if (!tcpHeader)
    {
        return ParseResult::FAILURE;
    }
    if (!isModbusPort(tcpHeader->srcPort) && !isModbusPort(tcpHeader->dstPort)) /* 验证端口 */
    {
        return ParseResult::FAILURE;
    }
    /* 解析MBAP头 */
    MbapHeader mbap;
    uint32_t mbapHeaderLen = 0, pduLen = 0;
    if (!parseMbapHeader(payload, payloadLen, mbap, mbapHeaderLen, pduLen))
    {
        if (payloadLen < modbus::MBAP_HEADER_LEN) /* 长度不足(可能被分包) */
        {
            return ParseResult::CONTINUE;
        }
        return ParseResult::FAILURE;
    }
    if (0 != mbap.protocolId) /* 验证协议标识符: 非Modbus协议 */
    {
        return ParseResult::FAILURE;
    }
    /* 检查是否收齐完整的PDU数据 */
    uint32_t fullFrameLen = mbapHeaderLen + pduLen;
    if (payloadLen < fullFrameLen) /* PDU数据未收齐(被TCP分片) */
    {
        return ParseResult::CONTINUE;
    }
    /* 定位PDU */
    const uint8_t* pduStart = payload + mbapHeaderLen;
    /* 解析功能码 */
    uint8_t rawFuncCode = pduStart[0];
    bool isException = (0 != (rawFuncCode & (uint8_t)(modbus::FunctionCode::EXCEPTION_MASK)));
    auto funcCode = (modbus::FunctionCode)(rawFuncCode & 0x7F);
    if (!modbus::isValidFunctionCode((uint8_t)(funcCode))) /* 验证功能码有效性 */
    {
        return ParseResult::FAILURE;
    }
    /* 验证最小PDU长度 */
    uint32_t minFrameLen = modbus::getMinFrameLength(funcCode, isException);
    if (pduLen < minFrameLen)
    {
        return ParseResult::FAILURE;
    }
    uint32_t funcDataLen = pduLen - 1; /* 减去1个字节功能码 */
    /* 功能数据处理 */
    if (isException)
    {
        if (0 == funcDataLen) /* 异常响应必须包含异常码 */
        {
            return ParseResult::FAILURE;
        }
    }
    /* 判断报文方向: 如果源端口是Modbus端口, 说明是从站响应 */
    bool isRequest = !isModbusPort(tcpHeader->srcPort);
    auto d = std::make_shared<modbus::DataSt>();
    d->transactionId = mbap.transactionId;
    d->slaveAddress = mbap.unitId;
    d->isBroadcast = false; /* TCP无广播概念 */
    d->funcCode = funcCode;
    d->isRequest = isRequest;
    d->isException = isException;
    if (isException)
    {
        d->rawData = nullptr;
        d->rawDataLen = 0;
        d->funcDataOffset = 0;
        d->funcDataLen = 0;
    }
    else
    {
        d->rawData = payload;
        d->rawDataLen = fullFrameLen;
        d->funcDataOffset = mbapHeaderLen + 1;
        d->funcDataLen = funcDataLen;
    }
    if (m_dataCallback)
    {
        m_dataCallback(ntp, totalLen, header, d);
    }
    consumeLen = fullFrameLen;
    return ParseResult::SUCCESS;
}

void ModbusTcpParser::setDataCallback(const DATA_CALLBACK& callback)
{
    m_dataCallback = callback;
}

bool ModbusTcpParser::parseMbapHeader(const uint8_t* data, uint32_t dataLen, MbapHeader& mbap, uint32_t& headerLen, uint32_t& pduLen)
{
    if (dataLen < modbus::MBAP_HEADER_LEN)
    {
        return false;
    }
    /* 大端字节序转换 */
    mbap.transactionId = (data[0] << 8) | data[1];
    mbap.protocolId = (data[2] << 8) | data[3];
    mbap.length = (data[4] << 8) | data[5]; /* unitId(1字节) + PDU长度 */
    mbap.unitId = data[6];
    headerLen = modbus::MBAP_HEADER_LEN;
    pduLen = mbap.length - 1;
    if (pduLen < 1) /* PDU长度至少包含功能码(1字节) */
    {
        return false;
    }
    if (pduLen > modbus::MAX_PDU_LENGTH) /* PDU长度不超过Modbus协议最大值 */
    {
        return false;
    }
    if (pduLen > dataLen - headerLen) /* 数据包被分片或不完整 */
    {
        return false;
    }
    return true;
}

bool ModbusTcpParser::isModbusPort(uint16_t port) const
{
    return (m_modbusPortList.end() != std::find(m_modbusPortList.begin(), m_modbusPortList.end(), port));
}
} // namespace npacket
