#include "modbus_rtu.h"

#include <algorithm>

namespace npacket
{
/**
 * @brief 限制ModbusRTU配置
 * @param cfg 外部定义的配置信息
 * @return 限制后的新配置
 */
ModbusRtuConfig limitModbusRtuConfig(ModbusRtuConfig cfg)
{
    if (cfg.bufferTimeout < 1 || cfg.bufferTimeout > 5000) /* 最小值1ms(防止过度敏感), 最大值5秒(防止僵尸缓存) */
    {
        cfg.bufferTimeout = 100;
    }
    if (cfg.maxBufferSize < 256 || cfg.maxBufferSize > 65535) /* 最小值256字节(单帧最大值), 最大值64KB(防止内存攻击) */
    {
        cfg.maxBufferSize = 4096;
    }
    return cfg;
}

ModbusRtuParser::ModbusRtuParser(ModbusRtuConfig cfg) : m_cfg(limitModbusRtuConfig(cfg)) {}

uint32_t ModbusRtuParser::getProtocol() const
{
    return ApplicationProtocol::MODBUS_RTU;
}

ParseResult ModbusRtuParser::parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                   const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen,
                                   uint32_t& consumeLen)
{
    consumeLen = 0;
    cleanupBuffer(ntp);
    if (!payload || 0 == payloadLen)
    {
        return ParseResult::FAILURE; /* 这里返回失败, 防止空数据占用 */
    }
    /* 追加数据到缓冲区 */
    m_buffer.insert(m_buffer.end(), payload, payload + payloadLen);
    m_lastReceiveTime = ntp;
    /* 处理缓冲区数据 */
    ParseResult result = ParseResult::FAILURE;
    if (m_buffer.size() <= m_cfg.maxBufferSize)
    {
        /* 循环解析缓冲区中的完整数据包 */
        result = ParseResult::CONTINUE;
        while (true)
        {
            if (m_buffer.size() < modbus::RTU_MIN_FRAME) /* 数据不足, 等待更多数据 */
            {
                return ParseResult::CONTINUE;
            }
            uint32_t frameLen = 0;
            result = tryParseBuffer(ntp, frameLen);
            consumeLen += frameLen;
            if (ParseResult::FAILURE == result) /* 无效数据 */
            {
                m_buffer.clear();
                break;
            }
            else if (ParseResult::CONTINUE == result) /* 数据不足 */
            {
                break;
            }
        }
    }
    return result;
}

void ModbusRtuParser::reset()
{
    m_buffer.clear();
    m_transactionId = 0;
}

void ModbusRtuParser::setDataCallback(const DATA_CALLBACK& callback)
{
    m_dataCallback = callback;
}

void ModbusRtuParser::setTimeoutDataCallback(const TIMEOUT_DATA_CALLBACK& callback)
{
    m_timeoutDataCallback = callback;
}

void ModbusRtuParser::cleanupBuffer(const std::chrono::steady_clock::time_point& ntp)
{
    if (!m_buffer.empty())
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_lastReceiveTime).count();
        if (elapsed >= m_cfg.bufferTimeout) /* 超时, 触发回调 */
        {
            if (m_timeoutDataCallback)
            {
                m_timeoutDataCallback(ntp, m_buffer.data(), m_buffer.size());
            }
            m_buffer.clear();
        }
    }
}

ParseResult ModbusRtuParser::tryParseBuffer(const std::chrono::steady_clock::time_point& ntp, uint32_t& frameLen)
{
    frameLen = 0;
    /* 检查头部字段 */
    uint8_t slaveAddress = m_buffer[0];
    uint8_t rawFuncCode = m_buffer[1];
    /* 验证从地址 */
    if (!isValidSlaveAddress(slaveAddress)) /* 地址无效, 视为无效数据 */
    {
        return ParseResult::FAILURE;
    }
    auto funcCode = (modbus::FunctionCode)(rawFuncCode & 0x7F);
    /* 验证功能码有效性 */
    if (!modbus::isValidFunctionCode((uint8_t)(funcCode))) /* 功能码无效，视为无效数据 */
    {
        return ParseResult::FAILURE;
    }
    /* 获取最小帧长度 */
    bool isException = (rawFuncCode & 0x80);
    uint32_t minDataLen = modbus::getMinFrameLength(funcCode, isException);
    frameLen = (2 + minDataLen + 2); /* 地址 + 功能码 + 数据 + CRC */
    if (m_buffer.size() < frameLen) /* 数据长度不足, 等待更多数据 */
    {
        frameLen = m_buffer.size();
        return ParseResult::CONTINUE;
    }
    /* CRC校验 */
    uint16_t crcReceived = (m_buffer[frameLen - 2] | (m_buffer[frameLen - 1] << 8));
    uint16_t crcCalculated = calcCRC16(m_buffer.data(), frameLen - 2);
    if (crcReceived != crcCalculated) /* CRC不匹配, 视为无效数据 */
    {
        return ParseResult::FAILURE;
    }
    /* 数据定位 */
    const uint8_t* dataPtr = m_buffer.data() + 2;
    modbus::ExceptionCode exceptionCode = modbus::ExceptionCode::ILLEGAL_FUNCTION;
    /* 异常响应处理 */
    if (isException)
    {
        if (minDataLen < 1) /* 异常响应必须包含异常码 */
        {
            return ParseResult::FAILURE;
        }
        exceptionCode = (modbus::ExceptionCode)(dataPtr[0]);
    }
    /* 生成数据包 */
    modbus::DataSt d;
    d.transactionId = generateTransactionId();
    d.slaveAddress = slaveAddress;
    d.isBroadcast = (0 == slaveAddress);
    d.funcCode = funcCode;
    d.data = (isException ? nullptr : dataPtr);
    d.dataLen = (isException ? 0 : minDataLen);
    d.isException = isException;
    d.exceptionCode = exceptionCode;
    /* 成功解析, 触发回调 */
    if (m_dataCallback)
    {
        m_dataCallback(ntp, d);
    }
    /* 从缓冲区移除已解析的数据 */
    m_buffer.erase(m_buffer.begin(), m_buffer.begin() + frameLen);
    return ParseResult::SUCCESS;
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
    if (m_cfg.allowedAddressList.empty()) /* 未设置白名单, 允许所有 */
    {
        return true;
    }
    return (m_cfg.allowedAddressList.end() != std::find(m_cfg.allowedAddressList.begin(), m_cfg.allowedAddressList.end(), address));
}

uint16_t ModbusRtuParser::generateTransactionId()
{
    return m_transactionId.fetch_add(1, std::memory_order_relaxed);
}
} // namespace npacket
