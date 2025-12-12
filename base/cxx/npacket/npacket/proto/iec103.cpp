#include "iec103.h"

#include <math.h>
#include <string.h>

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

ParseResult Iec103Parser::parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
{
    if (header && TransportProtocol::TCP != header->getProtocol() && TransportProtocol::UDP != header->getProtocol())
    {
        return ParseResult::FAILURE;
    }
    if (payloadLen < 5) /* IEC103包最小5个字节 */
    {
        return ParseResult::FAILURE;
    }
    if (parseFixedFrame(ntp, totalLen, header, payload, payloadLen))
    {
        return ParseResult::SUCCESS;
    }
    else if (parseVariableFrame(ntp, totalLen, header, payload, payloadLen))
    {
        return ParseResult::SUCCESS;
    }
    return ParseResult::FAILURE;
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
    case 0x02:
        return parseAsud2(identify, elements, elementLen);
    case 0x03:
        return parseAsud3(identify, elements, elementLen);
    case 0x04:
        return parseAsud4(identify, elements, elementLen);
    case 0x05:
        return parseAsud5(identify, elements, elementLen);
    case 0x06:
        return parseAsud6(identify, elements, elementLen);
    case 0x07:
        return parseAsud7(identify, elements, elementLen);
    case 0x08:
        return parseAsud8(identify, elements, elementLen);
    case 0x09:
        return parseAsud9(identify, elements, elementLen);
    case 0x0A:
        return parseAsud10(identify, elements, elementLen);
    case 0x0B:
        return parseAsud11(identify, elements, elementLen);
    case 0x14:
        return parseAsud20(identify, elements, elementLen);
    case 0x15:
        return parseAsud21(identify, elements, elementLen);
    case 0x17:
        return parseAsud23(identify, elements, elementLen);
    case 0x18:
        return parseAsud24(identify, elements, elementLen);
    case 0x19:
        return parseAsud25(identify, elements, elementLen);
    case 0x1A:
        return parseAsud26(identify, elements, elementLen);
    case 0x1B:
        return parseAsud27(identify, elements, elementLen);
    case 0x1C:
        return parseAsud28(identify, elements, elementLen);
    case 0x1D:
        return parseAsud29(identify, elements, elementLen);
    case 0x1E:
        return parseAsud30(identify, elements, elementLen);
    case 0x1F:
        return parseAsud31(identify, elements, elementLen);
    case 0x26:
        return parseAsud38(identify, elements, elementLen);
    case 0x2A:
        return parseAsud42(identify, elements, elementLen);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu1> Iec103Parser::parseAsud1(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu1>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            count = getDpi(elements + offset, elementLen - offset, asdu->dpi);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getCP32Time2a(elements + offset, elementLen - offset, asdu->tm);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getSin(elements + offset, elementLen - offset, asdu->sin);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu2> Iec103Parser::parseAsud2(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu2>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            count = getDpi(elements + offset, elementLen - offset, asdu->dpi);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getRet(elements + offset, elementLen - offset, asdu->ret);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getFan(elements + offset, elementLen - offset, asdu->fan);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getCP32Time2a(elements + offset, elementLen - offset, asdu->tm);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getSin(elements + offset, elementLen - offset, asdu->sin);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu3> Iec103Parser::parseAsud3(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu3>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        for (uint8_t i = 0; i < identify.vsq.num; ++i)
        {
            iec103::MEA mea;
            count = getMea(elements + offset, elementLen - offset, mea);
            if (count <= 0)
            {
                return nullptr;
            }
            offset += count;
            asdu->meaList.emplace_back(mea);
        }
        return asdu;
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu4> Iec103Parser::parseAsud4(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu4>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            count = getScl(elements + offset, elementLen - offset, asdu->scl);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getRet(elements + offset, elementLen - offset, asdu->ret);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getFan(elements + offset, elementLen - offset, asdu->fan);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getCP32Time2a(elements + offset, elementLen - offset, asdu->tm);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu5> Iec103Parser::parseAsud5(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu5>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            count = getCol(elements + offset, elementLen - offset, asdu->col);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            if (elementLen - offset < 12)
            {
                break;
            }
            asdu->ascii1 = elements[offset];
            asdu->ascii2 = elements[offset + 1];
            asdu->ascii3 = elements[offset + 2];
            asdu->ascii4 = elements[offset + 3];
            asdu->ascii5 = elements[offset + 4];
            asdu->ascii6 = elements[offset + 5];
            asdu->ascii7 = elements[offset + 6];
            asdu->ascii8 = elements[offset + 7];
            asdu->freeValue1 = elements[offset + 8];
            asdu->freeValue2 = elements[offset + 9];
            asdu->freeValue3 = elements[offset + 10];
            asdu->freeValue4 = elements[offset + 11];
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu6> Iec103Parser::parseAsud6(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    int32_t offset = 2;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu6>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        if (getCP56Time2a(elements + offset, elementLen - offset, asdu->tm) > 0)
        {
            return asdu;
        }
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu7> Iec103Parser::parseAsud7(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    int32_t offset = 2;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu7>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        if (getScn(elements + offset, elementLen - offset, asdu->scn) > 0)
        {
            return asdu;
        }
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu8> Iec103Parser::parseAsud8(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    int32_t offset = 2;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu8>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        if (getScn(elements + offset, elementLen - offset, asdu->scn) > 0)
        {
            return asdu;
        }
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu9> Iec103Parser::parseAsud9(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                        uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu9>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        for (uint8_t i = 0; i < identify.vsq.num; ++i)
        {
            iec103::MEA mea;
            count = getMea(elements + offset, elementLen - offset, mea);
            if (count <= 0)
            {
                return nullptr;
            }
            offset += count;
            asdu->meaList.emplace_back(mea);
        }
        return asdu;
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu10> Iec103Parser::parseAsud10(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu10>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            count = getRii(elements + offset, elementLen - offset, asdu->rii);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getNgd(elements + offset, elementLen - offset, asdu->ngd);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            for (uint8_t i = 0; i < asdu->ngd.no; ++i)
            {
                iec103::DataSet10 dataSet10;
                count = getGin(elements + offset, elementLen - offset, dataSet10.gin);
                if (count <= 0)
                {
                    return nullptr;
                }
                offset += count;
                count = getKod(elements + offset, elementLen - offset, dataSet10.kod);
                if (count <= 0)
                {
                    return nullptr;
                }
                offset += count;
                count = getGdd(elements + offset, elementLen - offset, dataSet10.gdd);
                if (count <= 0)
                {
                    return nullptr;
                }
                offset += count;
                for (uint8_t j = 0; j < dataSet10.gdd.number; ++j)
                {
                    iec103::GID gid;
                    count = getGid(elements + offset, elementLen - offset, dataSet10.gdd, gid);
                    if (count <= 0)
                    {
                        return nullptr;
                    }
                    offset += count;
                    dataSet10.gidList.emplace_back(gid);
                }
                asdu->dataSet.emplace_back(dataSet10);
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu11> Iec103Parser::parseAsud11(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu11>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            count = getRii(elements + offset, elementLen - offset, asdu->rii);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getGin(elements + offset, elementLen - offset, asdu->gin);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getNde(elements + offset, elementLen - offset, asdu->nde);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            for (uint8_t i = 0; i < asdu->nde.no; ++i)
            {
                iec103::DataSet11 dataSet11;
                count = getKod(elements + offset, elementLen - offset, dataSet11.kod);
                if (count <= 0)
                {
                    return nullptr;
                }
                offset += count;
                count = getGdd(elements + offset, elementLen - offset, dataSet11.gdd);
                if (count <= 0)
                {
                    return nullptr;
                }
                offset += count;
                for (uint8_t j = 0; j < dataSet11.gdd.number; ++j)
                {
                    iec103::GID gid;
                    count = getGid(elements + offset, elementLen - offset, dataSet11.gdd, gid);
                    if (count <= 0)
                    {
                        return nullptr;
                    }
                    offset += count;
                    dataSet11.gidList.emplace_back(gid);
                }
                asdu->dataSet.emplace_back(dataSet11);
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu20> Iec103Parser::parseAsud20(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu20>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            count = getDco(elements + offset, elementLen - offset, asdu->dco);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getRii(elements + offset, elementLen - offset, asdu->rii);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu21> Iec103Parser::parseAsud21(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu21>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            count = getRii(elements + offset, elementLen - offset, asdu->rii);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getNog(elements + offset, elementLen - offset, asdu->nog);
            if (count <= 0)
            {
                break;
            }
            for (uint8_t i = 0; i < asdu->nog; ++i)
            {
                iec103::DataSet21 dataSet21;
                count = getGin(elements + offset, elementLen - offset, dataSet21.gin);
                if (count <= 0)
                {
                    return nullptr;
                }
                offset += count;
                count = getKod(elements + offset, elementLen - offset, dataSet21.kod);
                if (count <= 0)
                {
                    return nullptr;
                }
                offset += count;
                asdu->dataSet.emplace_back(dataSet21);
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu23> Iec103Parser::parseAsud23(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu23>();
        asdu->func = elements[0];
        asdu->_ = 0;
        for (uint8_t i = 0; i < identify.vsq.num; ++i)
        {
            iec103::DataSet23 dataSet23;
            count = getFan(elements + offset, elementLen - offset, dataSet23.fan);
            if (count <= 0)
            {
                return nullptr;
            }
            offset += count;
            count = getSof(elements + offset, elementLen - offset, dataSet23.sof);
            if (count <= 0)
            {
                return nullptr;
            }
            offset += count;
            count = getCP56Time2a(elements + offset, elementLen - offset, dataSet23.tm);
            if (count <= 0)
            {
                return nullptr;
            }
            offset += count;
            asdu->dataSet.emplace_back(dataSet23);
        }
        return asdu;
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu24> Iec103Parser::parseAsud24(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu24>();
        asdu->func = elements[0];
        asdu->_ = 0;
        do
        {
            count = getToo(elements + offset, elementLen - offset, asdu->too);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getTov(elements + offset, elementLen - offset, asdu->tov);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getFan(elements + offset, elementLen - offset, asdu->fan);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getAcc(elements + offset, elementLen - offset, asdu->acc);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu25> Iec103Parser::parseAsud25(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu25>();
        asdu->func = elements[0];
        asdu->_ = 0;
        do
        {
            count = getToo(elements + offset, elementLen - offset, asdu->too);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getTov(elements + offset, elementLen - offset, asdu->tov);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getFan(elements + offset, elementLen - offset, asdu->fan);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getAcc(elements + offset, elementLen - offset, asdu->acc);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu26> Iec103Parser::parseAsud26(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 3, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu26>();
        asdu->func = elements[0];
        asdu->_ = 0;
        asdu->__ = 0;
        do
        {
            count = getTov(elements + offset, elementLen - offset, asdu->tov);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getFan(elements + offset, elementLen - offset, asdu->fan);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getNof(elements + offset, elementLen - offset, asdu->nof);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getNoc(elements + offset, elementLen - offset, asdu->noc);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getNoe(elements + offset, elementLen - offset, asdu->noe);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getInt(elements + offset, elementLen - offset, asdu->interval);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getCP32Time2a(elements + offset, elementLen - offset, asdu->tm);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu27> Iec103Parser::parseAsud27(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 3, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu27>();
        asdu->func = elements[0];
        asdu->_ = 0;
        asdu->__ = 0;
        do
        {
            count = getTov(elements + offset, elementLen - offset, asdu->tov);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getFan(elements + offset, elementLen - offset, asdu->fan);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getAcc(elements + offset, elementLen - offset, asdu->acc);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getRpv(elements + offset, elementLen - offset, asdu->rpv);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getRsv(elements + offset, elementLen - offset, asdu->rsv);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getRfa(elements + offset, elementLen - offset, asdu->rfa);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu28> Iec103Parser::parseAsud28(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 4;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu28>();
        asdu->func = elements[0];
        asdu->_ = 0;
        asdu->__ = 0;
        asdu->___ = 0;
        if (getFan(elements + offset, elementLen - offset, asdu->fan) > 0)
        {
            return asdu;
        }
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu29> Iec103Parser::parseAsud29(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu29>();
        asdu->func = elements[0];
        asdu->_ = 0;
        do
        {
            count = getFan(elements + offset, elementLen - offset, asdu->fan);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getNot(elements + offset, elementLen - offset, asdu->not_);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getTap(elements + offset, elementLen - offset, asdu->tap);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            for (uint8_t i = 0; i < asdu->not_; ++i)
            {
                iec103::DataSet29 dataSet29;
                if (elementLen - offset < 2)
                {
                    return nullptr;
                }
                dataSet29.func = elements[offset];
                dataSet29.inf = elements[offset + 1];
                offset += 2;
                count = getDpi(elements + offset, elementLen - offset, dataSet29.dpi);
                if (count <= 0)
                {
                    return nullptr;
                }
                asdu->dataSet.emplace_back(dataSet29);
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu30> Iec103Parser::parseAsud30(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 3, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu30>();
        asdu->func = elements[0];
        asdu->_ = 0;
        asdu->__ = 0;
        do
        {
            count = getTov(elements + offset, elementLen - offset, asdu->tov);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getFan(elements + offset, elementLen - offset, asdu->fan);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getAcc(elements + offset, elementLen - offset, asdu->acc);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getNdv(elements + offset, elementLen - offset, asdu->ndv);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getNfe(elements + offset, elementLen - offset, asdu->nfe);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            for (uint8_t i = 0; i < asdu->ndv; ++i)
            {
                iec103::SDV sdv;
                count = getSdv(elements + offset, elementLen - offset, sdv);
                if (count <= 0)
                {
                    return nullptr;
                }
                offset += count;
                asdu->sdvList.emplace_back(sdv);
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu31> Iec103Parser::parseAsud31(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu31>();
        asdu->func = elements[0];
        asdu->_ = 0;
        do
        {
            count = getToo(elements + offset, elementLen - offset, asdu->too);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getTov(elements + offset, elementLen - offset, asdu->tov);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getFan(elements + offset, elementLen - offset, asdu->fan);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getAcc(elements + offset, elementLen - offset, asdu->acc);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu38> Iec103Parser::parseAsud38(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu38>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            count = getVti(elements + offset, elementLen - offset, asdu->vti);
            if (count <= 0)
            {
                break;
            }
            offset += count;
            count = getQds(elements + offset, elementLen - offset, asdu->qds);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

std::shared_ptr<iec103::Asdu42> Iec103Parser::parseAsud42(const iec103::DataUnitIdentify& identify, const uint8_t* elements,
                                                          uint32_t elementLen)
{
    int32_t offset = 2, count = 0;
    if (elementLen >= offset)
    {
        auto asdu = std::make_shared<iec103::Asdu42>();
        asdu->func = elements[0];
        asdu->inf = elements[1];
        do
        {
            for (uint8_t i = 0; i < identify.vsq.num; ++i)
            {
                iec103::DPI dpi;
                count = getDpi(elements + offset, elementLen - offset, dpi);
                if (count <= 0)
                {
                    return nullptr;
                }
                offset += count;
                asdu->dpiList.emplace_back(dpi);
            }
            count = getSin(elements + offset, elementLen - offset, asdu->sin);
            if (count <= 0)
            {
                break;
            }
            return asdu;
        } while (0);
    }
    return nullptr;
}

int Iec103Parser::getAcc(const uint8_t* data, uint32_t dataLen, iec103::ACC& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getAsc(const uint8_t* data, uint32_t dataLen, iec103::ASC& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getCol(const uint8_t* data, uint32_t dataLen, iec103::COL& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getDco(const uint8_t* data, uint32_t dataLen, iec103::DCO& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0] & 0x03; /* 占2位 */
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getDpi(const uint8_t* data, uint32_t dataLen, iec103::DPI& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0] & 0x03; /* 占2位 */
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getSpi(const uint8_t* data, uint32_t dataLen, iec103::SPI& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0] & 0x01; /* 占1位 */
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getTedpi(const uint8_t* data, uint32_t dataLen, iec103::TEDPI& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0] & 0x03; /* 占2位 */
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getFan(const uint8_t* data, uint32_t dataLen, iec103::FAN& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = (data[1] << 8) | data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getInt(const uint8_t* data, uint32_t dataLen, iec103::INT& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = (data[1] << 8) | data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getMea(const uint8_t* data, uint32_t dataLen, iec103::MEA& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val.ov = (0x01 == (data[0] & 0x01)) ? 1 : 0; /* 第1位 */
        val.er = (0x02 == (data[0] & 0x02)) ? 1 : 0; /* 第2位 */
        val.res = (0x04 == (data[0] & 0x04)) ? 1 : 0; /* 第3位 */
        uint8_t sign = (0x80 == (data[1] & 0x80)) ? 1 : 0; /* 符号位 */
        if (1 == sign) /* 负数 */
        {
            val.mval = -1.0 * (float)((~((data[1] << 8) | (data[0] & 0xf8)) & 0xfff8) + 0x0008) / 0x8000;
        }
        else if (0 == sign) /* 正数 */
        {
            val.mval = (float)((data[1] << 8) | (data[0] & 0xf8)) / 0x8000;
        }
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getNfe(const uint8_t* data, uint32_t dataLen, iec103::NFE& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = (data[1] << 8) | data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getNoc(const uint8_t* data, uint32_t dataLen, iec103::NOC& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getNoe(const uint8_t* data, uint32_t dataLen, iec103::NOE& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = (data[1] << 8) | data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getNof(const uint8_t* data, uint32_t dataLen, iec103::NOF& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = (data[1] << 8) | data[0];
        return BYTE_NUM;
    }
    return -1;
}
int Iec103Parser::getNot(const uint8_t* data, uint32_t dataLen, iec103::NOT& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getNdv(const uint8_t* data, uint32_t dataLen, iec103::NDV& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getRet(const uint8_t* data, uint32_t dataLen, iec103::RET& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = (data[1] << 8) | data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getRfa(const uint8_t* data, uint32_t dataLen, iec103::RFA& val)
{
    const int BYTE_NUM = 4; /* 4个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, BYTE_NUM);
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getRpv(const uint8_t* data, uint32_t dataLen, iec103::RPV& val)
{
    const int BYTE_NUM = 4; /* 4个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, BYTE_NUM);
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getRsv(const uint8_t* data, uint32_t dataLen, iec103::RSV& val)
{
    const int BYTE_NUM = 4; /* 4个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, BYTE_NUM);
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getRii(const uint8_t* data, uint32_t dataLen, iec103::RII& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getScl(const uint8_t* data, uint32_t dataLen, iec103::SCL& val)
{
    const int BYTE_NUM = 4; /* 4个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, BYTE_NUM);
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getScn(const uint8_t* data, uint32_t dataLen, iec103::SCN& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getSdv(const uint8_t* data, uint32_t dataLen, iec103::SDV& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        uint8_t sign = (0x80 == (data[1] & 0x80)) ? 1 : 0; /* 符号位 */
        if (1 == sign) /* 负数 */
        {
            val = -1.0 * (float)((~((data[1] << 8) | data[0]) & 0xffff) + 0x0001) / 0x8000;
        }
        else if (0 == sign) /* 正数 */
        {
            val = (float)((data[1] << 8) | data[0]) / 0x8000;
        }
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getSin(const uint8_t* data, uint32_t dataLen, iec103::SIN& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getSof(const uint8_t* data, uint32_t dataLen, iec103::SOF& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val.tp = (0x01 == (data[0] & 0x01)) ? 1 : 0;
        val.tm = (0x02 == (data[0] & 0x02)) ? 1 : 0;
        val.test = (0x04 == (data[0] & 0x04)) ? 1 : 0;
        val.otev = (0x08 == (data[0] & 0x08)) ? 1 : 0;
        val.res = (data[0] >> 4) & 0x0f;
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getTap(const uint8_t* data, uint32_t dataLen, iec103::TAP& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = (data[1] << 8) | data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getToo(const uint8_t* data, uint32_t dataLen, iec103::TOO& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getTov(const uint8_t* data, uint32_t dataLen, iec103::TOV& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getCP32Time2a(const uint8_t* data, uint32_t dataLen, iec103::CP32Time2a& val)
{
    const int BYTE_NUM = 4; /* 4个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val.hour = data[3] & 0x1f;
        val.minute = data[2] & 0x3f;
        val.millisecond = (data[1] << 8) | data[0];
        val.summerTime = data[3] >> 7;
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getCP56Time2a(const uint8_t* data, uint32_t dataLen, iec103::CP56Time2a& val)
{
    const int BYTE_NUM = 7; /* 7个字节 */
    if (data && dataLen >= BYTE_NUM)
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
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getNgd(const uint8_t* data, uint32_t dataLen, iec103::NGD& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val.no = data[0] & 0x3f;
        val.count = (0x40 == (data[0] & 0x40)) ? 1 : 0;
        val.cont = (0x80 == (data[0] & 0x80)) ? 1 : 0;
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getGin(const uint8_t* data, uint32_t dataLen, iec103::GIN& val)
{
    const int BYTE_NUM = 2; /* 2个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val.group = data[0];
        val.entry = data[1];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getGdd(const uint8_t* data, uint32_t dataLen, iec103::GDD& val)
{
    const int BYTE_NUM = 3; /* 3个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val.dataType = data[0];
        val.dataSize = data[1];
        val.number = data[2] & 0x7f;
        val.cont = (0x80 == (data[2] & 0x80)) ? 1 : 0;
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getKod(const uint8_t* data, uint32_t dataLen, iec103::KOD& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getNde(const uint8_t* data, uint32_t dataLen, iec103::NDE& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val.no = data[0] & 0x3f;
        val.count = (0x40 == (data[0] & 0x40)) ? 1 : 0;
        val.cont = (0x80 == (data[0] & 0x80)) ? 1 : 0;
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getGrc(const uint8_t* data, uint32_t dataLen, iec103::GRC& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getNog(const uint8_t* data, uint32_t dataLen, iec103::NOG& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val = data[0];
        return dataLen;
    }
    return -1;
}

int Iec103Parser::getDPIWithTime(const uint8_t* data, uint32_t dataLen, iec103::DPIWithTime& val)
{
    const int BYTE_NUM = 6; /* 6个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        uint32_t offset = 0;
        offset += getDpi(data + offset, dataLen - offset, val.dpi);
        offset += getCP32Time2a(data + offset, dataLen - offset, val.tm);
        getSin(data + offset, dataLen - offset, val.sin);
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getDPIWithRet(const uint8_t* data, uint32_t dataLen, iec103::DPIWithRet& val)
{
    const int BYTE_NUM = 10; /* 10个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        uint32_t offset = 0;
        offset += getDpi(data + offset, dataLen - offset, val.dpi);
        offset += getRet(data + offset, dataLen - offset, val.ret);
        offset += getFan(data + offset, dataLen - offset, val.fan);
        offset += getCP32Time2a(data + offset, dataLen - offset, val.tm);
        getSin(data + offset, dataLen - offset, val.sin);
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getValWithRet(const uint8_t* data, uint32_t dataLen, iec103::ValWithRet& val)
{
    const int BYTE_NUM = 12; /* 12个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        uint32_t offset = 0;
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val.real32Value, &temp, 4);
        offset += 4;
        offset += getRet(data + offset, dataLen - offset, val.ret);
        offset += getFan(data + offset, dataLen - offset, val.fan);
        getCP32Time2a(data + offset, dataLen - offset, val.tm);
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getBsi(const uint8_t* data, uint32_t dataLen, uint8_t val[4])
{
    const int BYTE_NUM = 4; /* 4个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val[0] = data[3];
        val[1] = data[2];
        val[2] = data[1];
        val[3] = data[0];
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getUint32(const uint8_t* data, uint32_t dataLen, uint8_t valSize, uint32_t& val)
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

int Iec103Parser::getInt32(const uint8_t* data, uint32_t dataLen, uint8_t valSize, int32_t& val)
{
    if (data && dataLen > 0 && dataLen >= valSize)
    {
        val = 0;
        uint8_t sign = (0x80 == (data[valSize - 1] & 0x80)) ? 1 : 0; /* 符号位 */
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

int Iec103Parser::getUfloat(const uint8_t* data, uint32_t dataLen, uint8_t valSize, double& val)
{
    if (data && dataLen > 0 && dataLen >= valSize)
    {
        val = 0.0;
        uint32_t nValue = 0;
        uint8_t sign = (0x80 == (data[valSize - 1] & 0x80)) ? 1 : 0; /* 符号位 */
        if (1 == sign) /* 负数 */
        {
            nValue = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
            float temp = (nValue & 0x007fffff) * 1.0 / 0x800000;
            uint8_t n = ((nValue & 0xff800000) >> 23) - 127;
            val = pow(2.0, n);
            val = ((double)1 + temp) * val;
        }
        else /* 正数 */
        {
            for (uint8_t i = 0; i < valSize; ++i)
            {
                nValue |= (data[i] << 8 * i);
            }
            float f = 0.0;
            memcpy(&f, &nValue, valSize);
            val = (double)f;
        }
        return valSize;
    }
    return -1;
}

int Iec103Parser::getFloat(const uint8_t* data, uint32_t dataLen, uint8_t valSize, float& val)
{
    if (data && dataLen > 0 && dataLen >= valSize)
    {
        val = 0.0;
        uint32_t nValue = 0;
        for (uint8_t i = 0; i < valSize; ++i)
        {
            nValue |= (data[i] << 8 * i);
        }
        memcpy(&val, &nValue, valSize);
        return valSize;
    }
    return -1;
}

int Iec103Parser::getIEEE754R32(const uint8_t* data, uint32_t dataLen, float& val)
{
    const int BYTE_NUM = 4; /* 4个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        int temp = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
        memcpy(&val, &temp, BYTE_NUM);
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getIEEE754R64(const uint8_t* data, uint32_t dataLen, double& val)
{
    const int BYTE_NUM = 8; /* 8个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        memcpy(&val, data, BYTE_NUM);
        return BYTE_NUM;
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
        return gdd.dataSize;
    }
    return -1;
}

int Iec103Parser::getVti(const uint8_t* data, uint32_t dataLen, iec103::VTI& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val.type = (0x80 == (data[0] & 0x80)) ? 1 : 0;
        val.value = data[0] & 0x7f;
        return BYTE_NUM;
    }
    return -1;
}

int Iec103Parser::getQds(const uint8_t* data, uint32_t dataLen, iec103::QDS& val)
{
    const int BYTE_NUM = 1; /* 1个字节 */
    if (data && dataLen >= BYTE_NUM)
    {
        val.iv = (0x80 == (data[0] & 0x80)) ? 1 : 0;
        val.nt = (0x40 == (data[0] & 0x40)) ? 1 : 0;
        val.sb = (0x20 == (data[0] & 0x20)) ? 1 : 0;
        val.bl = (0x10 == (data[0] & 0x10)) ? 1 : 0;
        val.res = (data[0] & 0x0e) >> 1;
        val.ov = data[0] & 0x01;
        return BYTE_NUM;
    }
    return -1;
}
} // namespace npacket
