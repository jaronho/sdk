#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "status_code.h"

namespace nsocket
{
namespace ws
{
/**
 * @brief WebSocket数据帧
 */
class Frame
{
public:
    /**
     * @brief 帧头回调
     */
    using HEAD_CALLBACK = std::function<void()>;

    /**
     * @brief 帧负载回调
     * @param offset 当前分段数据的偏移值
     * @param data 分段数据
     * @param dataLen 分段数据长度 
     */
    using PAYLOAD_CALLBACK = std::function<void(size_t offset, const unsigned char* data, int dataLen)>;

    /**
     * @brief 结束回调
     */
    using FINISH_CALLBACK = std::function<void()>;

public:
    /**
     * @brief 解析
     * @param data 收到的数据
     * @param length 数据长度
     * @param headCb 头部回调
     * @param payloadCb 负载回调
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parse(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const PAYLOAD_CALLBACK& payloadCb,
              const FINISH_CALLBACK& finishCb);

    /**
     * @brief 创建文本帧
     * @param data [输出]帧数据
     * @param text 文本内容
     * @param isClient 是否为客户端帧数据
     * @param isFin 是否最后一个帧
     */
    static void createTextFrame(std::vector<unsigned char>& data, const std::string& text, bool isClient = false, bool isFin = true);

    /**
     * @brief 创建二进制帧
     * @param data [输出]帧数据
     * @param bytes 帧数据(字节流)
     * @param isClient 是否为客户端帧数据
     * @param isFin 是否最后一个帧
     */
    static void createBinaryFrame(std::vector<unsigned char>& data, const std::vector<unsigned char>& bytes, bool isClient = false,
                                  bool isFin = true);

    /**
     * @brief 创建关闭帧
     * @param data [输出]帧数据
     * @param code 关闭状态码
     * @param isClient 是否为客户端帧数据
     */
    static void createCloseFrame(std::vector<unsigned char>& data, const CloseCode& code = CloseCode::close_normal, bool isClient = false);

    /**
     * @brief 创建ping帧
     * @param data [输出]帧数据
     * @param isClient 是否为客户端帧数据
     */
    static void createPingFrame(std::vector<unsigned char>& data, bool isClient = false);

    /**
     * @brief 创建pong帧
     * @param data [输出]帧数据
     * @param isClient 是否为客户端帧数据
     */
    static void createPongFrame(std::vector<unsigned char>& data, bool isClient = false);

public:
    unsigned char fin; /* 是否为最后一帧数据, 0: 后面还有数据, 1: 后面没有数据了 */
    unsigned char rsv[3] = {0}; /* 保留 */
    unsigned char opcode; /* 消息类型, 0x0: 表示附加数据帧
                                       0x1: 表示文本数据帧
                                       0x2: 表示二进制数据帧
                                       0x3-7: 暂时无定义, 为以后的非控制帧保留
                                       0x8: 表示连接关闭(控制帧, 不允许分片)
                                       0x9: 表示ping(控制帧, 不允许分片)
                                       0xA: 表示pong(控制帧, 不允许分片)
                                       0xB-F: 暂时无定义, 为以后的控制帧保留 */
    unsigned char mask = 0; /* 是否经过掩码处理, 0: 没处理过, 1: 处理过
                               客户端发送过来的必须为1, 服务端发送出去的必须为0, 否则接收方需要断开连接 */
    uint32_t payloadLen = 0; /* 负载长度 */
    unsigned char maskingKey[4] = {0}; /* 掩码 */

private:
    /**
     * @brief 创建帧数据
     * @param data [输出]帧数据
     */
    void create(std::vector<unsigned char>& data);

    /**
     * @brief 重置
     */
    void reset();

    /**
     * @brief 解析FIN + RSV + OPCODE
     * @param data 数据
     * @param length 数据长度
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseFinRsvOpcode(const unsigned char* data, int length);

    /**
     * @brief 解析MASK + PAYLOAD_LEN
     * @param data 数据
     * @param length 数据长度
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseMaskPayloadLen(const unsigned char* data, int length, const FINISH_CALLBACK& finishCb);

    /**
     * @brief 解析扩展负载字节
     * @param data 数据
     * @param length 数据长度
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parsePayloadLen(const unsigned char* data, int length, int needByteCount);

    /**
     * @brief 解析MASKING_KEY
     * @param data 数据
     * @param length 数据长度
     * @param headCb 帧头回调
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseMaskingKey(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const FINISH_CALLBACK& finishCb);

    /**
     * @brief 解析负载
     * @param data 数据
     * @param length 数据长度
     * @param payloadCb 负载回调
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parsePayload(const unsigned char* data, int length, const PAYLOAD_CALLBACK& payloadCb, const FINISH_CALLBACK& finishCb);

private:
    /**
     * @brief 解析步骤
     */
    enum class ParseStep
    {
        fin_rsv_opcode, /* FIN(是否为最后一帧数据) + RSV(保留) + OPCODE(消息类型) */
        mask_payload_len, /* MASK(是否经过掩码处理) + PAYLOAD_LEN(数据长度) */
        payload_len_2, /* 扩展数据长度 */
        payload_len_8, /* 扩展数据长度 */
        masking_key, /* 掩码 */
        payload /* 负载数据 */
    };

    ParseStep m_parseStep = ParseStep::fin_rsv_opcode; /* 解析步骤 */
    uint32_t m_payloadReceived = 0; /* 已接收负载长度 */
    std::vector<unsigned char> m_tmpBytes; /* 临时字节 */
};
} // namespace ws
} // namespace nsocket
