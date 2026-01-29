#pragma once
#include <functional>
#include <vector>

#include "../protocol_parser.h"
#include "iec103_def.h"

namespace npacket
{
/**
 * 类型标识(主 -> 从):                         类型标识(从 -> 主):
 * -----------------------------               ---------------------------------------
 * | 类型 | 说明               |               | 类型 | 说明                         |
 * ----------------------------|               --------------------------------------|
 * | 6    | 时间同步           | C_SYN_TA_3    | 1    | 带时标的报文                 | M_TM_TA_3
 * | 7    | 启动总查询(总召唤) | C_IGI_NA_3    | 2    | 具有相对时间的带时标的报文   | M_TMR_TA_3
 * | 10   | 通用分类数据       | C_GD_NA_3     | 3    | 被测值I                      | M_MEI_NA_3
 * | 20   | 一般命令           | C_GRC_NA_3    | 4    | 具有相对时间的带时标的被测值 | M_TME_TA_3
 * | 21   | 通用分类命令       | C_GC_NA_3     | 5    | 标识                         | M_IRC_NA_3, M_IRF_NA_3, M_IRS_NA_3
 * | 24   | 扰动数据传输的命令 | C_ODT_NA_3    | 6    | 时钟同步                     | M_SYN_TA_3
 * | 25   | 扰动数据传输的认可 | C_ADT_NA_3    | 8    | 总查询(总召唤)结束           | M_TGI_NA_3
 * -----------------------------               | 9    | 被测值II                     | M_MEII_NA_3
 *                                             | 10   | 通用分类数据                 | M_GD_N(T)A_3
 *                                             | 11   | 通用分类标识                 | M_GI_N(T)A_3
 *                                             | 23   | 被记录的扰动表               | M_LRD_TA_3
 *                                             | 26   | 扰动数据传输准备就绪         | M_RTD_TA_3
 *                                             | 27   | 被记录的通道传输准备就绪     | M_RTC_NA_3
 *                                             | 28   | 带标志的状态变位传输准备就绪 | M_RTT_NA_3
 *                                             | 29   | 传送带标志的状态变位         | M_TDT_NA_3
 *                                             | 30   | 传送扰动值                   | M_TDN_NA_3
 *                                             | 31   | 传送结束                     | M_EOT_NA_3
 *                                             ---------------------------------------
 * 传送原因(主 -> 从):                传送原因(从 -> 主):
 * -------------------------------    ---------------------------------------
 * | 原因 | 说明                 |    | 原因 | 说明                         |
 * |------|----------------------|    |------|------------------------------|
 * | 8    | 时间同步             |    | 1    | 自发(突发)                   |
 * | 9    | 总查询(总召唤)的启动 |    | 2    | 循环                         |
 * | 20   | 一般命令             |    | 3    | 复位帧计数位(FCB)            |
 * | 31   | 扰动数据的传输       |    | 4    | 复位通信单元(CU)             |
 * | 40   | 通用分类写命令       |    | 5    | 启动/重新启动                |
 * | 42   | 通用分类读命令       |    | 6    | 电源合上                     |
 * -------------------------------    | 7    | 测试模式                     |
 *                                    | 8    | 时间同步                     |
 *                                    | 9    | 总查询(总召唤)               |
 *                                    | 10   | 总查询(总召唤)终止           |
 *                                    | 11   | 当地操作                     |
 *                                    | 12   | 远方操作                     |
 *                                    | 20   | 命令的肯定认可               |
 *                                    | 21   | 命令的否定认可               |
 *                                    | 31   | 扰动数据的传送               |
 *                                    | 40   | 通用分类写命令的肯定认可     |
 *                                    | 41   | 通用分类写命令的否定认可     |
 *                                    | 42   | 对通用分类读命令有效数据响应 |
 *                                    | 43   | 对通用分类读命令无效数据响应 |
 *                                    | 44   | 通用分类写确认               |
 *                                    ---------------------------------------    
 */

/**
 * @brief IEC103协议解析器
 */
class Iec103Parser : public ProtocolParser
{
public:
    /**
     * @brief 固定帧回调
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部(当数据走的是非网络时, 为空)
     * @param frame 固定帧数据
     */
    using FIXED_FRAME_CALLBACK =
        std::function<void(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                           const std::shared_ptr<ProtocolHeader>& header, const std::shared_ptr<iec103::FixedFrame>& frame)>;

