#pragma once
#include <memory>
#include <string>
#include <vector>

#include "protocol_raw.h"

namespace npacket
{
/**
 * @brief 协议头部
 */
class ProtocolHeader
{
public:
    /**
     * @brief 获取协议在TCP/IP四层模型的第几层
     * @return 层类型
     */
    virtual LayerType getLayerType() const = 0;

    /**
     * @brief 获取(网络层/传输层)协议类型
     * @return 协议类型(NetworkProtocolType/TransportProtocolType)
     */
    virtual uint32_t getProtocolType() const = 0;

    std::shared_ptr<ProtocolHeader> parent = nullptr; /* 父层头部 */
};

/**
 * @brief 以太网II协议头部
 */
class EthernetIIHeader final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinSize()
    {
        return 14;
    }

    /**
     * @brief 获取协议在TCP/IP四层模型的第几层
     * @return 物理层
     */
    LayerType getLayerType() const override
    {
        return LayerType::physical;
    }

    /**
     * @brief 获取协议类型
     * @return 物理层返回值恒为0
     */
    uint32_t getProtocolType() const override
    {
        return 0;
    }

    uint8_t header_len = 0; /* 头部长度 */
    std::vector<std::string> dst_mac; /* 目标MAC地址 */
    std::vector<std::string> src_mac; /* 源MAC地址 */
    uint32_t next_protocol = 0; /* 下一层协议类型 */
};

/**
 * @brief IPv4协议头部
 */
class Ipv4Header final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinSize()
    {
        return 20;
    }

    /**
     * @brief 获取协议在TCP/IP四层模型的第几层
     * @return 网络层
     */
    LayerType getLayerType() const override
    {
        return LayerType::network;
    }

    /**
     * @brief 获取协议类型
     * @return 网络层IPv4协议类型(NetworkProtocolType)
     */
    uint32_t getProtocolType() const override
    {
        return NetworkProtocolType::IPv4;
    }

    uint8_t version = 0; /* 版本 */
    uint8_t header_len = 0; /* 头部长度 */
    uint8_t tos = 0; /* 服务类型 */
    uint16_t total_len = 0; /* 报文总长度 */
    uint16_t identification = 0; /* 标识 */
    uint8_t flag_reserved = 0; /* 分段标志: 保留位 */
    uint8_t flag_dont = 0; /* 分段标志: 没有分段 */
    uint8_t flag_more = 0; /* 分段标志: 更多分段 */
    uint8_t frag_offset = 0; /* 分段偏移数 */
    uint8_t ttl = 0; /* 报文生存时间 */
    uint8_t next_protocol = 0; /* 下一层协议类型 */
    uint16_t checksum = 0; /* 头部校验和 */
    std::string src_addr; /* 源IP地址 */
    std::string dst_addr; /* 目的IP地址 */
};

/**
 * @brief ARP协议头部
 */
class ArpHeader final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinSize()
    {
        return 28;
    }

    /**
     * @brief 获取协议在TCP/IP四层模型的第几层
     * @return 网络层
     */
    LayerType getLayerType() const override
    {
        return LayerType::network;
    }

    /**
     * @brief 获取协议类型
     * @return 网络层ARP协议类型(NetworkProtocolType)
     */
    uint32_t getProtocolType() const override
    {
        return NetworkProtocolType::ARP;
    }

    uint8_t header_len = 0; /* 头部长度 */
    uint16_t hardware_type = 0; /* 硬件地址类型(表示物理网络类型, 即数据链路层使用的协议, 其中0x0001为以太网) */
    uint16_t protocol_type = 0; /* 协议地址类型(网络层使用的协议) */
    uint8_t hardware_size = 0; /* 硬件地址长度(源和目的物理地址的长度, 单位字节) */
    uint8_t protocol_size = 0; /* 协议地址长度(源和目的的协议地址的长度, 单位字节) */
    uint16_t opcode = 0; /* 操作(记录该报文的类型, 其中1表示ARP请求报文, 2表示ARP响应报文) */
    std::vector<std::string> sender_mac; /* 源MAC地址 */
    std::string sender_ip; /* 源IP地址 */
    std::vector<std::string> target_mac; /* 目的MAC地址 */
    std::string target_ip; /* 目的IP地址 */
};

