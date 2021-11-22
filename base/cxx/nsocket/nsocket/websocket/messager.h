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
 * @brief 消息接收者
 */
class Messager
{
    friend class Server;

protected:
    /**
     * @brief 开始接收消息
     */
    virtual void onMessageBegin(const std::shared_ptr<Session>& session);

    /**
     * @brief 收到消息数据
     */
    virtual void onMessagePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen);

    /**
     * @brief 消息接收结束
     */
    virtual void onMessageEnd(const std::shared_ptr<Session>& session);
};

/**
 * @brief 消息接收者(对内容批次接收, 针对大数量的请求, 比如上传文件之类的)
 */
class Messager_batch final : public Messager
{
public:
    std::function<void(const std::shared_ptr<Session>& session)> beginCb;
    std::function<void(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen)> payloadCb;
    std::function<void(const std::shared_ptr<Session>& session)> endCb;

protected:
    void onMessageBegin(const std::shared_ptr<Session>& session) override;
    void onMessagePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen) override;
    void onMessageEnd(const std::shared_ptr<Session>& session) override;
};

/**
 * @brief 消息接收者(内容一次性接收, 针对小数据量的请求)
 */
class Messager_simple final : public Messager
{
public:
    std::function<void(const std::shared_ptr<Session>& session, bool isText, const std::string& msg)> onMessage;

protected:
    void onMessageBegin(const std::shared_ptr<Session>& session) override;
    void onMessagePayload(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen) override;
    void onMessageEnd(const std::shared_ptr<Session>& session) override;

private:
    std::mutex m_mutex;
    std::unordered_map<int64_t, std::shared_ptr<std::string>> m_messageMap;
};
} // namespace ws
} // namespace nsocket
