#include "modbus_rtu.h"

#include <algorithm>

namespace npacket
{
ModbusRtuParser::ModbusRtuParser(const std::vector<uint8_t> addressList, bool generateTransId)
    : m_allowedAddressList(addressList.begin(), addressList.end()), m_generateTransId(generateTransId)
{
}

uint32_t ModbusRtuParser::getProtocol() const
{
    return ApplicationProtocol::MODBUS_RTU;
}

bool ModbusRtuParser::parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                            const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
{
    if (!payload || payloadLen < modbus::RTU_MIN_FRAME)
    {
        return false;
    }
    /* 解析头部字段 */
    uint8_t slaveAddress = payload[0];
    uint8_t rawFuncCode = payload[1];
    if (!isValidSlaveAddress(slaveAddress)) /* 验证从地址 */
    {
        return false;
    }
    bool isException = (0 != (rawFuncCode & (uint8_t)(modbus::FunctionCode::EXCEPTION_MASK))); /* 是否为异常响应 */
    auto funcCode = (modbus::FunctionCode)(rawFuncCode & 0x7F);
    if (!modbus::isValidFunctionCode((uint8_t)(funcCode))) /* 验证功能码有效性 */
    {
        return false;
    }
    /* CRC校验 */
    uint16_t crcReceived = (payload[payloadLen - 2] | (payload[payloadLen - 1] << 8));
    uint16_t crcCalculated = calcCRC16(payload, payloadLen - 2);
    if (crcReceived != crcCalculated) /* CRC不匹配 */
    {
        return false;
    }
    /* 数据定位 */
    const uint8_t* data = payload + 2;
    uint32_t dataLen = payloadLen - modbus::RTU_MIN_FRAME; /* 去掉头部和CRC */
    modbus::ExceptionCode exceptionCode = modbus::ExceptionCode::ILLEGAL_FUNCTION;
    if (isException)
    {
        if (dataLen < 1) /* 异常响应验证 */
        {
            return false; /* 异常响应必须包含异常码 */
        }
        exceptionCode = (modbus::ExceptionCode)(data[0]);
        dataLen = 0; /* 异常帧无后续数据 */
    }
    else /* 正常请求/响应, 验证最小数据长度 */
    {
        uint32_t minDataLen = modbus::getMinFrameLength(funcCode);
        if (dataLen < minDataLen)
        {
            return false;
        }
    }
    if (m_dataCallback)
    {
        modbus::DataSt d;
        d.transactionId = m_generateTransId ? generateTransactionId() : 0;
        d.slaveAddress = slaveAddress;
        d.isBroadcast = (0 == slaveAddress);
        d.funcCode = funcCode;
        if (isException)
        {
            d.data = nullptr;
            d.dataLen = 0;
        }
        else
        {
            d.data = data;
            d.dataLen = dataLen;
        }
        d.isException = isException;
        d.exceptionCode = exceptionCode;
        m_dataCallback(ntp, totalLen, d);
    }
    return true;
}

void ModbusRtuParser::setDataCallback(const DATA_CALLBACK& callback)
{
    m_dataCallback = callback;
}

uint16_t ModbusRtuParser::calcCRC16(const uint8_t* data, uint32_t len) const
{
    uint16_t crc = 0xFFFF;
    for (uint32_t i = 0; i < len; ++i)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool ModbusRtuParser::isValidSlaveAddress(uint8_t address) const
{
    if (m_allowedAddressList.empty()) /* 未设置白名单, 允许所有 */
    {
        return true;
    }
    return (m_allowedAddressList.end() != m_allowedAddressList.find(address));
}

uint16_t ModbusRtuParser::generateTransactionId()
{
    return m_transactionId.fetch_add(1, std::memory_order_relaxed);
}
} // namespace npacket
