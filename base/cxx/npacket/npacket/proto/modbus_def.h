#pragma once
#include <cstdint>

namespace npacket
{
namespace modbus
{
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
 * @brief 最小区间帧长度计算器
 */
inline uint32_t getMinFrameLength(FunctionCode code)
{
    switch (code)
    {
    case FunctionCode::READ_COILS:
        return 4; /* 起始地址(2) + 数量(2) */
    case FunctionCode::READ_DISCRETE_INPUTS:
        return 4;
    case FunctionCode::READ_HOLDING_REGISTERS:
        return 4;
    case FunctionCode::READ_INPUT_REGISTERS:
        return 4;
    case FunctionCode::WRITE_SINGLE_COIL:
        return 4; /* 地址(2) + 值(2) */
    case FunctionCode::WRITE_SINGLE_REGISTER:
        return 4;
    case FunctionCode::DIAGNOSTICS:
        return 2; /* 子功能(2) */
    case FunctionCode::WRITE_MULTIPLE_COILS:
        return 5; /* 起始地址(2) + 数量(2) + 字节计数(1) */
    case FunctionCode::WRITE_MULTIPLE_REGISTERS:
        return 5;
    default:
        return 1; /* 至少功能码 */
    }
}
} // namespace modbus
} // namespace npacket
