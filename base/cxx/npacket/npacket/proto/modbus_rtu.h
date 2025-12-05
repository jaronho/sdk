#pragma once
#include <atomic>
#include <functional>
#include <unordered_set>

#include "../protocol_parser.h"
#include "modbus_def.h"

namespace npacket
{
/**
 * @brief Modbus/RTU协议解析器
 */
class ModbusRtuParser : public ProtocolParser
{
public:
    /**
     * @brief 数据回调
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param data Modbus数据
     */
    using DATA_CALLBACK =
        std::function<void(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const modbus::DataSt& data)>;

public:
    /**
     * @brief 构造函数
     * @param addressList 允许的从站地址列表(空表示允许所有地址)
     * @param generateTransId 是否生成事务ID
     */
    ModbusRtuParser(const std::vector<uint8_t> addressList = {}, bool generateTransId = true);

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
     * @return true-成功, false-失败
     */
    bool parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header,
               const uint8_t* payload, uint32_t payloadLen) override;

    /**
     * @brief 设置数据回调
     * @param callback 回调
     */
    void setDataCallback(const DATA_CALLBACK& callback);

private:
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
    DATA_CALLBACK m_dataCallback = nullptr; /* 数据回调 */
    std::unordered_set<uint8_t> m_allowedAddressList; /* 空=允许所有地址 */
    bool m_generateTransId = false; /* 是否生成事务ID */
    std::atomic_uint16_t m_transactionId{0}; /* 事务ID生成器 */
};
} // namespace npacket
