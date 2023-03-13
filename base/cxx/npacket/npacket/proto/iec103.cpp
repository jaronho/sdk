#include "iec103.h"

#include <unordered_map>

namespace npacket
{
/**
 * 控制域
 * 1. 主控单元 -> 保护设备
 * |----------------------------------------|
 * | D7     | D6  | D5  | D4  | D3 D2 D1 D0 |
 * | 备用位 | PRM | FCB | FCV | 功能码      |
 * |----------------------------------------|
 * 备用位: 始终为0
 * PRM(启动报文位): 为1表示主控单元向保护设备传输, 主控单元为启动站
 * FCB(帧计数位): 
 * FCV(帧计数有效位): 为0表示FCB的变化无效, 为1表示FCB的变化有效, 发送/无回答服务/广播报文无需考虑报文丢失和重复重传, 无需改变FCB的状态, 故FCV常为0
 * |--------------------------------------------------|
 * | 功能码 | 帧类型        | 功能              | FCV |
 * | 0      | 发送/确认帧   | 复位通信单元(CU)  | 0   |
 * | 1-2    | -             | 备用              | -   |
 * | 3      | 发送/确认帧   | 传送数据          | 1   |
 * | 4      | 发送/无回答帧 | 传送数据          | 0   |
 * | 5-6    | -             | 备用              | -   |
 * | 7      | 发送/确认帧   | 复位帧计数位(FCB) | 0   |
 * | 8      | -             | 备用              | -   |
 * | 9      | -             | 备用              | -   |
 * | 10(A)  | 请求/响应帧   | 召唤1级用户数据   | 1   |
 * | 11(B)  | 请求/响应帧   | 召唤2级用户数据   | 1   |
 * | 12-13  | -             | 备用              | -   |
 * | 14-15  | -             | 备用              | -   |
 * |--------------------------------------------------|
 * 2. 保护设备 -> 主控单元
 * |----------------------------------------|
 * | D7     | D6  | D5  | D4  | D3 D2 D1 D0 |
 * | 备用位 | PRM | ACD | DFC | 功能码      |
 * |----------------------------------------|
 * 备用位: 始终为0
 * PRM(启动报文位): 为0表示保护设备向主控单元传输
 * ACD(要求访问位): 为1表示保护设备希望向主控单元传输1级用户数据
 * DFC(数据流控制位): 为0表示保护设备可以接受数据, 为1表示保护设备缓冲区已满
 * |--------------------------------------|
 * | 功能码 | 帧类型 | 功能               |
 * | 0      | 确认帧 | 确认               |
 * | 1      | 确认帧 | 链路忙, 未收到报文 |
 * | 2-5    | -      | 备用               |
 * | 6-7    | -      | 备用               |
 * | 8      | 响应帧 | 以数据响应请求帧   |
 * | 9      | 响应帧 | 无所召唤的数据     |
 * | 10-15  | -      | 备用               |
 * |--------------------------------------|
 */
uint32_t Iec103Parser::getProtocol() const
{
    return ApplicationProtocol::IEC103;
}

bool Iec103Parser::parse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
{
    if (header && TransportProtocol::TCP != header->getProtocol() && TransportProtocol::UDP != header->getProtocol())
    {
        return false;
    }
    if (payloadLen < 5) /* IEC103包最小5个字节 */
    {
        return false;
    }
    if (parseFixedFrame(totalLen, header, payload, payloadLen))
    {
        return true;
    }
    else if (parseVariableFrame(totalLen, header, payload, payloadLen))
    {
        return true;
    }
    return false;
    return false;
}

bool Iec103Parser::parseFixedFrame(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload,
                                   uint32_t payloadLen)
{
    /**
     * |----------------------------------|
     * | 10H(启动字符)                    |
     * | 控制域(C)                        |
     * | 地址域(A), 保护设备的地址        |
     * | 帧校验和(CS)=(控制域+地址域)%256 |
     * | 16H(结束字符)                    |
     * |----------------------------------|
     */
    if (5 == payloadLen && 0x10 == payload[0] && 0x16 == payload[4])
    {
        return false;
    }
    uint8_t ctrl = payload[1];
    uint8_t addr = payload[2];
    uint8_t checksum = payload[3];
    if ((ctrl + addr) % 256 == checksum) /* 校验成功 */
    {
        uint8_t prm = (ctrl & 0x40) >> 6;
        if (1 == prm) /* 主控单元 -> 保护设备 */
        {
            uint8_t fcb = (ctrl & 0x20) >> 5;
            uint8_t fcv = (ctrl & 0x10) >> 4;
            uint8_t func = (ctrl & 0xF);
            onMaster(totalLen, header, fcb, fcv, func, nullptr, 0);
        }
        else if (0 == prm) /* 保护设备 -> 主控单元 */
        {
            uint8_t acd = (ctrl & 0x20) >> 5;
            uint8_t dfc = (ctrl & 0x10) >> 4;
            uint8_t func = (ctrl & 0xF);
            onSlave(totalLen, header, acd, dfc, func, nullptr, 0);
        }
        return true;
    }
    return false;
}

bool Iec103Parser::parseVariableFrame(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload,
                                      uint32_t payloadLen)
{
    /**
     * |-----------------------------------------------|
     * | 68H(启动字符1)                                |
     * | L(报文长度=控制域+地址域+链路用户数据)        |
     * | L(重复上面的报文长度)                         |
     * | 68H(启动字符2)                                |
     * | 控制域(C)                                     |
     * | 地址域(A), 保护设备的地址                     |
     * | 链路用户数据(可变长度)                        |
     * | 帧校验和(CS)=(控制域+地址域+链路用户数据)%256 |
     * | 16H(结束字符)                                 |
     * |-----------------------------------------------|
     */
    if (payloadLen >= 9 && 0x68 == payload[0] && payload[1] == payload[2] && 0x68 == payload[3] && 0x16 == payload[payloadLen - 1])
    {
        uint8_t length = payload[1];
        uint32_t frameLen = 4 + length + 2;
        if (frameLen == payloadLen)
        {
            uint8_t ctrl = payload[4];
            uint8_t addr = payload[5];
            const uint8_t* data = payload + 6;
            uint8_t dataLen = length - 2;
            uint8_t checksum = payload[payloadLen - 2];
            uint64_t s = (uint64_t)ctrl + addr;
            for (uint8_t i = 0; i < dataLen; ++i)
            {
                s += data[i];
            }
            if (s % 256 == checksum) /* 校验成功 */
            {
                uint8_t prm = (ctrl & 0x40) >> 6;
                if (1 == prm) /* 主控单元 -> 保护设备 */
                {
                    uint8_t fcb = (ctrl & 0x20) >> 5;
                    uint8_t fcv = (ctrl & 0x10) >> 4;
                    uint8_t func = (ctrl & 0xF);
                    onMaster(totalLen, header, fcb, fcv, func, data, dataLen);
                }
                else if (0 == prm) /* 保护设备 -> 主控单元 */
                {
                    uint8_t acd = (ctrl & 0x20) >> 5;
                    uint8_t dfc = (ctrl & 0x10) >> 4;
                    uint8_t func = (ctrl & 0xF);
                    onSlave(totalLen, header, acd, dfc, func, data, dataLen);
                }
                return true;
            }
        }
    }
    return false;
}

void Iec103Parser::onMaster(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, uint8_t fcb, uint8_t fcv, uint8_t func,
                            const uint8_t* data, uint8_t dataLen)
{
}

void Iec103Parser::onSlave(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, uint8_t acd, uint8_t dfc, uint8_t func,
                           const uint8_t* data, uint8_t dataLen)
{
}
} // namespace npacket
