#include "iec103.h"

namespace npacket
{
/**
 * 控制域
 * 1. 主站 -> 从站, 控制方向
 * ------------------------------------------
 * | D7     | D6  | D5  | D4  | D3 D2 D1 D0 |
 * |--------|-----|-----|-------------------|
 * | 备用位 | PRM | FCB | FCV | 功能码      |
 * ------------------------------------------
 * 备用位: 始终为0
 * PRM(启动报文位): 为1表示主站向从站传输, 主站为启动站
 * FCB(帧计数位): 
 * FCV(帧计数有效位): 为0表示FCB的变化无效, 为1表示FCB的变化有效, 发送/无回答服务/广播报文无需考虑报文丢失和重复重传, 无需改变FCB的状态, 故FCV常为0
 * -----------------------------------------------------
 * | 功能码 | 帧类型        | 功能               | FCV |
 * |--------|---------------|--------------------------|
 * | 0      | 发送/确认帧   | 复位通信单元(CU)   | 0   | C_RCU_NA_3
 * | 1-2    | -             | 备用               | -   |
 * | 3      | 发送/确认帧   | 传送数据           | 1   |
 * | 4      | 发送/无回答帧 | 传送数据           | 0   |
 * | 5-6    | -             | 备用               | -   |
 * | 7      | 发送/确认帧   | 复位帧计数位(FCB)  | 0   | C_RFB_NA_3
 * | 8      | -             | 备用               | -   |
 * | 9      | 请求/响应帧   | 请求链路状态       | 0   | C_RLK_NA_3
 * | 10     | 请求/响应帧   | 召唤1级数据        | 1   | C_PL1_NA_3
 * | 11     | 请求/响应帧   | 召唤2级数据        | 1   | C_PL2_NA_3
 * | 12-13  | -             | 备用               | -   |
 * | 14-15  | -             | 厂商和用户协商定义 | -   |
 * -----------------------------------------------------
 * 2. 从站 -> 主站, 监视方向
 * ------------------------------------------
 * | D7     | D6  | D5  | D4  | D3 D2 D1 D0 |
 * |--------|-----|-----|-----|-------------|
 * | 备用位 | PRM | ACD | DFC | 功能码      |
 * ------------------------------------------
 * 备用位: 始终为0
 * PRM(启动报文位): 为0表示从站向主站传输
 * ACD(要求访问位): 为1表示从站希望向主站传输1级用户数据
 * DFC(数据流控制位): 为0表示从站可以接受数据, 为1表示从站缓冲区已满
 * ----------------------------------------------------
 * | 功能码 | 帧类型 | 功能                           |
 * |--------|--------|--------------------------------|
 * | 0      | 确认帧 | 确认                           | M_CON_NA_3
 * | 1      | 确认帧 | 链路忙, 未收到报文             | M_BY_NA_3
 * | 2-5    | -      | 备用                           |
 * | 6-7    | -      | 厂商和用户协商定义             |
 * | 8      | 响应帧 | 以数据响应请求帧               |
 * | 9      | 响应帧 | 无所召唤的数据                 | M_NY_NA_3
 * | 10     | -      | 备用                           |
 * | 11     | 响应帧 | 以链路状态或访问请求回答请求帧 | M_LKR_NA_3
 * | 12     | -      | 备用                           |
 * | 13     | -      | 厂商和用户协商定义             |
 * | 14     | -      | 链路服务未工作                 | M_LKR_NA_3
 * | 15     | -      | 链路服务未工作                 | M_LKR_NA_3
 * ----------------------------------------------------
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
}

void Iec103Parser::setFixedFrameCallback(const FIXED_FRAME_CALLBACK& callback)
{
    m_fixedFrameCb = callback;
}

void Iec103Parser::setVariableFrameCallback(const VARIABLE_FRAME_CALLBACK& callback)
{
    m_variableFrameCb = callback;
}

bool Iec103Parser::parseFixedFrame(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload,
                                   uint32_t payloadLen)
{
    /**
     * 固定帧:
     * -------------------------------------------------------
     * | 10H(启动字符)                                       |
     * | 控制域(C)                                           |
     * | 地址域(A), 0-254: 站地址, 255: 广播地址             |
     * | 帧校验和(CS)=(控制域+地址域)%256                    |
     * | 16H(结束字符)                                       |
     * -------------------------------------------------------
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
        uint8_t fcb_acd = (ctrl & 0x20) >> 5;
        uint8_t fcv_dfc = (ctrl & 0x10) >> 4;
        uint8_t func = (ctrl & 0xF);
        auto frame = std::make_shared<iec103::FixedFrame>();
        frame->prm = prm;
        frame->fcb_acd = fcb_acd;
        frame->fcv_dfc = fcv_dfc;
        frame->func = func;
        frame->addr = addr;
        if (m_fixedFrameCb)
        {
            m_fixedFrameCb(totalLen, header, frame);
        }
        return true;
    }
    return false;
}

bool Iec103Parser::parseVariableFrame(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload,
                                      uint32_t payloadLen)
{
    /**
     * 可变帧:
     * -------------------------------------------------------
     * | 68H(启动字符1)                                      |
     * | L(报文长度=控制域+地址域+链路规约数据)              |
     * | L(重复上面的报文长度)                               |
     * | 68H(启动字符2)                                      |
     * | 控制域(C)                                           |
     * | 地址域(A), 0-254: 站地址, 255: 广播地址             |
     * | 链路规约数据单元(ASDU), 可变长度                    |
     * | 帧校验和(CS)=(控制域+地址域+链路规约数据)%256       |
     * | 16H(结束字符)                                       |
     * -------------------------------------------------------
     * ASDU结构:
     * ---------------------------------------
     * 数据 | 类型标识(TYP)                  |
     * 单元 | 可变结构限定词(VSQ)            | 最高位: 信息体中元素的信息体地址是否连续(0x00-断续, 0x80-连续), 剩余7位: 信息体个数
     * 标识 | 传送原因(COT)                  |
     *      | 应用服务数据单元公共地址(ADDR) |
     * ----------------------------------------------
     *      | 功能类型(FUN)                  | 信息体
     *      | 信息序号(INF)                  | 标识符
     *      |----------------------------------------
     * 信   | 信息元素集, 可变长度           |
     * 息   |----------------------------------------
     * 体   | 时标毫秒(高位)                 |
     *      | 时标毫秒(低位)                 | 任
     *      | IV | RES | 时标MIN             | 选
     *      | SU | 时标HOUR                  |
     * ----------------------------------------------
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
                uint8_t fcb_acd = (ctrl & 0x20) >> 5;
                uint8_t fcv_dfc = (ctrl & 0x10) >> 4;
                uint8_t func = (ctrl & 0xF);
                auto frame = std::make_shared<iec103::VariableFrame>();
                frame->prm = prm;
                frame->fcb_acd = fcb_acd;
                frame->fcv_dfc = fcv_dfc;
                frame->func = func;
                frame->addr = addr;
                frame->asdu = parseAsdu(data, length - 2);
                if (m_variableFrameCb)
                {
                    m_variableFrameCb(totalLen, header, frame);
                }
                return true;
            }
        }
    }
    return false;
}

std::shared_ptr<iec103::Asdu> Iec103Parser::parseAsdu(const uint8_t* data, uint32_t dataLen)
{
    if (dataLen < 4)
    {
        return nullptr;
    }
    iec103::DataUnitIdentify identify;
    parseDataUnitIdentify(data, identify);
    auto asdu = parseInfoSet(identify, data + 4, dataLen - 4);
    if (!asdu)
    {
        asdu = std::make_shared<iec103::Asdu>();
        asdu->identify = identify;
    }
    return asdu;
}

void Iec103Parser::parseDataUnitIdentify(const uint8_t ch[4], iec103::DataUnitIdentify& identify)
{
    identify.type = ch[0];
    identify.vsq.continuous = (0x00 == (ch[1] & 0x80)) ? 0 : 1; /* 信息体中元素的信息体地址是否连续: 0x00-断续, 0x80-连续 */
    identify.vsq.num = (ch[1] & 0x7F); /* 信息体个数 */
    identify.cot = ch[2];
    identify.commonAddr = ch[3];
}

std::shared_ptr<iec103::Asdu> Iec103Parser::parseInfoSet(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                         uint32_t elementLen)
{
    return nullptr;
}
} // namespace npacket
