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
     * @brief 原始数据回调函数(无法解析出包体)
     * @param data 数据 
     */
    using RAW_CALLBACK = std::function<void(const std::vector<unsigned char>& data)>;

public:
    /**
     * @brief 构造函数
     * @param bodyMaxLen 允许的包体最大长度
     */
    Payload(int bodyMaxLen);

    /**
     * @brief 重置
     */
    void reset();

    /**
     * @brief 对数据进行拼接/拆包
     * @param data 数据
     * @param bodyCb 包体回调
     * @param rawCb 原始数据回调
     */
    void unpack(const std::vector<unsigned char>& data, const BODY_CALLBACK& bodyCb, const RAW_CALLBACK& rawCb);

    /**
     * @brief 对数据进行打包
     * @param body 包体
     * @param bodyLen 包体长度
     * @param data [输出]包数据 = 包头 + 包体
     */
    static void pack(const unsigned char* body, int bodyLen, std::vector<unsigned char>& data);

    /**
     * @brief 对数据进行打包
     * @param body 包体
     * @param data [输出]包数据 = 包头 + 包体
     */
    static void pack(const std::vector<unsigned char>& body, std::vector<unsigned char>& data);

private:
    std::vector<unsigned char> m_recvBuffer; /* 接收缓冲区 */
    int m_bodyMaxLen; /* 允许的包体最大长度 */
    int m_head; /* 包头(占4个字节, 用于存放包体长度) */
    std::vector<unsigned char> m_body; /* 已接收的包体内容 */
};
} // namespace nsocket
