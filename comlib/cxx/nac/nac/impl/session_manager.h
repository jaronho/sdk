#pragma once
#include <map>

#include "logger/logger_manager.h"
#include "protocol_adapter.hpp"

namespace nac
{
/**
 * @brief 会话管理器
 */
class SessionManager final : public std::enable_shared_from_this<SessionManager>
{
public:
    /**
     * @brief 消息接收者
     * @param bizCode 业务码
     * @param seqId 序列ID
     * @param data 数据
     */
    using MsgReceiver = std::function<void(unsigned int bizCode, int64_t seqId, const std::string& data)>;

    /**
     * @brief 响应回调
     * @param sendOk 是否发送成功
     * @param bizCode 业务码
     * @param seqId 序列ID
     * @param data 数据
     */
    using ResponseCallback = std::function<void(bool sendOk, unsigned int bizCode, int64_t seqId, const std::string& data)>;

public:
    /**
     * @brief 设置协议适配器
     * @param adapter 适配器
     */
    void setProtocolAdapter(const std::shared_ptr<ProtocolAdapter>& adapter);

    /**
     * @brief 设置消息接收者
     * @param receiver 接收者
     */
    void setMsgReceiver(const MsgReceiver& receiver);

    /**
     * @brief 发送消息
     * @param bizCode 业务码
     * @param data 数据
     * @param timeout 超时, 大于0时表示需要等待服务端响应数据, 等于0表示不需要响应
     * @param callback 响应回调
     * @return 序列ID
     */
    int64_t sendMsg(unsigned int bizCode, const std::string& data, int timeout, const ResponseCallback& callback);

    /**
     * @brief 清除会话路由表
     */
    void clearSessionMap();

private:
    /**
     * @brief 响应数据包
     * @param packet 数据包
     */
    void onProcessPacket(const std::shared_ptr<ProtocolAdapter::Packet>& packet);

    /**
     * @brief 响应发送数据回调
     * @param sendOk 是否发送成功
     * @param bizcode 业务码
     * @param seqId 序列ID
     * @param callback 发送响应回调
     */
    void onResponseCallback(bool sendOk, unsigned int bizCode, int64_t seqId, const ResponseCallback& callback);

private:
    class Session;
    friend Session;

private:
    std::vector<threading::ScopedSignalConnection> m_connections; /* 信号连接 */
    std::weak_ptr<ProtocolAdapter> m_wpProtocolAdapter; /* 协议适配器 */
    MsgReceiver m_msgReceiver = nullptr; /* 消息接收者 */
    std::mutex m_mutexSessionMap;
    std::map<int64_t, std::shared_ptr<Session>> m_sessionMap; /* 会话路由表 */
    logger::Logger m_logger = logger::LoggerManager::getLogger("NAC");
};
} // namespace nac