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

bool Iec103Parser::parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header,
                         const uint8_t* payload, uint32_t payloadLen)
{
    if (header && TransportProtocol::TCP != header->getProtocol() && TransportProtocol::UDP != header->getProtocol())
    {
        return false;
    }
    if (payloadLen < 5) /* IEC103包最小5个字节 */
    {
        return false;
    }
    if (parseFixedFrame(ntp, totalLen, header, payload, payloadLen))
    {
        return true;
    }
    else if (parseVariableFrame(ntp, totalLen, header, payload, payloadLen))
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

bool Iec103Parser::parseFixedFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                   const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
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
                m_fixedFrameCb(ntp, totalLen, header, frame);
            }
            return true;
        }
    }
    return false;
}

bool Iec103Parser::parseVariableFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                      const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
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
                    m_variableFrameCb(ntp, totalLen, header, frame);
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
    }
    asdu->identify = identify;
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
    switch (identify.type)
    {
    case 0x01:
        return parseAsud1(identify, elements, elementLen);
    default:
        break;
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu1> Iec103Parser::parseAsud1(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    if (elementLen > 2)
    {
        auto asdu = std::make_shared<iec103::Asdu1>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        uint32_t offset = 2;
        int32_t count = getDpi(elements + offset, elementLen - offset, asdu->dpi);
        if (count < 0)
        {
            return nullptr;
        }
        offset += count;
        count = getCP32Time2a(elements + offset, elementLen - offset, asdu->tm);
        if (count < 0)
        {
            return nullptr;
        }
        offset += count;
        count = getSin(elements + offset, elementLen - offset, asdu->sin);
        if (count < 0)
        {
            return nullptr;
        }
        return asdu;
    }
    return nullptr;
}

int Iec103Parser::getAcc(const uint8_t* data, uint32_t dataLen, iec103::ACC& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getAsc(const uint8_t* data, uint32_t dataLen, iec103::ASC& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getCol(const uint8_t* data, uint32_t dataLen, iec103::COL& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getDco(const uint8_t* data, uint32_t dataLen, iec103::DCO& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0] & 0x03; /* 占2位 */
        return 1;
    }
    return -1;
}

int Iec103Parser::getDpi(const uint8_t* data, uint32_t dataLen, iec103::DPI& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0] & 0x03; /* 占2位 */
        return 1;
    }
    return -1;
}

int Iec103Parser::getSpi(const uint8_t* data, uint32_t dataLen, iec103::SPI& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0] & 0x01; /* 占1位 */
        return 1;
    }
    return -1;
}

int Iec103Parser::getTedpi(const uint8_t* data, uint32_t dataLen, iec103::TEDPI& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0] & 0x03; /* 占2位 */
        return 1;
    }
    return -1;
}

int Iec103Parser::getFan(const uint8_t* data, uint32_t dataLen, iec103::FAN& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val = (data[1] << 8) | data[0];
        return 2;
    }
    return -1;
}

int Iec103Parser::getInt(const uint8_t* data, uint32_t dataLen, iec103::INT& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val = (data[1] << 8) | data[0];
        return 2;
    }
    return -1;
}

int Iec103Parser::getMea(const uint8_t* data, uint32_t dataLen, iec103::MEA& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val.ov = (0x01 == (data[0] & 0x01)) ? 1 : 0; /* 第1位 */
        val.er = (0x02 == (data[0] & 0x02)) ? 1 : 0; /* 第2位 */
        val.res = (0x04 == (data[0] & 0x04)) ? 1 : 0; /* 第3位 */
        uint8_t sign = (0x80 == (data[1] & 0x80)) ? 1 : 0; /* 符号位 */
        uint32_t tmp = (data[1] << 8) | (data[0] & 0xf8);
        if (1 == sign) /* 负数 */
        {
            val.mval = -1.0 * (float)((~tmp & 0xfff8) + 0x0008) / 0x8000;
        }
        else if (0 == sign) /* 正数 */
        {
            val.mval = (float)tmp / 0x8000;
        }
        return 2;
    }
    return -1;
}

int Iec103Parser::getNfe(const uint8_t* data, uint32_t dataLen, iec103::NFE& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val = (data[1] << 8) | data[0];
        return 2;
    }
    return -1;
}

int Iec103Parser::getNoc(const uint8_t* data, uint32_t dataLen, iec103::NOC& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getNoe(const uint8_t* data, uint32_t dataLen, iec103::NOE& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val = (data[1] << 8) | data[0];
        return 2;
    }
    return -1;
}

