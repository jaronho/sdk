#pragma once
#include <chrono>

#include "protocol.h"

namespace npacket
{
/**
 * @brief 应用层协议
 */
enum ApplicationProtocol
{
    NONE = 0, /* 无 */
    AMQP, /* 高级消息队列协议(Advanced Message Queuing Protocol), 是一种开放标准的应用层消息中间件协议, 设计用于面向消息的中间件(MOM)系统 */
    BACNET, /* BACNET协议(主要基于UDP传输) */
    TPKT_COTP, /* 传输层轻量级封装(Transport Packet) -> 面向连接的传输协议(Connection-Oriented Transport Protocol) */
    DNP3, /* DNP3协议(主要用于电力和水务自动化) */
    ETHERNET_IP, /* 以太网工业协议(Ethernet Industrial Protocol), 它是CIP(Common Industrial Protocol)在标准以太网上的具体实现 */
    FTP, /* 文件传输协议 */
    HTTP, /* 超文本传输协议 */
    IEC103, /* 一种用于电力系统远程监控和控制的通信协议, 由国际电工委员会(IEC)制定 */
    IEC104, /* 一种用于电力系统远程监控和控制的通信协议, 由国际电工委员会(IEC)制定 */
    MODBUS_RTU, /* Modbus远程终端单元协议(基于串行通信) */
    MODBUS_TCP, /* Modbus传输控制协议(基于以太网通信) */
    MQTT, /* 消息队列遥测传输(Message Queuing Telemetry Transport), 是一种轻量级的发布/订阅消息传输协议 */
    NFS, /* 网络文件系统协议 */
    OMRON_FINS, /* 工厂接口网络服务(Factory Interface Network Service), 是欧姆龙(OMRON)公司开发的专有工业通信协议, 用于其PLC与上位机之间的数据交换 */
    OPC_UA, /* 开放平台通信统一架构(Open Platform Communications Unified Architecture), 是一种跨平台面向服务的工业通信协议 */
    POP3, /* 邮局协议3(用于电子邮件接收) */
    PROFINET, /* PROFINET协议(主要用于工业自动化) */
    SMB2, /* 第二代服务器消息块协议(Server Message Block v2) */
    SMTP, /* 简单邮件传输协议(用于电子邮件发送) */
    TELNET, /* 基于TCP的远程登录协议 */
};

/**
 * @brief 解析结果
 */
enum ParseResult
{
    SUCCESS = 0, /* 解析成功 */
    FAILURE, /* 解析失败 */
    CONTINUE, /* 数据不足, 需要继续接收 */
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
    virtual uint32_t getParentProtocol() const noexcept;

    /**
     * @brief 获取应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    virtual uint32_t getProtocol() const noexcept = 0;

    /**
     * @brief 解析
     * @param pd 协议数据
     * @param consumeLen [输出]消耗的长度, 重要: 解析成功时, 需要在接口内部对此变量赋值所解析的数据长度, 解析失败/数据不足时赋值0
     * @return 解析结果
     */
    virtual ParseResult parse(const ProtocolData& pd, uint32_t& consumeLen) = 0;

    /**
     * @brief 重置
     */
    virtual void reset();
};
} // namespace npacket
