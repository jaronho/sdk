#pragma once
#include <boost/system/system_error.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "logger/logger_manager.h"
#include "threading/thread_proxy.hpp"

namespace nac
{
/**
 * @brief 会话封装器
 */
class SessionWrapper : public std::enable_shared_from_this<SessionWrapper>
{
public:
    /**
     * @brief 响应回调
     * @param sendOk 是否发送成功
     * @param bizCode 业务码
     * @param seqId 序列ID
     * @param data 数据
     */
    using RespCallback = std::function<void(bool sendOk, int32_t bizCode, int64_t seqId, const std::string& data)>;

public:
    /**
     * @brief 发送消息
     * @param bizCode 业务码
     * @param seqId 序列ID, 如果填0则内部自动生成
     * @param data 数据
     * @param timeout 超时, 大于0时表示需要等待响应数据, 小等于0表示不需要等待响应数据
     * @param callback 响应回调
     * @return 序列ID, -1表示失败
     */
    int64_t sendMsg(int32_t bizCode, int64_t seqId, const std::string& data, int timeout, const RespCallback& callback = nullptr);

    /**
     * @brief 清除会话路由表
     */
    void clearSessionMap();

protected:
    /**
     * @brief 发送回调
     * @param code 错误码
     * @param length 数据长度
     */
    using SendCallback = std::function<void(const boost::system::error_code& code, size_t length)>;

protected:
    /**
     * @brief 响应数据(子类收到包时调用)
     * @param bizCode 业务码
     * @param seqId 序列ID
     * @param length 包大小(可能大于data的大小)
     * @param data 数据
     * @return true-成功, false-失败(需要继续处理)
     */
    bool onResp(int32_t bizCode, int64_t seqId, size_t length, const std::string& data);

    /**
     * @brief 获取日志器(子类可重写)
     * @return 日志器
     */
    virtual logger::Logger myLogger();

    /**
     * @brief 获取定时器执行线程(子类可重写)
     * @return 定时器执行线程
     */
    virtual std::shared_ptr<threading::Executor> myTimerExecutor();

    /**
     * @brief 发送消息(子类必须实现)
     * @param bizCode 业务码
     * @param seqId 序列ID
     * @param data 数据
     * @param timeout 超时, 大于0时表示需要等待响应数据, 小等于0表示不需要等待响应数据
     * @param callback 发送回调
     * @return 序列ID, -1表示失败
     */
    virtual int64_t sendImpl(int32_t bizCode, int64_t seqId, const std::string& data, int timeout, const SendCallback& callback) = 0;

private:
    /**
     * @brief 响应回调
     * @param sendOk 是否发送成功
     * @param bizcode 业务码
     * @param seqId 序列ID
     * @param waitResp 是否需要等待响应数据
     * @param callback 响应回调
     */
    void onRespCallback(bool sendOk, int32_t bizCode, int64_t seqId, bool waitResp, const RespCallback& callback);

private:
    class Session;
    friend Session;

private:
    std::mutex m_mutexSessionMap;
    std::unordered_map<int64_t, std::shared_ptr<Session>> m_sessionMap; /* 会话路由表 */
};
} // namespace nac
