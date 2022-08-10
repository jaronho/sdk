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
     * @brief 包头回调函数
     * @param head 包头 
     * @return 包体长度, 小于0表示解析出错
     */
    using HEAD_CALLBACK = std::function<int(const std::vector<unsigned char>& head)>;

    /**
     * @brief 包体回调函数
     * @param body 包体 
     */
    using BODY_CALLBACK = std::function<void(const std::vector<unsigned char>& body)>;

public:
    /**
     * @brief 构造函数
     * @param headLen 包头长度
     */
    Payload(unsigned int headLen);

    /**
     * @brief 获取包头长度
     * @return 包头长度
     */
    unsigned int getHeadLen() const;

    /**
     * @brief 重置缓冲区
     */
    void reset();

    /**
     * @brief 对数据进行拼接/拆包
     * @param data 数据
     * @param headCb 包头回调
     * @param bodyCb 包体回调
     */
    void unpack(const std::vector<unsigned char>& data, const HEAD_CALLBACK& headCb, const BODY_CALLBACK& bodyCb);

private:
    /* 解析步骤 */
    enum class ParseStep
    {
        head = 0, /* 解析包头 */
        body = 1 /* 解析包体 */
    };

private:
    unsigned int m_headLen = 0; /* 包头长度 */
    std::vector<unsigned char> m_recvBuffer; /* 接收缓冲区 */
    ParseStep m_parseStep = ParseStep::head; /* 解析步骤 */
    int m_bodyLen = 0; /* 要接收的总包体长度, 小于0表示出错 */
    std::vector<unsigned char> m_body; /* 已接收的包体内容 */
};
} // namespace nsocket
