#pragma once
#include <vector>

#include "utilitiy/bytearray/bytearray.h"

namespace rpc
{
/**
 * @breif 错误码
 */
enum class ErrorCode
{
    OK = 0, /* 成功 */
    UNREGISTER, /* 未注册 */
    REGISTER_REPEAT, /* 重复注册 */
    CALL_BROKER_FAILED, /* 调用代理失败 */
    TARGET_NOT_FOUND, /* 目标不存在 */
    CALL_TARGET_FAILED, /* 调用目标失败 */
    TARGET_INNER_ERROR /* 目标内部出错 */
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
    case ErrorCode::OK:
        return "ok";
    case ErrorCode::UNREGISTER:
        return "unregister";
    case ErrorCode::REGISTER_REPEAT:
        return "register repeat";
    case ErrorCode::CALL_BROKER_FAILED:
        return "call borker failed";
    case ErrorCode::TARGET_NOT_FOUND:
        return "target not found";
    case ErrorCode::CALL_TARGET_FAILED:
        return "call target failed";
    case ErrorCode::TARGET_INNER_ERROR:
        return "target inner error";
    default:
        return "unknow error code [" + std::to_string((int)code) + "]";
    }
}

/**
 * @brief 消息类型
 */
enum class MsgType
{
    HEARTBEAT = 0, /* 心跳(客户端 -> 代理服务) */
    REGISTER, /* 注册(客户端 -> 代理服务) */
    REGISTER_RESULT, /* 注册结果(代理服务 -> 客户端) */
    CALL, /* 调用(客户端A -> 代理服务 -> 客户端B) */
    REPLY /* 应答(客户端B -> 代理服务 -> 客户端A) */
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
    virtual void encode(utilitiy::ByteArray& ba) = 0;

    /**
     * @brief 解码(字节流转数据结构)
     * @param ba 字节流
     */
    virtual void decode(utilitiy::ByteArray& ba) = 0;

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
        return MsgType::HEARTBEAT;
    }

    int size() const override
    {
        int sz = 0;
        sz += utilitiy::ByteArray::bcount((int)type());
        return sz;
    }

    void encode(utilitiy::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
    };

    void decode(utilitiy::ByteArray& ba) override{};
};

/**
 * @brief 注册
 */
class msg_register final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::REGISTER;
    }

    int size() const override
    {
        int sz = 0;
        sz += utilitiy::ByteArray::bcount((int)type());
        sz += utilitiy::ByteArray::bcount(self_id);
        return sz;
    }

    void encode(utilitiy::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeString(self_id);
    };

    void decode(utilitiy::ByteArray& ba) override
    {
        ba.readString(self_id);
    };

    std::string self_id; /* 客户端自身ID */
};

/**
 * @brief 注册结果
 */
class msg_register_result final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::REGISTER_RESULT;
    }

    int size() const override
    {
        int sz = 0;
        sz += utilitiy::ByteArray::bcount((int)type());
        sz += utilitiy::ByteArray::bcount((int)code);
        return sz;
    }

    void encode(utilitiy::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeInt((int)code);
    };

    void decode(utilitiy::ByteArray& ba) override
    {
        code = (ErrorCode)ba.readInt();
    };

    ErrorCode code = ErrorCode::OK; /* 错误码 */
};

/**
 * @brief 调用
 */
class msg_call : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::CALL;
    }

    int size() const override
    {
        int sz = 0;
        sz += utilitiy::ByteArray::bcount((int)type());
        sz += utilitiy::ByteArray::bcount(seq_id);
        sz += utilitiy::ByteArray::bcount(call_id);
        sz += utilitiy::ByteArray::bcount(reply_id);
        sz += utilitiy::ByteArray::bcount(data);
        return sz;
    }

    void encode(utilitiy::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeInt64(seq_id);
        ba.writeString(call_id);
        ba.writeString(reply_id);
        ba.writeBytes(data);
    }

    void decode(utilitiy::ByteArray& ba) override
    {
        seq_id = ba.readInt64();
        ba.readString(call_id);
        ba.readString(reply_id);
        ba.readBytes(data);
    }

    long long seq_id = 0; /* 序列ID */
    std::string call_id; /* 调用方ID */
    std::string reply_id; /* 应答方ID */
    std::vector<unsigned char> data; /* 数据 */
};

/**
 * @brief 应答
 */
class msg_reply final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::REPLY;
    }

    int size() const override
    {
        int sz = 0;
        sz += utilitiy::ByteArray::bcount((int)type());
        sz += utilitiy::ByteArray::bcount(seq_id);
        sz += utilitiy::ByteArray::bcount(call_id);
        sz += utilitiy::ByteArray::bcount(reply_id);
        sz += utilitiy::ByteArray::bcount(data);
        sz += utilitiy::ByteArray::bcount((int)code);
        return sz;
    }

    void encode(utilitiy::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeInt64(seq_id);
        ba.writeString(call_id);
        ba.writeString(reply_id);
        ba.writeBytes(data);
        ba.writeInt((int)code);
    };

    void decode(utilitiy::ByteArray& ba) override
    {
        seq_id = ba.readInt64();
        ba.readString(call_id);
        ba.readString(reply_id);
        ba.readBytes(data);
        code = (ErrorCode)ba.readInt();
    };

    long long seq_id = 0; /* 序列ID */
    std::string call_id; /* 调用方ID */
    std::string reply_id; /* 应答方ID */
    std::vector<unsigned char> data; /* 数据 */
    ErrorCode code = ErrorCode::OK; /* 错误码 */
};
} // namespace rpc
