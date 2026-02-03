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
     * @brief 数据回调
     * @param ntp 数据包接收时间点
     * @param data Modbus数据
     */
    using DATA_CALLBACK =
        std::function<void(const std::chrono::steady_clock::time_point& ntp, const std::shared_ptr<modbus::DataSt>& data)>;

    /**
     * @brief 超时数据回调
     * @param ntp 时间点
     * @param data 数据
     * @param dataLen 数据长度
     */
    using TIMEOUT_DATA_CALLBACK =
        std::function<void(const std::chrono::steady_clock::time_point& ntp, const uint8_t* data, uint32_t dataLen)>;

public:
    /**
     * @brief 构造函数
     * @param isMaster 主站还是从站, true-主站, false-从站
     * @param cfg 配置
     */
    ModbusRtuParser(bool isMaster, ModbusRtuConfig cfg = ModbusRtuConfig());

    /**
     * @brief 获取应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    uint32_t getProtocol() const noexcept override;

    /**
     * @brief 解析
     * @param num 数据序号
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param payload 负载
     * @param payloadLen 负载长度
     * @param consumeLen [输出]消耗的长度
     * @return 解析结果
     */
    ParseResult parse(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                      const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen) override;

    /**
     * @brief 重置
     */
    void reset() override;

    /**
     * @brief 设置数据回调
     * @param callback 回调
     */
    void setDataCallback(const DATA_CALLBACK& callback);

    /**
     * @brief 设置超时数据回调
     * @param callback 回调
     */
    void setTimeoutDataCallback(const TIMEOUT_DATA_CALLBACK& callback);

private:
    /**
     * @brief 清理缓存
     * @param ntp 时间点
     */
    void cleanupBuffer(const std::chrono::steady_clock::time_point& ntp);

    /**
     * @brief 尝试解析缓存
     * @param ntp 时间点
     * @param frameLen [输出]帧长度
     * @return 解析结果
     */
    ParseResult tryParseBuffer(const std::chrono::steady_clock::time_point& ntp, uint32_t& frameLen);

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
    const bool m_isMaster; /* 主站还是从站, true-主站, false-从站 */
    const ModbusRtuConfig m_cfg; /* 配置 */
    DATA_CALLBACK m_dataCallback = nullptr; /* 数据回调 */
    TIMEOUT_DATA_CALLBACK m_timeoutDataCallback = nullptr; /* 超时数据回调 */
    std::vector<uint8_t> m_buffer; /* 缓冲区 */
    std::chrono::steady_clock::time_point m_lastReceiveTime; /* 上次接收时间点 */
    std::atomic_uint16_t m_transactionId{0}; /* 事务ID生成器 */
};
} // namespace npacket