int Iec103Parser::getNof(const uint8_t* data, uint32_t dataLen, iec103::NOF& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val = (data[1] << 8) | data[0];
        return 2;
    }
    return -1;
}
int Iec103Parser::getNot(const uint8_t* data, uint32_t dataLen, iec103::NOT& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getNdv(const uint8_t* data, uint32_t dataLen, iec103::NDV& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getRet(const uint8_t* data, uint32_t dataLen, iec103::RET& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val = (data[1] << 8) | data[0];
        return 2;
    }
    return -1;
}

int Iec103Parser::getRfa(const uint8_t* data, uint32_t dataLen, iec103::RFA& val)
{
    if (data && dataLen >= 4) /* 4个字节 */
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, 4);
        return 4;
    }
    return -1;
}

int Iec103Parser::getRpv(const uint8_t* data, uint32_t dataLen, iec103::RPV& val)
{
    if (data && dataLen >= 4) /* 4个字节 */
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, 4);
        return 4;
    }
    return -1;
}

int Iec103Parser::getRsv(const uint8_t* data, uint32_t dataLen, iec103::RSV& val)
{
    if (data && dataLen >= 4) /* 4个字节 */
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, 4);
        return 4;
    }
    return -1;
}

int Iec103Parser::getRii(const uint8_t* data, uint32_t dataLen, iec103::RII& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getScl(const uint8_t* data, uint32_t dataLen, iec103::SCL& val)
{
    if (data && dataLen >= 4) /* 4个字节 */
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, 4);
        return 4;
    }
    return -1;
}

int Iec103Parser::getScn(const uint8_t* data, uint32_t dataLen, iec103::SCN& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getSdv(const uint8_t* data, uint32_t dataLen, iec103::SDV& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val = (data[1] << 8) | data[0];
        return 2;
    }
    return -1;
}

int Iec103Parser::getSin(const uint8_t* data, uint32_t dataLen, iec103::SIN& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getSof(const uint8_t* data, uint32_t dataLen, iec103::SOF& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val.tp = (0x01 == (data[0] & 0x01)) ? 1 : 0;
        val.tm = (0x02 == (data[0] & 0x02)) ? 1 : 0;
        val.test = (0x04 == (data[0] & 0x04)) ? 1 : 0;
        val.otev = (0x08 == (data[0] & 0x08)) ? 1 : 0;
        val.res = (data[0] >> 4) & 0x0f;
        return 1;
    }
    return -1;
}

int Iec103Parser::getTap(const uint8_t* data, uint32_t dataLen, iec103::TAP& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val = (data[1] << 8) | data[0];
        return 2;
    }
    return -1;
}

