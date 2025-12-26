#include "modbus_tcp.h"

#include <algorithm>

namespace npacket
{
ModbusTcpParser::ModbusTcpParser(const std::vector<uint16_t>& portList)
    : m_modbusPortList(portList.empty() ? std::vector<uint16_t>{502} : portList)
{
}

uint32_t ModbusTcpParser::getProtocol() const
{
    return ApplicationProtocol::MODBUS_TCP;
}

ParseResult ModbusTcpParser::parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                   const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen,
                                   uint32_t& consumeLen)
{
    consumeLen = 0;
    if (!header || TransportProtocol::TCP != header->getProtocol())
    {
        return ParseResult::FAILURE;
    }
    auto tcpHeader = std::dynamic_pointer_cast<TcpHeader>(header);
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
    uint32_t mbapHeaderLen = 0;
    if (!parseMbapHeader(payload, payloadLen, mbap, mbapHeaderLen))
    {
        return ParseResult::FAILURE;
    }
    if (0 != mbap.protocolId) /* 验证协议标识符: 非Modbus协议 */
    {
        return ParseResult::FAILURE;
    }
    /* 定位PDU */
    const uint8_t* pduStart = payload + mbapHeaderLen;
    uint32_t pduLen = payloadLen - mbapHeaderLen;
    if (pduLen < mbap.length || mbap.length < 1) /* 验证PDU长度与MBAP.length一致: 数据不完整或长度字段错误 */
    {
        return ParseResult::FAILURE;
    }
    /* 解析功能码 */
    uint8_t rawFuncCode = pduStart[0];
    bool isException = (0 != (rawFuncCode & (uint8_t)(modbus::FunctionCode::EXCEPTION_MASK)));
    auto funcCode = (modbus::FunctionCode)(rawFuncCode & 0x7F);
    if (!modbus::isValidFunctionCode((uint8_t)(funcCode))) /* 验证功能码有效性 */
    {
        return ParseResult::FAILURE;
    }
    /* 验证最小PDU长度 */
    uint32_t dataLen = mbap.length - 1; /* 减去功能码 */
    if (dataLen < modbus::getMinFrameLength(funcCode, isException))
    {
        return ParseResult::FAILURE;
    }
    /* 判断报文方向: 如果源端口是Modbus端口, 说明是从站响应 */
    bool isRequest = !isModbusPort(tcpHeader->srcPort);
    /* 功能数据处理 */
    if (isException)
    {
        if (dataLen < 1) /* 异常响应必须包含异常码 */
        {
            return ParseResult::FAILURE;
        }
    }
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
        d->rawDataLen = modbus::MBAP_HEADER_LEN + mbap.length;
        d->funcDataOffset = mbapHeaderLen + 1;
        d->funcDataLen = dataLen;
    }
    if (m_dataCallback)
    {
        m_dataCallback(ntp, totalLen, header, d);
    }
    consumeLen = modbus::MBAP_HEADER_LEN + mbap.length;
    return ParseResult::SUCCESS;
}

void ModbusTcpParser::setDataCallback(const DATA_CALLBACK& callback)
{
    m_dataCallback = callback;
}

bool ModbusTcpParser::parseMbapHeader(const uint8_t* data, uint32_t dataLen, MbapHeader& mbap, uint32_t& headerLen)
{
    if (dataLen < modbus::MBAP_HEADER_LEN)
    {
        return false;
    }
    /* 大端字节序转换 */
    mbap.transactionId = (data[0] << 8) | data[1];
    mbap.protocolId = (data[2] << 8) | data[3];
    mbap.length = (data[4] << 8) | data[5];
    mbap.unitId = data[6];
    headerLen = modbus::MBAP_HEADER_LEN;
    if (mbap.length < 1) /* PDU长度至少包含功能码(1字节) */
    {
        return false;
    }
    if (mbap.length > modbus::MAX_PDU_LENGTH) /* PDU长度不超过Modbus协议最大值 */
    {
        return false;
    }
    if (mbap.length > dataLen - modbus::MBAP_HEADER_LEN) /* 数据包被分片或不完整 */
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
