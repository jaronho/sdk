#include "modbus_rtu.h"

#include <algorithm>

namespace npacket
{
ModbusRtuParser::ModbusRtuParser(const std::vector<uint8_t>& addressList) : m_allowedAddressList(addressList) {}

uint32_t ModbusRtuParser::getProtocol() const
{
    return ApplicationProtocol::MODBUS_RTU;
}

bool ModbusRtuParser::parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                            const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
{
    if (!payload || payloadLen < 4) /* Modbus/RTU最小帧长: 地址(1) + 功能码(1) + CRC(2) = 4字节 */
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
    if ((uint8_t)(funcCode) > 0x2B) /* 验证功能码有效性(超过最大标准功能码) */
    {
        return false;
    }
    if (!validateFrameLength(funcCode, payloadLen)) /* 验证帧长度与功能码匹配 */
    {
        return false;
    }
    /* CRC校验 */
    uint16_t crcReceived = (payload[payloadLen - 1] << 8) | payload[payloadLen - 2];
    uint16_t crcCalculated = calcCRC16(payload, payloadLen - 2);
    if (crcReceived != crcCalculated) /* CRC不匹配 */
    {
        return false;
    }
    /* 数据定位 */
    const uint8_t* data = payload + 2;
    uint32_t dataLen = payloadLen - 4; /* 去掉头部和CRC */
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
    /* 广播地址特殊处理(地址0不应有响应) */
    if (0 == slaveAddress && !isException) /* 广播请求不应出现在接收路径(除非是伪造) */
    {
        return false;
    }
    if (m_dataCallback)
    {
        m_dataCallback(ntp, totalLen, slaveAddress, funcCode, data, dataLen, isException, exceptionCode);
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
    return (crc >> 8) | (crc << 8); /* 字节序转换 */
}

bool ModbusRtuParser::isValidSlaveAddress(uint8_t address) const
{
    if (m_allowedAddressList.empty()) /* 未设置白名单, 允许所有 */
    {
        return true;
    }
    return (m_allowedAddressList.end() != std::find(m_allowedAddressList.begin(), m_allowedAddressList.end(), address));
}

bool ModbusRtuParser::validateFrameLength(modbus::FunctionCode code, uint32_t len) const
{
    if (len < 4) /* 基础长度：地址(1) + 功能码(1) + CRC(2) = 4 */
    {
        return false;
    }
    uint32_t expectedMinLen = 4;
    switch (code)
    {
    case modbus::FunctionCode::READ_COILS:
    case modbus::FunctionCode::READ_DISCRETE_INPUTS:
    case modbus::FunctionCode::READ_HOLDING_REGISTERS:
    case modbus::FunctionCode::READ_INPUT_REGISTERS:
        expectedMinLen += 4; /* 起始地址(2) + 数量(2) */
        break;
    case modbus::FunctionCode::WRITE_SINGLE_COIL:
    case modbus::FunctionCode::WRITE_SINGLE_REGISTER:
        expectedMinLen += 4; /* 地址(2) + 值(2) */
        break;
    case modbus::FunctionCode::DIAGNOSTICS:
        expectedMinLen += 2; /* 子功能码(2) */
        break;
    case modbus::FunctionCode::WRITE_MULTIPLE_COILS:
        expectedMinLen += 5; /* 起始地址(2) + 数量(2) + 字节计数(1) */
        break;
    case modbus::FunctionCode::WRITE_MULTIPLE_REGISTERS:
        expectedMinLen += 5; /* 起始地址(2) + 数量(2) + 字节计数(1) */
        break;
    default: /* 其他功能码暂不验证具体长度 */
        break;
    }
    return len >= expectedMinLen;
}
} // namespace npacket