int Iec103Parser::getToo(const uint8_t* data, uint32_t dataLen, iec103::TOO& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getTov(const uint8_t* data, uint32_t dataLen, iec103::TOV& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getCP32Time2a(const uint8_t* data, uint32_t dataLen, iec103::CP32Time2a& val)
{
    if (data && dataLen >= 4) /* 4个字节 */
    {
        val.hour = data[3] & 0x1f;
        val.minute = data[2] & 0x3f;
        val.millisecond = (data[1] << 8) | data[0];
        val.summerTime = data[3] >> 7;
        return 4;
    }
    return -1;
}

int Iec103Parser::getCP56Time2a(const uint8_t* data, uint32_t dataLen, iec103::CP56Time2a& val)
{
    if (data && dataLen >= 7) /* 7个字节 */
    {
        val.year = data[6] & 0x7f;
        val.year += (val.year < 70) ? 2000 : 1900;
        val.month = data[5] & 0x0f;
        val.day = data[4] & 0x1f;
        val.wday = data[4] & 0xe0;
        val.hour = data[3] & 0x1f;
        val.minute = data[2] & 0x3f;
        val.millisecond = (data[1] << 8) | data[0];
        val.summerTime = data[3] >> 7;
        return 7;
    }
    return -1;
}

int Iec103Parser::getNGD(const uint8_t* data, uint32_t dataLen, iec103::NGD& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val.no = data[0] & 0x3f;
        val.count = (0x40 == (data[0] & 0x40)) ? 1 : 0;
        val.cont = (0x80 == (data[0] & 0x80)) ? 1 : 0;
        return 1;
    }
    return -1;
}

int Iec103Parser::getGin(const uint8_t* data, uint32_t dataLen, iec103::GIN& val)
{
    if (data && dataLen >= 2) /* 2个字节 */
    {
        val.group = data[0];
        val.entry = data[1];
        return 2;
    }
    return -1;
}

int Iec103Parser::getGdd(const uint8_t* data, uint32_t dataLen, iec103::GDD& val)
{
    if (data && dataLen >= 3) /* 3个字节 */
    {
        val.dataType = data[0];
        val.dataSize = data[1];
        val.number = data[2] & 0x7f;
        val.cont = (0x80 == (data[2] & 0x80)) ? 1 : 0;
        return 3;
    }
    return -1;
}

int Iec103Parser::getKod(const uint8_t* data, uint32_t dataLen, iec103::KOD& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getNde(const uint8_t* data, uint32_t dataLen, iec103::NDE& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val.no = data[0] & 0x3f;
        val.count = (0x40 == (data[0] & 0x40)) ? 1 : 0;
        val.cont = (0x80 == (data[0] & 0x80)) ? 1 : 0;
        return 1;
    }
    return -1;
}

int Iec103Parser::getGrc(const uint8_t* data, uint32_t dataLen, iec103::GRC& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getNog(const uint8_t* data, uint32_t dataLen, iec103::NOG& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val = data[0];
        return 1;
    }
    return -1;
}

int Iec103Parser::getDPIWithTime(const uint8_t* data, uint32_t dataLen, iec103::DPIWithTime& val)
{
    if (data && dataLen >= 6) /* 6个字节 */
    {
        uint32_t offset = 0;
        offset += getDpi(data + offset, dataLen - offset, val.dpi);
        offset += getCP32Time2a(data + offset, dataLen - offset, val.tm);
        getSin(data + offset, dataLen - offset, val.sin);
        return 6;
    }
    return -1;
}

int Iec103Parser::getDPIWithRet(const uint8_t* data, uint32_t dataLen, iec103::DPIWithRet& val)
{
    if (data && dataLen >= 10) /* 10个字节 */
    {
        uint32_t offset = 0;
        offset += getDpi(data + offset, dataLen - offset, val.dpi);
        offset += getRet(data + offset, dataLen - offset, val.ret);
        offset += getFan(data + offset, dataLen - offset, val.fan);
        offset += getCP32Time2a(data + offset, dataLen - offset, val.tm);
        getSin(data + offset, dataLen - offset, val.sin);
        return 10;
    }
    return -1;
}

int Iec103Parser::getValWithRet(const uint8_t* data, uint32_t dataLen, iec103::ValWithRet& val)
{
    if (data && dataLen >= 12) /* 12个字节 */
    {
        uint32_t offset = 0;
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val.real32Value, &temp, 4);
        offset += 4;
        offset += getRet(data + offset, dataLen - offset, val.ret);
        offset += getFan(data + offset, dataLen - offset, val.fan);
        getCP32Time2a(data + offset, dataLen - offset, val.tm);
        return 12;
    }
    return -1;
}

int Iec103Parser::getBsi(const const uint8_t* data, uint32_t dataLen, uint8_t val[4])
{
    if (data && dataLen >= 4) /* 4个字节 */
    {
        val[0] = data[3];
        val[1] = data[2];
        val[2] = data[1];
        val[3] = data[0];
        return 4;
    }
    return -1;
}

int Iec103Parser::getUint32(const const uint8_t* data, uint32_t dataLen, uint8_t valSize, uint32_t& val)
{
    if (data && dataLen > 0 && dataLen >= valSize)
    {
        val = 0;
        for (uint8_t i = 0; i < valSize; ++i)
        {
            val |= (data[i] << 8 * i);
        }
        return valSize;
    }
    return -1;
}

int Iec103Parser::getInt32(const const uint8_t* data, uint32_t dataLen, uint8_t valSize, int32_t& val)
{
    if (data && dataLen > 0 && dataLen >= valSize)
    {
        val = 0;
        uint8_t sign = (0x80 == (data[valSize - 1] & 0x80)) ? 1 : 0;
        for (uint8_t i = 0; i < 4; ++i)
        {
            val |= (data[i] << 8 * i);
        }
        if (1 == sign) /* 负数 */
        {
            if (1 == valSize)
            {
                val = (-1.0) * ((int)((~val) & 0x000000ff) + 0x01);
            }
            else if (2 == valSize)
            {
                val = (-1.0) * ((int)((~val) & 0x0000ffff) + 0x01);
            }
            else if (3 == valSize)
            {
                val = (-1.0) * ((int)((~val) & 0x00ffffff) + 0x01);
            }
        }
        return valSize;
    }
    return -1;
}

int Iec103Parser::getUfloat(const const uint8_t* data, uint32_t dataLen, uint8_t valSize, double& val)
{
    if (data && dataLen > 0 && dataLen >= valSize)
    {
        val = 0.0;
        uint32_t nValue = 0;
        uint8_t sign = (0x80 == (data[valSize - 1] & 0x80)) ? 1 : 0;
        if (1 == sign) /* 负数 */
        {
            nValue = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
            float temp = (nValue & 0x007fffff) * 1.0 / 0x800000;
            uint8_t n = ((nValue & 0xff800000) >> 23) - 127;
            val = pow(2.0, n);
            val = (1 + temp) * val;
        }
        else /* 正数 */
        {
            for (uint8_t i = 0; i < valSize; ++i)
            {
                nValue |= (data[i] << 8 * i);
            }
            float f = 0.0;
            memcpy(&f, &nValue, 4);
            val = (double)f;
        }
        return valSize;
    }
    return -1;
}

