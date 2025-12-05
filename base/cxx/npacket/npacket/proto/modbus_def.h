#pragma once
#include <stdint.h>

namespace npacket
{
namespace modbus
{
constexpr uint32_t MAX_PDU_LENGTH = 253; /* Modbus协议最大PDU长度 */
constexpr uint32_t RTU_MIN_FRAME = 4; /* Modbus/RTU最小帧长度: 地址(1) + 功能码(1) + CRC(2) = 4字节 */
constexpr uint32_t MBAP_HEADER_LEN = 7; /* Modbus/TCP MBAP头固定长度: 事务ID(2) + 协议ID(2) + 长度(2) + 单元ID(1) = 7字节 */

/**
 * @brief 功能码
 */
enum FunctionCode
{
    /* 基础功能码 */
    READ_COILS = 0x01,
    READ_DISCRETE_INPUTS = 0x02,
    READ_HOLDING_REGISTERS = 0x03,
    READ_INPUT_REGISTERS = 0x04,
    WRITE_SINGLE_COIL = 0x05,
    WRITE_SINGLE_REGISTER = 0x06,
    READ_EXCEPTION_STATUS = 0x07,

    /* 诊断功能码 */
    DIAGNOSTICS = 0x08,
    GET_COM_EVENT_COUNTER = 0x0B,
    GET_COM_EVENT_LOG = 0x0C,
    REPORT_SERVER_ID = 0x11,

    /* 高级功能码 */
    WRITE_MULTIPLE_COILS = 0x0F,
    WRITE_MULTIPLE_REGISTERS = 0x10,
    MASK_WRITE_REGISTER = 0x16,
    READ_WRITE_MULTIPLE_REGISTERS = 0x17,
    READ_FIFO_QUEUE = 0x18,

    /* 文件记录 */
    READ_FILE_RECORD = 0x14,
    WRITE_FILE_RECORD = 0x15,

    /* 封装接口 */
    READ_DEVICE_IDENTIFICATION = 0x2B, /* 0x2B/0x0E */

    /* 异常响应标记 */
    EXCEPTION_MASK = 0x80
};

/**
 * @brief 异常响应码
 */
enum ExceptionCode
{
    ILLEGAL_FUNCTION = 0x01,
    ILLEGAL_DATA_ADDRESS = 0x02,
    ILLEGAL_DATA_VALUE = 0x03,
    SERVER_DEVICE_FAILURE = 0x04,
    ACKNOWLEDGE = 0x05,
    SERVER_DEVICE_BUSY = 0x06,
    MEMORY_PARITY_ERROR = 0x08,
    GATEWAY_PATH_UNAVAILABLE = 0x0A,
    GATEWAY_TARGET_FAILED_TO_RESPOND = 0x0B
};

/**
 * @brief Modbus数据
 */
struct DataSt
{
    uint16_t transactionId = 0; /* 事务ID(TCP)或自增ID(RTU) */
    uint8_t slaveAddress = 0; /* 从站地址 */
    bool isBroadcast = false; /* 是否为广播地址(RTU地址0) */
    FunctionCode funcCode; /* 功能码 */
    const uint8_t* data; /* 数据指针 */
    uint32_t dataLen = 0; /* 数据长度 */
    bool isException = false; /* 是否为异常响应 */
    ExceptionCode exceptionCode; /* 异常码(仅当isException为true时有效) */
};

/**
 * @brief 最小区间帧长度计算器
 * @param code 功能码
 * @return 最小帧长度
 */
inline uint32_t getMinFrameLength(FunctionCode code)
{
    switch (code)
    {
    /* 基础读写功能码 */
    case FunctionCode::READ_COILS:
    case FunctionCode::READ_DISCRETE_INPUTS:
    case FunctionCode::READ_HOLDING_REGISTERS:
    case FunctionCode::READ_INPUT_REGISTERS:
        return 4; /* 起始地址(2) + 数量(2) */
    case FunctionCode::WRITE_SINGLE_COIL:
    case FunctionCode::WRITE_SINGLE_REGISTER:
        return 4; /* 地址(2) + 值(2) */
    case FunctionCode::READ_EXCEPTION_STATUS:
        return 1; /* 仅功能码 */
    /* 诊断功能码 */
    case FunctionCode::DIAGNOSTICS:
        return 2; /* 子功能码(2) */
    case FunctionCode::GET_COM_EVENT_COUNTER:
    case FunctionCode::GET_COM_EVENT_LOG:
        return 1; /* 仅功能码 */
    case FunctionCode::REPORT_SERVER_ID:
        return 1; /* 仅功能码 */
    /* 批量操作功能码 */
    case FunctionCode::WRITE_MULTIPLE_COILS:
    case FunctionCode::WRITE_MULTIPLE_REGISTERS:
        return 5; /* 起始地址(2) + 数量(2) + 字节计数(1) */
    case FunctionCode::MASK_WRITE_REGISTER:
        return 6; /* 地址(2) + AND掩码(2) + OR掩码(2) */
    case FunctionCode::READ_WRITE_MULTIPLE_REGISTERS:
        return 10; /* 读起始地址(2) + 读数量(2) + 写起始地址(2) + 写数量(2) + 写字节计数(1) */
    case FunctionCode::READ_FIFO_QUEUE:
        return 2; /* 队列指针地址(2) */
    /* 文件记录功能码 */
    case FunctionCode::READ_FILE_RECORD:
    case FunctionCode::WRITE_FILE_RECORD:
        return 9; /* 功能码(1) + 字节计数(1) + 最少1个子请求(7) */
    /* 封装接口 */
    case FunctionCode::READ_DEVICE_IDENTIFICATION:
        return 3; /* MEI类型(1) + 读设备ID码(1) + 对象ID(1) */
    default:
        return 1; /* 默认至少功能码 */
    }
}

/**
 * @brief 验证功能码有效性
 * @param funcCode 功能码
 * @return true-有效, false-无效
 */
inline bool isValidFunctionCode(uint8_t funcCode)
{
    funcCode &= 0x7F; /* 清除异常标记 */
    switch (funcCode)
    {
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x0B:
    case 0x0C:
    case 0x0F:
    case 0x10:
    case 0x11:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x2B:
        return true;
    default:
        return false;
    }
}
} // namespace modbus
} // namespace npacket
