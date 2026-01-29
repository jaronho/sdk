#pragma once
#include <memory>

#include "../protocol_parser.h"

namespace npacket
{
/**
 * @brief COTP协议解析器
 */
class CotpParser : public ProtocolParser
{
public:
    /**
     * @brief TPKT信息(4个字节), 作用是包含用户协议(5~7层)的数据长度
     */
    struct TpktInfo
    {
        uint8_t version = 0; /* TPKT版本号 */
        uint8_t reserverd = 0; /* 保留 */
        uint16_t length = 0; /* 长度(TCP的payload的长度) */
    };

    /**
     * @brief PDU标识类型
     */
    enum class PduType
    {
        ED = 0x01, /* 加急数据(Expedited Data) */
        EA = 0x02, /* 加急数据确认(Expedited Data Acknowledgement) */
        UD = 0x04, /* 用户数据(User Data) */
        RJ = 0x05, /* 拒绝(Reject) */
        AK = 0x06, /* 数据确认(Data Acknowledgement) */
        ER = 0x07, /* TPDU错误(TPDU Error) */
        DR = 0x08, /* 断开请求(Disconnect Request) */
        DC = 0x0C, /* 断开确认(Disconnect Confirm) */
        CC = 0x0D, /* 连接确认(Connect Confirm) */
        CR = 0x0E, /* 连接请求(Connect Request) */
        DT = 0x0F, /* 数据传输(Data) */
    };

    /**
     * @brief COTP信息, 作用是定义了数据传输的基本单位
     */
    struct CotpInfo
    {
        uint8_t length = 0; /* COTP后续数据的长度(注意: 长度不包含当前字段所占字节), 值为: 2或17 */
        PduType pduType; /* PDU标识类型 */
    };

    /**
     * @brief COTP信息(3个字节)
     */
    struct CotpInfo3 : public CotpInfo
    {
        uint16_t tpduNumber = 0; /* 传输协议数据单元 */
        uint8_t lastDataUnit = 0; /* 是否最后一个数据单元, 0-No, 1-Yes */
    };

    /**
     * @brief COTP信息(18个字节)
     */
    struct CotpInfo18 : public CotpInfo
    {
        uint16_t dstRef = 0; /* 目标的引用 */
        uint16_t srcRef = 0; /* 源的引用 */
        uint8_t nClass = 0;
        uint8_t extendedFormats = 0; /* 是否使用拓展样式, 0-False, 1-True */
        uint8_t noExplicitFlowControl = 0; /* 是否有明确的指定流控制, 0-False, 1-True */
        uint8_t parameterCode = 0;
        uint8_t parameterLength = 0;
    };

    /**
     * @brief S7COMM消息类型
     */
    enum class RosctrType
    {
        JOB = 0x01, /* 作业请求(job with acknowledgement), 由主设备发送的请求(例如: 读/写存储器, 读/写块, 启动/停止设备, 设置通信) */
        ACK = 0x02, /* 确认响应(acknowledgement without additional field), 没有数据的简单确认 */
        ACK_DATA = 0x03, /* 确认数据响应(acknowledgement with additional field), 一般都是响应JOB的请求 */
        USERDATA = 0x07, /* 原始协议的扩展, 参数字段包含请求/响应ID(用于编程/调试, 读取SZL, 安全功能, 时间设置, 循环读取 等) */
    };

    /**
     * @brief S7COMM信息(西门子专有协议)
     */
    struct S7CommInfo
    {
        struct Header
        {
            uint8_t protocolId = 0; /* 协议ID */
            RosctrType rosctr; /* 消息类型 */
            uint16_t redundancyIdentification = 0; /* 冗余数据, 通常为: 0x0000 */
            uint16_t protocolDataUnitReference = 0; /* 协议数据单元参考, 通过请求事件增加 */
            uint16_t parameterLength = 0; /* 参数的总长度 */
            uint16_t dataLength = 0; /* 数据长度, 如果读取PLC内部数据, 此处为0x0000, 对于其他功能, 则为Data部分的数据长度 */
            uint8_t errorClass = 0; /* 错误类型(http://gmiru.com/resources/s7proto/constants.txt) */
            uint8_t errorCode = 0; /* 错误代码(http://gmiru.com/resources/s7proto/constants.txt) */
        };

        struct Parameter
        {
        };

        struct Data
        {
        };

        Header header;
        Parameter param;
        Data data;
    };

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
    ParseResult parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                      const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen) override;

private:
    /**
     * @param 解析S7COMM 
     */
    bool parseS7Comm(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                     const TpktInfo& tpktInfo, const std::shared_ptr<CotpInfo>& cotpInfo, const uint8_t* payload, uint32_t payloadLen);
};
} // namespace npacket
