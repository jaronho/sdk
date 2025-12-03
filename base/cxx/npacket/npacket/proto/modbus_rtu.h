#pragma once
#include <functional>

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
     * @param slaveAddr 从站地址
     * @param funcCode 功能码
     * @param data 数据
     * @param dataLen 数据长度
     * @param isException 是否为异常响应
     * @param exceptionCode 异常码(仅当isException为true时有效)
     */
    using DATA_CALLBACK = std::function<void(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, uint8_t slaveAddress,
                                             modbus::FunctionCode funcCode, const uint8_t* data, uint32_t dataLen, bool isException,
                                             modbus::ExceptionCode exceptionCode)>;

public:
    /**
     * @brief 构造函数
     * @param addressList 允许的从站地址列表(空表示允许所有地址)
     */
    ModbusRtuParser(const std::vector<uint8_t>& addressList = {});

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
     * @brief 验证帧长度与功能码匹配
     * @param code 功能码
     * @param len 帧长度
     * @return true-匹配, false-不匹配
     */
    bool validateFrameLength(modbus::FunctionCode code, uint32_t len) const;

private:
    DATA_CALLBACK m_dataCallback = nullptr; /* 数据回调 */
    std::vector<uint8_t> m_allowedAddressList; /* 空=允许所有地址 */
};
} // namespace npacket