/**
 * @brief IPv6协议头部
 */
class Ipv6Header final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinSize()
    {
        return 40;
    }

    /**
     * @brief 获取协议在TCP/IP四层模型的第几层
     * @return 网络层
     */
    LayerType getLayerType() const override
    {
        return LayerType::network;
    }

    /**
     * @brief 获取协议类型
     * @return 网络层IPv6协议类型(NetworkProtocolType)
     */
    uint32_t getProtocolType() const override
    {
        return NetworkProtocolType::IPv6;
    }

    uint8_t version = 0; /* 版本 */
    uint8_t header_len = 0; /* 头部长度 */
    uint8_t traffic_class = 0; /* 通信类别 */
    uint32_t flow_label = 0; /* 流标记 */
    uint16_t payload_len = 0; /* 负载长度 */
    uint8_t next_protocol = 0; /* 下一层协议类型 */
    uint8_t hop_limit = 0; /* 跳跃限制 */
    std::vector<std::string> src_addr; /* 源IP地址 */
    std::vector<std::string> dst_addr; /* 目的IP地址 */
};

/**
 * @brief TCP协议头部
 */
class TcpHeader final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinSize()
    {
        return 20;
    }

    /**
     * @brief 获取协议在TCP/IP四层模型的第几层
     * @return 传输层
     */
    LayerType getLayerType() const override
    {
        return LayerType::transport;
    }

    /**
     * @brief 获取协议类型
     * @return 传输层TCP协议类型(TransportProtocolType)
     */
    uint32_t getProtocolType() const override
    {
        return TransportProtocolType::TCP;
    }

    uint16_t src_port = 0; /* 源端口 */
    uint16_t dst_port = 0; /* 目的端口 */
    uint32_t seq = 0; /* 序号 */
    uint32_t ack = 0; /* 确认序号 */
    uint8_t header_len = 0; /* 头部长度 */
    uint8_t flag_reserved = 0; /* 标志: 保留位 */
    uint8_t flag_nonce = 0; /* 标志: 保留位 */
    uint8_t flag_cwr = 0; /* 标志: 保留位 */
    uint8_t flag_ecn_echo = 0; /* 标志: 保留位 */
    uint8_t flag_urgent = 0; /* 标志: 紧急 */
    uint8_t flag_ack = 0; /* 标志: 确认 */
    uint8_t flag_push = 0; /* 标志: 推送 */
    uint8_t flag_reset = 0; /* 标志: 复位 */
    uint8_t flag_syn = 0; /* 标志: 同步 */
    uint8_t flag_fin = 0; /* 标志: 终止 */
    uint16_t window = 0; /* 窗口大小 */
    uint16_t checksum = 0; /* 检验和 */
    uint16_t urgptr = 0; /* 紧急指针 */
};

/**
 * @brief UDP协议头部
 */
class UdpHeader final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinSize()
    {
        return 8;
    }

    /**
     * @brief 获取协议在TCP/IP四层模型的第几层
     * @return 传输层
     */
    LayerType getLayerType() const override
    {
        return LayerType::transport;
    }

    /**
     * @brief 获取协议类型
     * @return 传输层UDP协议类型(TransportProtocolType)
     */
    uint32_t getProtocolType() const override
    {
        return TransportProtocolType::UDP;
    }

    uint8_t header_len = 0; /* 头部长度 */
    uint16_t src_port = 0; /* 源端口 */
    uint16_t dst_port = 0; /* 目的端口 */
    uint16_t total_len = 0; /* 报文总长度 */
    uint16_t checksum = 0; /* 检验和 */
};
} // namespace npacket
