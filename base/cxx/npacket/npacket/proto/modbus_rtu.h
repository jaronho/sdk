#pragma once
#include <atomic>
#include <functional>
#include <vector>

#include "../protocol_parser.h"
#include "modbus_def.h"

namespace npacket
{
/**
 * @brief Modbus/RTU协议配置
 */
struct ModbusRtuConfig
{
    std::vector<uint8_t> allowedAddressList; /* 允许的从站地址列表(空表示允许所有地址) */
    size_t bufferTimeout = 100; /* 缓冲区超时时间(ms) */
    size_t maxBufferSize = 4096; /* 缓冲区最大大小 */
};

/**
 * @brief Modbus/RTU协议解析器
 */
class ModbusRtuParser : public ProtocolParser
{
public:
    /**
     * @brief 非法数据类型
     */
    enum IllegalDataType
    {
        TIMEOUT, /* 超时未完整 */
        INVALID /* 无效数据(地址/功能码/CRC错误) */
    };

    /**
     * @brief 数据回调
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param data Modbus数据
     */
    using DATA_CALLBACK =
        std::function<void(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const modbus::DataSt& data)>;

    /**
     * @brief 非法数据回调(超时或无效数据)
     * @param ntp 时间点
     * @param data 数据
     * @param dataLen 数据长度
     * @param type 非法数据类型
     */
    using ILLEGAL_DATA_CALLBACK =
        std::function<void(const std::chrono::steady_clock::time_point& ntp, const uint8_t* data, uint32_t dataLen, IllegalDataType type)>;

public:
    /**
     * @brief 构造函数
     * @param cfg 配置
     */
    ModbusRtuParser(ModbusRtuConfig cfg = ModbusRtuConfig());

    /**
     * @brief 获取应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    uint32_t getProtocol() const override;

    /**
     * @brief 解析
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param payload 层负载
     * @param payloadLen 层负载长度
     * @return 解析结果
     */
    ParseResult parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header,
                      const uint8_t* payload, uint32_t payloadLen) override;

    /**
     * @brief 设置数据回调
     * @param callback 回调
     */
    void setDataCallback(const DATA_CALLBACK& callback);

    /**
     * @brief 设置非法数据回调
     * @param callback 回调
     */
    void setIllegalDataCallback(const ILLEGAL_DATA_CALLBACK& callback);

private:
    void checkTimeout(const std::chrono::steady_clock::time_point& ntp);

    ParseResult tryParseBuffer(const std::chrono::steady_clock::time_point& ntp);

    void handleIllegalData(const std::chrono::steady_clock::time_point& ntp, const uint8_t* data, uint32_t dataLen, IllegalDataType type);

    /**
     * @brief CRC16校验计算
     * @param data 数据
     * @param len 数据长度
     * @return CRC16校验值
     */
    uint16_t calcCRC16(const uint8_t* data, uint32_t len) const;

    /**
     * @brief 验证从站地址
     * @param address 从站地址
     * @return true-有效, false-无效
     */
    bool isValidSlaveAddress(uint8_t address) const;

    /**
     * @brief 生成事务ID
     * @return 事务ID
     */
    uint16_t generateTransactionId();

private:
    const ModbusRtuConfig m_cfg; /* 配置 */
    DATA_CALLBACK m_dataCallback = nullptr; /* 数据回调 */
    ILLEGAL_DATA_CALLBACK m_illegalDataCallback = nullptr; /* 非法数据回调 */
    std::vector<uint8_t> m_buffer; /* 缓冲区 */
    std::chrono::steady_clock::time_point m_lastReceiveTime; /* 上次接收时间点 */
    std::atomic_uint16_t m_transactionId{0}; /* 事务ID生成器 */
};
} // namespace npacket
