#pragma once
#include <functional>
#include <mutex>
#include <string>

#include "session.h"

namespace nsocket
{
namespace ws
{
/**
 * @brief 服务器消息接收者(接口类)
 */
class SrvMessager
{
    friend class Server;

protected:
    /**
     * @brief 开始接收消息
     */
    virtual void onMessageBegin(const std::shared_ptr<Session>& session) = 0;

    /**
     * @brief 收到消息数据
     */
    virtual void onMessagePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen) = 0;

    /**
     * @brief 消息接收结束
     */
    virtual void onMessageEnd(const std::shared_ptr<Session>& session) = 0;
};

/**
 * @brief 服务器消息接收者(对内容批次接收, 针对大数量的请求, 比如上传文件之类的)
 */
class SrvMessager_batch : public SrvMessager
{
public:
    /**
     * @brief 开始接收消息响应函数, 注意: 严禁在函数内执行死循环或长循环逻辑, 否则会否则会阻塞该连接线程
     * @param session 会话
     */
    std::function<void(const std::shared_ptr<Session>& session)> beginCb = nullptr;

    /**
     * @brief 消息内容响应函数(可能会被批量调用), 注意: 严禁在函数内执行死循环或长循环逻辑, 否则会否则会阻塞该连接线程
     * @param session 会话
     * @param offset 本次接收的消息内容块在完整消息中偏移位置
     * @param data 本次接收的消息内容块
     * @param dataLen 本次接收的消息内容块长度
     */
    std::function<void(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen)> payloadCb = nullptr;

    /**
     * @brief 消息接收结束响应函数, 注意: 严禁在函数内执行死循环或长循环逻辑, 否则会否则会阻塞该连接线程
     * @param session 会话
     */
    std::function<void(const std::shared_ptr<Session>& session)> endCb = nullptr;

protected:
    void onMessageBegin(const std::shared_ptr<Session>& session) override;
    void onMessagePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen) override;
    void onMessageEnd(const std::shared_ptr<Session>& session) override;
};

/**
 * @brief 服务器消息接收者(内容一次性接收, 针对小数据量的请求)
 */
class SrvMessager_simple : public SrvMessager
{
public:
    /**
     * @brief 消息响应函数, 注意: 严禁在函数内执行死循环或长循环逻辑, 否则会否则会阻塞该连接线程
     * @param session 会话
     * @param isText 消息是否为文本类型, true-文本, fasle-非文本
     * @param msg 消息内容
     */
    std::function<void(const std::shared_ptr<Session>& session, bool isText, const std::string& msg)> onMessage = nullptr;

protected:
    void onMessageBegin(const std::shared_ptr<Session>& session) override;
    void onMessagePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen) override;
    void onMessageEnd(const std::shared_ptr<Session>& session) override;

private:
    std::mutex m_mutex;
    std::unordered_map<uint64_t, std::shared_ptr<std::string>> m_messageMap;
};
} // namespace ws
} // namespace nsocket
