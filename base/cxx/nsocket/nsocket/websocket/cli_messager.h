#pragma once
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace nsocket
{
namespace ws
{
/**
 * @brief 客户端消息接收者(接口类)
 */
class CliMessager
{
    friend class Client;

protected:
    /**
     * @brief 开始接收消息
     */
    virtual void onMessageBegin(bool isText) = 0;

    /**
     * @brief 收到消息数据
     */
    virtual void onMessagePayload(bool isText, size_t offset, const unsigned char* data, int dataLen) = 0;

    /**
     * @brief 消息接收结束
     */
    virtual void onMessageEnd(bool isText) = 0;
};

/**
 * @brief 客户端消息接收者(对内容批次接收, 针对大数量的请求, 比如上传文件之类的)
 */
class CliMessager_batch : public CliMessager
{
public:
    /**
     * @brief 开始接收消息响应函数, 注意: 严禁在函数内执行死循环或长循环逻辑, 否则会否则会阻塞该连接线程
     * @param isText 消息是否为文本类型, true-文本, fasle-非文本
     */
    std::function<void(bool isText)> beginCb = nullptr;

    /**
     * @brief 消息内容响应函数(可能会被批量调用), 注意: 严禁在函数内执行死循环或长循环逻辑, 否则会否则会阻塞该连接线程
     * @param isText 消息是否为文本类型, true-文本, fasle-非文本
     * @param offset 本次接收的消息内容块在完整消息中偏移位置
     * @param data 本次接收的消息内容块
     * @param dataLen 本次接收的消息内容块长度
     */
    std::function<void(bool isText, size_t offset, const unsigned char* data, int dataLen)> payloadCb = nullptr;

    /**
     * @brief 消息接收结束响应函数, 注意: 严禁在函数内执行死循环或长循环逻辑, 否则会否则会阻塞该连接线程
     * @param isText 消息是否为文本类型, true-文本, fasle-非文本
     */
    std::function<void(bool isText)> endCb = nullptr;

protected:
    void onMessageBegin(bool isText) override;
    void onMessagePayload(bool isText, size_t offset, const unsigned char* data, int dataLen) override;
    void onMessageEnd(bool isText) override;
};

/**
 * @brief 客户端消息接收者(内容一次性接收, 针对小数据量的请求)
 */
class CliMessager_simple : public CliMessager
{
public:
    /**
     * @brief 消息响应函数, 注意: 严禁在函数内执行死循环或长循环逻辑, 否则会否则会阻塞该连接线程
     * @param isText 消息是否为文本类型, true-文本, fasle-非文本
     * @param msg 消息内容
     */
    std::function<void(bool isText, const std::string& msg)> onMessage = nullptr;

protected:
    void onMessageBegin(bool isText) override;
    void onMessagePayload(bool isText, size_t offset, const unsigned char* data, int dataLen) override;
    void onMessageEnd(bool isText) override;

private:
    std::string m_msg;
};
} // namespace ws
} // namespace nsocket
