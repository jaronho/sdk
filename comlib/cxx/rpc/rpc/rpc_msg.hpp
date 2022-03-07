#pragma once
#include <vector>

#include "utility/bytearray/bytearray.h"

namespace rpc
{
/**
 * @breif 错误码
 */
enum class ErrorCode
{
    ok = 0, /* 成功 */
    unbind, /* 未绑定 */
    bind_repeat, /* 重复绑定 */
    call_broker_failed, /* 调用代理失败 */
    replyer_not_found, /* 应答者不存在 */
    call_replyer_failed, /* 调用应答者失败 */
    replyer_inner_error, /* 目标内部出错 */
    timeout /* 超时 */
};

/**
 * @breif 错误描述
 * @param code 错误码
 * @return 描述
 */
static std::string error_desc(const ErrorCode& code)
{
    switch (code)
    {
    case ErrorCode::ok:
        return "ok";
    case ErrorCode::unbind:
        return "unbind";
    case ErrorCode::bind_repeat:
        return "bind repeat";
    case ErrorCode::call_broker_failed:
        return "call borker failed";
    case ErrorCode::replyer_not_found:
        return "replyer not found";
    case ErrorCode::call_replyer_failed:
        return "call repyer failed";
    case ErrorCode::replyer_inner_error:
        return "replyer inner error";
    case ErrorCode::timeout:
        return "timeout";
    default:
        return "unknow error code [" + std::to_string((int)code) + "]";
    }
}

/**
 * @brief 消息类型
 */
enum class MsgType
{
    heartbeat = 0, /* 心跳(客户端 -> 代理服务) */
    bind, /* 绑定(客户端 -> 代理服务) */
    bind_result, /* 绑定结果(代理服务 -> 客户端) */
    call, /* 调用(客户端A -> 代理服务 -> 客户端B) */
    reply /* 应答(客户端B -> 代理服务 -> 客户端A) */
};

/**
 * @brief 消息基类ID
 */
class msg_base
{
public:
    /**
     * @brief 获取消息类型
     * @return 消息类型
     */
    virtual MsgType type() const = 0;

    /**
     * @brief 获取消息实际占用字节大小
     * @return 字节大小
     */
    virtual int size() const = 0;

    /**
     * @brief 编码(数据结构转字节流)
     * @param ba 字节流
     */
    virtual void encode(utility::ByteArray& ba) = 0;

    /**
     * @brief 解码(字节流转数据结构)
     * @param ba 字节流
     */
    virtual void decode(utility::ByteArray& ba) = 0;

    /**
     * @brief 获取消息最大长度
     * @return 消息最大长度
     */
    static int maxsize()
    {
        static const int MAX_SIZE = 1024 * 1024; /* 限定消息不超过1M */
        return MAX_SIZE;
    }

protected:
};

/**
 * @brief 心跳
 */
class msg_heartbeat final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::heartbeat;
    }

    int size() const override
    {
        int sz = 0;
        sz += utility::ByteArray::bcount((int)type());
        return sz;
    }

    void encode(utility::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
    };

    void decode(utility::ByteArray& ba) override{};
};

/**
 * @brief 绑定
 */
class msg_bind final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::bind;
    }

    int size() const override
    {
        int sz = 0;
        sz += utility::ByteArray::bcount((int)type());
        sz += utility::ByteArray::bcount(self_id);
        return sz;
    }

    void encode(utility::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeString(self_id);
    };

    void decode(utility::ByteArray& ba) override
    {
        ba.readString(self_id);
    };

    std::string self_id; /* 客户端自身ID */
};

/**
 * @brief 绑定结果
 */
class msg_bind_result final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::bind_result;
    }

    int size() const override
    {
        int sz = 0;
        sz += utility::ByteArray::bcount((int)type());
        sz += utility::ByteArray::bcount((int)code);
        return sz;
    }

    void encode(utility::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeInt((int)code);
    };

    void decode(utility::ByteArray& ba) override
    {
        code = (ErrorCode)ba.readInt();
    };

    ErrorCode code = ErrorCode::ok; /* 错误码 */
};

/**
 * @brief 调用
 */
class msg_call : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::call;
    }

    int size() const override
    {
        int sz = 0;
        sz += utility::ByteArray::bcount((int)type());
        sz += utility::ByteArray::bcount(seq_id);
        sz += utility::ByteArray::bcount(caller);
        sz += utility::ByteArray::bcount(replyer);
        sz += utility::ByteArray::bcount(proc);
        sz += utility::ByteArray::bcount(data);
        sz += utility::ByteArray::bcount(timeout);
        return sz;
    }

    void encode(utility::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeInt64(seq_id);
        ba.writeString(caller);
        ba.writeString(replyer);
        ba.writeInt(proc);
        ba.writeBytes(data);
        ba.writeInt(timeout);
    }

    void decode(utility::ByteArray& ba) override
    {
        seq_id = ba.readInt64();
        ba.readString(caller);
        ba.readString(replyer);
        proc = ba.readInt();
        ba.readBytes(data);
        timeout = ba.readInt();
    }

    long long seq_id = 0; /* 序列ID */
    std::string caller; /* 调用者ID */
    std::string replyer; /* 应答者ID */
    int proc = 0; /* 调用程序ID */
    std::vector<unsigned char> data; /* 数据 */
    int timeout = 0; /* 超时时间(毫秒) */
};

/**
 * @brief 应答
 */
class msg_reply final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::reply;
    }

    int size() const override
    {
        int sz = 0;
        sz += utility::ByteArray::bcount((int)type());
        sz += utility::ByteArray::bcount(seq_id);
        sz += utility::ByteArray::bcount(caller);
        sz += utility::ByteArray::bcount(replyer);
        sz += utility::ByteArray::bcount(proc);
        sz += utility::ByteArray::bcount(data);
        sz += utility::ByteArray::bcount((int)code);
        return sz;
    }

    void encode(utility::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeInt64(seq_id);
        ba.writeString(caller);
        ba.writeString(replyer);
        ba.writeInt(proc);
        ba.writeBytes(data);
        ba.writeInt((int)code);
    };

    void decode(utility::ByteArray& ba) override
    {
        seq_id = ba.readInt64();
        ba.readString(caller);
        ba.readString(replyer);
        proc = ba.readInt();
        ba.readBytes(data);
        code = (ErrorCode)ba.readInt();
    };

    long long seq_id = 0; /* 序列ID */
    std::string caller; /* 调用者ID */
    std::string replyer; /* 应答者ID */
    int proc = 0; /* 调用程序ID */
    std::vector<unsigned char> data; /* 数据 */
    ErrorCode code = ErrorCode::ok; /* 错误码 */
};
} // namespace rpc
