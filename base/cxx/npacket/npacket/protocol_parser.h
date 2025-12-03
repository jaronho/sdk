#pragma once
#include <chrono>
#include <mutex>
#include <vector>

#include "protocol.h"

namespace npacket
{
/**
 * @brief 应用层协议
 */
enum ApplicationProtocol
{
    NONE, /* 无 */
    FTP, /* 文件传输协议 */
    HTTP, /* 超文本传输协议 */
    IEC103, /* 一种用于电力系统远程监控和控制的通信协议, 由国际电工委员会(IEC)制定 */
    TPKT, /* 介于TCP和COTP协议之间, 属于传输服务类的协议, 主要用来在TCP和COTP之间建立桥梁, 一般与COTP一起发送, 当作Header段 */
    COTP, /* 面向连接的传输协议(Connection-Oriented Transport Protocol), 上层协议为TPKT协议 */
    S7COMM, /* (S7 Communication)是西门子S7通讯协议簇里的一种, 上层协议为COTP协议 */
    MODBUS_RTU, /* Modbus远程终端单元协议(基于串行通信) */
    MODBUS_TCP, /* Modbus传输控制协议(基于以太网通信) */
};

/**
 * @brief 应用层协议解析器(接口类)
 */
class ProtocolParser
{
public:
    /**
     * @brief 获取父应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    virtual uint32_t getParentProtocol() const;

    /**
     * @brief 获取应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    virtual uint32_t getProtocol() const = 0;

    /**
     * @brief 解析
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param payload 传输层负载
     * @param payloadLen 传输层负载长度
     * @return true-成功, false-失败
     */
    virtual bool parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header,
                       const uint8_t* payload, uint32_t payloadLen) = 0;
};
} // namespace npacket
