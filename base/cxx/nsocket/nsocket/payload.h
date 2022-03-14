#pragma once
#include <functional>
#include <vector>

namespace nsocket
{
/**
 * @brief 负载
 */
class Payload final
{
public:
    /**
     * @brief 包体回调函数
     * @param body 包体内容 
     */
    using BODY_CALLBACK = std::function<void(const std::vector<unsigned char>& body)>;

    /**
     * @brief 解析出错回调函数
     * @param data 数据 
     */
    using ERROR_CALLBACK = std::function<void(const std::vector<unsigned char>& data)>;

public:
    /**
     * @brief 构造函数
     * @param maxBodyLen 允许的最大包体长度
     * @param bigEndium 非包体字节序是否大端模式(选填), 默认大端
     */
    Payload(unsigned int maxBodyLen, bool bigEndium = true);

    /**
     * @brief 重置
     */
    void reset();

    /**
     * @brief 对数据进行拼接/拆包
     * @param data 数据
     * @param bodyCb 包体回调
     * @param errorCb 解析出错回调
     */
    void unpack(const std::vector<unsigned char>& data, const BODY_CALLBACK& bodyCb, const ERROR_CALLBACK& errorCb);

    /**
     * @brief 对数据进行组包
     * @param body 包体
     * @param bodyLen 包体长度
     * @param data [输出]包数据 = 包头 + 包体
     * @param bigEndium 非包体字节序是否大端模式(选填), 默认大端
     */
    static void pack(const unsigned char* body, unsigned int bodyLen, std::vector<unsigned char>& data, bool bigEndium = true);

    /**
     * @brief 对数据进行组包
     * @param body 包体
     * @param data [输出]包数据 = 包头 + 包体
     * @param bigEndium 非包体字节序是否大端模式(选填), 默认大端
     */
    static void pack(const std::vector<unsigned char>& body, std::vector<unsigned char>& data, bool bigEndium = true);

private:
    /* 解析步骤 */
    enum class ParseStep
    {
        head = 0, /* 解析包头 */
        body /* 解析包体 */
    };

private:
    std::vector<unsigned char> m_recvBuffer; /* 接收缓冲区 */
    unsigned int m_bodyMaxLen; /* 允许的包体最大长度 */
    ParseStep m_parseStep = ParseStep::head; /* 解析步骤 */
    bool m_bigEndium = true; /* 非包体字节序是否大端模式 */
    unsigned int m_head = 0; /* 包头(占4个字节, 用于存放包体长度) */
    std::vector<unsigned char> m_body; /* 已接收的包体内容 */
};
} // namespace nsocket