int Iec103Parser::getFloat(const const uint8_t* data, uint32_t dataLen, uint8_t valSize, float& val)
{
    if (data && dataLen > 0 && dataLen >= valSize)
    {
        val = 0.0;
        uint32_t nValue = 0;
        for (uint8_t i = 0; i < valSize; ++i)
        {
            nValue |= (data[i] << 8 * i);
        }
        memcpy(&val, &nValue, 4);
        return valSize;
    }
    return -1;
}

int Iec103Parser::getIEEE754R32(const const uint8_t* data, uint32_t dataLen, float& val)
{
    if (data && dataLen >= 4) /* 4个字节 */
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, 4);
        return 4;
    }
    return -1;
}

int Iec103Parser::getIEEE754R64(const const uint8_t* data, uint32_t dataLen, double& val)
{
    if (data && dataLen >= 8) /* 8个字节 */
    {
        memcpy(&val, data, 8);
        return 8;
    }
    return -1;
}

int Iec103Parser::getGid(const uint8_t* data, uint32_t dataLen, const iec103::GDD& gdd, iec103::GID& val)
{
    if (data && dataLen > 0)
    {
        switch (gdd.dataType)
        {
        case 1: /* ASCII字符 */
            return getAsc(data, dataLen, val.ascii);
        case 2: /* 成组8位串 */
            return getBsi(data, dataLen, val.bsi);
        case 3: /* 无符号整数 */
            return getUint32(data, dataLen, gdd.dataSize, val.uintValue);
        case 4: /* 整数 */
            return getInt32(data, dataLen, gdd.dataSize, val.intValue);
        case 5: /* 无符号浮点数 */
            return getUfloat(data, dataLen, gdd.dataSize, val.urealValue);
        case 6: /* 浮点数 */
            return getFloat(data, dataLen, gdd.dataSize, val.realValue);
        case 7: /* IEEE标准754短实数 */
            return getIEEE754R32(data, dataLen, val.real32Value);
        case 8: /* IEEE标准754实数 */
            return getIEEE754R64(data, dataLen, val.real64Value);
        case 9: /* 双点信息 */
            return getDpi(data, dataLen, val.dpi);
        case 10: /* 单点信息 */
            return getSpi(data, dataLen, val.spi);
        case 11: /* 带瞬变和差错的双点信息 */
            return getTedpi(data, dataLen, val.tedpi);
        case 12: /* 带品质描述词的被测值 */
            return getMea(data, dataLen, val.mea);
        case 14: /* 七个八位位组的二进制时间 */
            return getCP56Time2a(data, dataLen, val.tm);
        case 15: /* 通用分类标识序号 */
            return getGin(data, dataLen, val.gin);
        case 16: /* 相对时间 */
            return getRet(data, dataLen, val.ret);
        case 17: { /* 功能类型 */
            if (dataLen >= 2)
            {
                val.func = data[0];
                val.inf = data[1];
                return 2;
            }
        }
        break;
        case 18: /* 带时标的报文 */
            return getDPIWithTime(data, dataLen, val.dpiWithTime);
        case 19: /* 带相对时间的时标报文 */
            return getDPIWithRet(data, dataLen, val.dpiWithRet);
        case 20: /* 带相对时间的时标的被测值 */
            return getValWithRet(data, dataLen, val.valWithRet);
        case 22: /* 通用分类回答码 */
            return getGrc(data, dataLen, val.grc);
        case 23: /* 数据结构 */
            return getGdd(data, dataLen, val.gdd);
        default:
            break;
        }
    }
    return -1;
}

int Iec103Parser::getVti(const uint8_t* data, uint32_t dataLen, iec103::VTI& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val.type = (0x80 == (data[0] & 0x80)) ? 1 : 0;
        val.value = data[0] & 0x7f;
        return 1;
    }
    return -1;
}

int Iec103Parser::getQds(const uint8_t* data, uint32_t dataLen, iec103::QDS& val)
{
    if (data && dataLen >= 1) /* 1个字节 */
    {
        val.iv = (0x80 == (data[0] & 0x80)) ? 1 : 0;
        val.nt = (0x40 == (data[0] & 0x40)) ? 1 : 0;
        val.sb = (0x20 == (data[0] & 0x20)) ? 1 : 0;
        val.bl = (0x10 == (data[0] & 0x10)) ? 1 : 0;
        val.res = (data[0] & 0x0e) >> 1;
        val.ov = data[0] & 0x01;
        return 1;
    }
    return -1;
}
} // namespace npacket
