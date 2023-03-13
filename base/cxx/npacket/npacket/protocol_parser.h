#pragma once
#include <mutex>
#include <vector>

#include "protocol.h"

namespace npacket
{
/**
 * @brief 应用层协议
 */
enum ApplicationProtocol
{
    FTP,
    IEC103,
};

/**
 * @brief 应用层协议解析器(接口类)
 */
class ProtocolParser
{
public:
    /**
     * @brief 添加子解析器
     * @param parser 子解析器
     * @return true-添加成功, false-添加失败
     */
    bool addChild(const std::shared_ptr<ProtocolParser>& parser);

    /**
     * @brief 删除子解析器
     * @param protocol 应用层协议
     */
    void removeChild(uint32_t protocol);

    /**
     * @brief 获取子解析器列表
     * @param 子解析器列表
     */
    std::vector<std::shared_ptr<ProtocolParser>> getChildren();

    /**
     * @brief 获取应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    virtual uint32_t getProtocol() const = 0;

    /**
     * @brief 解析
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param payload 层负载
     * @param payloadLen 层负载长度
     * @return true-成功, false-失败
     */
    virtual bool parse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen) = 0;

private:
    std::mutex m_mutexChildren;
    std::vector<std::shared_ptr<ProtocolParser>> m_children; /* 子解析器列表 */
};
} // namespace npacket
