#pragma once
#include <vector>

#include "utilitiy/bytearray/bytearray.h"

namespace rpc
{
/**
 * @brief 消息类型
 */
enum class MsgType
{
    HEARTBEAT = 0, /* 心跳 */
    REQ_REGISTER, /* 请求注册 */
    NOTIFY_REGISTER_RESULT, /* 通知注册结果 */
    REQ_SEND_DATA, /* 请求发送数据 */
    NOTIFY_SEND_DATA_RESULT, /* 请求发送数据结果 */
    NOTIFY_RECV_DATA /* 通知收到数据 */
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
 * @brief 请求注册
 */
class msg_req_register final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::REQ_REGISTER;
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
 * @brief 通知注册结果
 */
class msg_notify_register_result final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::NOTIFY_REGISTER_RESULT;
    }

    int size() const override
    {
        int sz = 0;
        sz += utilitiy::ByteArray::bcount((int)type());
        sz += utilitiy::ByteArray::bcount(ok);
        sz += utilitiy::ByteArray::bcount(desc);
        return sz;
    }

    void encode(utilitiy::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeBool(ok);
        ba.writeString(desc);
    };

    void decode(utilitiy::ByteArray& ba) override
    {
        ok = ba.readBool();
        ba.readString(desc);
    };

    bool ok; /* 是否成功 */
    std::string desc; /* 描述 */
};

/**
 * @brief 请求发送数据
 */
class msg_req_send_data : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::REQ_SEND_DATA;
    }

    int size() const override
    {
        int sz = 0;
        sz += utilitiy::ByteArray::bcount((int)type());
        sz += utilitiy::ByteArray::bcount(seq_id);
        sz += utilitiy::ByteArray::bcount(target_id);
        sz += utilitiy::ByteArray::bcount(data);
        return sz;
    }

    void encode(utilitiy::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeInt64(seq_id);
        ba.writeString(target_id);
        ba.writeBytes(data);
    }

    void decode(utilitiy::ByteArray& ba) override
    {
        seq_id = ba.readInt64();
        ba.readString(target_id);
        ba.readBytes(data);
    }

    long long seq_id; /* 序列ID */
    std::string target_id; /* 接收方ID */
    std::vector<unsigned char> data; /* 数据 */
};

/**
 * @brief 通知请求发送数据结果
 */
class msg_req_send_data_result final : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::NOTIFY_SEND_DATA_RESULT;
    }

    int size() const override
    {
        int sz = 0;
        sz += utilitiy::ByteArray::bcount((int)type());
        sz += utilitiy::ByteArray::bcount(seq_id);
        sz += utilitiy::ByteArray::bcount(target_id);
        sz += utilitiy::ByteArray::bcount(ok);
        sz += utilitiy::ByteArray::bcount(desc);
        return sz;
    }

    void encode(utilitiy::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeInt64(seq_id);
        ba.writeString(target_id);
        ba.writeBool(ok);
        ba.writeString(desc);
    };

    void decode(utilitiy::ByteArray& ba) override
    {
        seq_id = ba.readInt64();
        ba.readString(target_id);
        ok = ba.readBool();
        ba.readString(desc);
    };

    long long seq_id; /* 序列ID */
    std::string target_id; /* 接收方ID */
    bool ok; /* 是否成功 */
    std::string desc; /* 描述 */
};

/**
 * @brief 通知接收数据 
 */
class msg_notify_recv_data : public msg_base
{
public:
    MsgType type() const override
    {
        return MsgType::NOTIFY_RECV_DATA;
    }

    int size() const override
    {
        int sz = 0;
        sz += utilitiy::ByteArray::bcount((int)type());
        sz += utilitiy::ByteArray::bcount(seq_id);
        sz += utilitiy::ByteArray::bcount(src_id);
        sz += utilitiy::ByteArray::bcount(data);
        return sz;
    }

    void encode(utilitiy::ByteArray& ba) override
    {
        ba.allocate(size());
        ba.writeInt((int)type());
        ba.writeInt64(seq_id);
        ba.writeString(src_id);
        ba.writeBytes(data);
    }

    void decode(utilitiy::ByteArray& ba) override
    {
        seq_id = ba.readInt64();
        ba.readString(src_id);
        ba.readBytes(data);
    }

    long long seq_id; /* 序列ID */
    std::string src_id; /* 发送方ID */
    std::vector<unsigned char> data; /* 数据 */
};
} // namespace rpc