    /**
     * @brief 可变帧回调
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部(当数据走的是非网络时, 为空)
     * @param frame 可变帧数据
     */
    using VARIABLE_FRAME_CALLBACK =
        std::function<void(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                           const std::shared_ptr<ProtocolHeader>& header, const std::shared_ptr<iec103::VariableFrame>& frame)>;

public:
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
     * @param consumeLen [输出]消耗的长度
     * @return 解析结果
     */
    ParseResult parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header,
                      const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen) override;

    /**
     * @brief 设置固定帧回调
     * @param callback 回调
     */
    void setFixedFrameCallback(const FIXED_FRAME_CALLBACK& callback);

    /**
     * @brief 设置可变帧回调
     * @param callback 回调
     */
    void setVariableFrameCallback(const VARIABLE_FRAME_CALLBACK& callback);

private:
    /**
     * @brief 解析固定帧
     */
    bool parseFixedFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header,
                         const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen);

    /**
     * @brief 解析可变帧
     */
    bool parseVariableFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                            const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen,
                            uint32_t& consumeLen);

    /**
     * @brief 解析应用服务数据单元
     */
    std::shared_ptr<iec103::Asdu> parseAsdu(const uint8_t* data, uint32_t dataLen);

    /**
     * @brief 解析数据单元标识
     */
    void parseDataUnitIdentify(const uint8_t ch[4], iec103::DataUnitIdentify& identify);

    /**
     * @brief 解析信息元素集
     */
    std::shared_ptr<iec103::Asdu> parseInfoSet(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);

    std::shared_ptr<iec103::Asdu1> parseAsud1(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu2> parseAsud2(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu3> parseAsud3(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu4> parseAsud4(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu5> parseAsud5(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu6> parseAsud6(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu7> parseAsud7(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu8> parseAsud8(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu9> parseAsud9(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu10> parseAsud10(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu11> parseAsud11(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu20> parseAsud20(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu21> parseAsud21(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu23> parseAsud23(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu24> parseAsud24(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu25> parseAsud25(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu26> parseAsud26(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu27> parseAsud27(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu28> parseAsud28(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu29> parseAsud29(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu30> parseAsud30(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu31> parseAsud31(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu38> parseAsud38(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);
    std::shared_ptr<iec103::Asdu42> parseAsud42(const iec103::DataUnitIdentify& identify, const uint8_t* elements, uint32_t elementLen);

    int getAcc(const uint8_t* data, uint32_t dataLen, iec103::ACC& val);
    int getAsc(const uint8_t* data, uint32_t dataLen, iec103::ASC& val);
    int getCol(const uint8_t* data, uint32_t dataLen, iec103::COL& val);
    int getDco(const uint8_t* data, uint32_t dataLen, iec103::DCO& val);
    int getDpi(const uint8_t* data, uint32_t dataLen, iec103::DPI& val);
    int getSpi(const uint8_t* data, uint32_t dataLen, iec103::SPI& val);
    int getTedpi(const uint8_t* data, uint32_t dataLen, iec103::TEDPI& val);
    int getFan(const uint8_t* data, uint32_t dataLen, iec103::FAN& val);
    int getInt(const uint8_t* data, uint32_t dataLen, iec103::INT& val);
    int getMea(const uint8_t* data, uint32_t dataLen, iec103::MEA& val);
    int getNfe(const uint8_t* data, uint32_t dataLen, iec103::NFE& val);
    int getNoc(const uint8_t* data, uint32_t dataLen, iec103::NOC& val);
    int getNoe(const uint8_t* data, uint32_t dataLen, iec103::NOE& val);
    int getNof(const uint8_t* data, uint32_t dataLen, iec103::NOF& val);
    int getNot(const uint8_t* data, uint32_t dataLen, iec103::NOT& val);
    int getNdv(const uint8_t* data, uint32_t dataLen, iec103::NDV& val);
    int getRet(const uint8_t* data, uint32_t dataLen, iec103::RET& val);
    int getRfa(const uint8_t* data, uint32_t dataLen, iec103::RFA& val);
    int getRpv(const uint8_t* data, uint32_t dataLen, iec103::RPV& val);
    int getRsv(const uint8_t* data, uint32_t dataLen, iec103::RSV& val);
    int getRii(const uint8_t* data, uint32_t dataLen, iec103::RII& val);
    int getScl(const uint8_t* data, uint32_t dataLen, iec103::SCL& val);
    int getScn(const uint8_t* data, uint32_t dataLen, iec103::SCN& val);
    int getSdv(const uint8_t* data, uint32_t dataLen, iec103::SDV& val);
    int getSin(const uint8_t* data, uint32_t dataLen, iec103::SIN& val);
    int getSof(const uint8_t* data, uint32_t dataLen, iec103::SOF& val);
    int getTap(const uint8_t* data, uint32_t dataLen, iec103::TAP& val);
    int getToo(const uint8_t* data, uint32_t dataLen, iec103::TOO& val);
    int getTov(const uint8_t* data, uint32_t dataLen, iec103::TOV& val);
    int getCP32Time2a(const uint8_t* data, uint32_t dataLen, iec103::CP32Time2a& val);
    int getCP56Time2a(const uint8_t* data, uint32_t dataLen, iec103::CP56Time2a& val);
    int getNgd(const uint8_t* data, uint32_t dataLen, iec103::NGD& val);
    int getGin(const uint8_t* data, uint32_t dataLen, iec103::GIN& val);
    int getGdd(const uint8_t* data, uint32_t dataLen, iec103::GDD& val);
    int getKod(const uint8_t* data, uint32_t dataLen, iec103::KOD& val);
    int getNde(const uint8_t* data, uint32_t dataLen, iec103::NDE& val);
    int getGrc(const uint8_t* data, uint32_t dataLen, iec103::GRC& val);
    int getNog(const uint8_t* data, uint32_t dataLen, iec103::NOG& val);
    int getDPIWithTime(const uint8_t* data, uint32_t dataLen, iec103::DPIWithTime& val);
    int getDPIWithRet(const uint8_t* data, uint32_t dataLen, iec103::DPIWithRet& val);
    int getValWithRet(const uint8_t* data, uint32_t dataLen, iec103::ValWithRet& val);
    int getBsi(const uint8_t* data, uint32_t dataLen, uint8_t val[4]);
    int getUint32(const uint8_t* data, uint32_t dataLen, uint8_t valSize, uint32_t& val);
    int getInt32(const uint8_t* data, uint32_t dataLen, uint8_t valSize, int32_t& val);
    int getUfloat(const uint8_t* data, uint32_t dataLen, uint8_t valSize, double& val);
    int getFloat(const uint8_t* data, uint32_t dataLen, uint8_t valSize, float& val);
    int getIEEE754R32(const uint8_t* data, uint32_t dataLen, float& val);
    int getIEEE754R64(const uint8_t* data, uint32_t dataLen, double& val);
    int getGid(const uint8_t* data, uint32_t dataLen, const iec103::GDD& gdd, iec103::GID& val);
    int getVti(const uint8_t* data, uint32_t dataLen, iec103::VTI& val);
    int getQds(const uint8_t* data, uint32_t dataLen, iec103::QDS& val);

private:
    FIXED_FRAME_CALLBACK m_fixedFrameCb = nullptr; /* 固定帧回调 */
    VARIABLE_FRAME_CALLBACK m_variableFrameCb = nullptr; /* 可变帧回调 */
}; // namespace npacket
} // namespace npacket
