#pragma once
#include <vector>

#include "net_msg_type.hpp"

/**
 * @brief 网络包: 请求设置自身ID 
 */
struct req_set_self_id
{
    /**
     * @brief 获取消息类型 
     */
    NetMsgType getMsgType()
    {
        return MSG_REQ_SET_SELF_ID;
    }

    /**
     * @brief 编码(结构体转为字节流) 
     */
    std::vector<unsigned char> encode()
    {
        std::vector<unsigned char> bytes;
        /* 编码时需要存入消息类型(小端) */
        size_t msgType = getMsgType();
        bytes.emplace_back((msgType >> 0) & 0xFF);
        bytes.emplace_back((msgType >> 8) & 0xFF);
        bytes.emplace_back((msgType >> 16) & 0xFF);
        bytes.emplace_back((msgType >> 24) & 0xFF);
        /* 存入`self_id`的长度(小端) */
        size_t selfIdLen = self_id.size();
        bytes.emplace_back((selfIdLen >> 0) & 0xFF);
        bytes.emplace_back((selfIdLen >> 8) & 0xFF);
        bytes.emplace_back((selfIdLen >> 16) & 0xFF);
        bytes.emplace_back((selfIdLen >> 24) & 0xFF);
        /* 存入`self_id`的值 */
        bytes.insert(bytes.end(), self_id.begin(), self_id.end());
        return bytes;
    };

    /**
     * @brief 解码(字节流转为结构体)
     */
    void decode(const std::vector<unsigned char>& bytes)
    {
        if (bytes.size() < 4)
        {
            return;
        }
        size_t offset = 0;
        /* 读取`self_id`的长度(小端) */
        size_t selfIdLen = 0;
        selfIdLen += bytes[offset + 0];
        selfIdLen += bytes[offset + 1];
        selfIdLen += bytes[offset + 2];
        selfIdLen += bytes[offset + 3];
        offset += 4;
        /* 读取`self_id`的值 */
        if (selfIdLen > bytes.size() - offset)
        {
            return;
        }
        self_id = std::string(bytes.begin() + offset, bytes.begin() + offset + selfIdLen);
    };

    std::string self_id;
};

/**
 * @brief 网络包: 请求发送数据 
 */
struct req_send_data
{
    /**
     * @brief 获取消息类型 
     */
    NetMsgType getMsgType()
    {
        return MSG_REQ_SEND_DATA;
    }

    /**
     * @brief 编码(结构体转为字节流) 
     */
    std::vector<unsigned char> encode()
    {
        std::vector<unsigned char> bytes;
        /* 编码时需要存入消息类型(小端) */
        size_t msgType = getMsgType();
        bytes.emplace_back((msgType >> 0) & 0xFF);
        bytes.emplace_back((msgType >> 8) & 0xFF);
        bytes.emplace_back((msgType >> 16) & 0xFF);
        bytes.emplace_back((msgType >> 24) & 0xFF);
        /* 存入`target_id`的长度(小端) */
        size_t targetIdLen = target_id.size();
        bytes.emplace_back((targetIdLen >> 0) & 0xFF);
        bytes.emplace_back((targetIdLen >> 8) & 0xFF);
        bytes.emplace_back((targetIdLen >> 16) & 0xFF);
        bytes.emplace_back((targetIdLen >> 24) & 0xFF);
        /* 存入`target_id`的值 */
        bytes.insert(bytes.end(), target_id.begin(), target_id.end());
        /* 存入`data`的长度(小端) */
        size_t dataLen = data.size();
        bytes.emplace_back((dataLen >> 0) & 0xFF);
        bytes.emplace_back((dataLen >> 8) & 0xFF);
        bytes.emplace_back((dataLen >> 16) & 0xFF);
        bytes.emplace_back((dataLen >> 24) & 0xFF);
        /* 存入`data`的值 */
        bytes.insert(bytes.end(), data.begin(), data.end());
        return bytes;
    }

    /**
     * @brief 解码(字节流转为结构体)
     */
    void decode(const std::vector<unsigned char>& bytes)
    {
        if (bytes.size() < 8)
        {
            return;
        }
        size_t offset = 0;
        /* 读取`target_id`的长度(小端) */
        size_t targetIdLen = 0;
        targetIdLen += bytes[offset + 0];
        targetIdLen += bytes[offset + 1];
        targetIdLen += bytes[offset + 2];
        targetIdLen += bytes[offset + 3];
        offset += 4;
        /* 读取`target_id`的值 */
        if (targetIdLen > bytes.size() - offset)
        {
            return;
        }
        target_id = std::string(bytes.begin() + offset, bytes.begin() + offset + targetIdLen);
        offset += targetIdLen;
        /* 读取`data`的长度(小端) */
        if (4 > bytes.size() - offset)
        {
            return;
        }
        size_t dataLen = 0;
        dataLen += bytes[offset + 0];
        dataLen += bytes[offset + 1];
        dataLen += bytes[offset + 2];
        dataLen += bytes[offset + 3];
        offset += 4;
        /* 读取`data`的值 */
        if (dataLen > bytes.size() - offset)
        {
            return;
        }
        data.insert(data.end(), bytes.begin() + offset, bytes.begin() + offset + dataLen);
    }

    std::string target_id;
    std::vector<unsigned char> data;
};

/**
 * @brief 网络包: 通知接收数据 
 */
struct notify_recv_data
{
    /**
     * @brief 获取消息类型 
     */
    NetMsgType getMsgType()
    {
        return MSG_NOTIFY_RECV_DATA;
    }

    /**
     * @brief 编码(结构体转为字节流) 
     */
    std::vector<unsigned char> encode()
    {
        std::vector<unsigned char> bytes;
        /* 编码时需要存入消息类型(小端) */
        size_t msgType = getMsgType();
        bytes.emplace_back((msgType >> 0) & 0xFF);
        bytes.emplace_back((msgType >> 8) & 0xFF);
        bytes.emplace_back((msgType >> 16) & 0xFF);
        bytes.emplace_back((msgType >> 24) & 0xFF);
        /* 存入`src_id`的长度(小端) */
        size_t srcIdLen = src_id.size();
        bytes.emplace_back((srcIdLen >> 0) & 0xFF);
        bytes.emplace_back((srcIdLen >> 8) & 0xFF);
        bytes.emplace_back((srcIdLen >> 16) & 0xFF);
        bytes.emplace_back((srcIdLen >> 24) & 0xFF);
        /* 存入`src_id`的值 */
        bytes.insert(bytes.end(), src_id.begin(), src_id.end());
        /* 存入`data`的长度(小端) */
        size_t dataLen = data.size();
        bytes.emplace_back((dataLen >> 0) & 0xFF);
        bytes.emplace_back((dataLen >> 8) & 0xFF);
        bytes.emplace_back((dataLen >> 16) & 0xFF);
        bytes.emplace_back((dataLen >> 24) & 0xFF);
        /* 存入`data`的值 */
        bytes.insert(bytes.end(), data.begin(), data.end());
        return bytes;
    }

    /**
     * @brief 解码(字节流转为结构体)
     */
    void decode(const std::vector<unsigned char>& bytes)
    {
        if (bytes.size() < 8)
        {
            return;
        }
        size_t offset = 0;
        /* 读取`src_id`的长度(小端) */
        size_t srcIdLen = 0;
        srcIdLen += bytes[offset + 0];
        srcIdLen += bytes[offset + 1];
        srcIdLen += bytes[offset + 2];
        srcIdLen += bytes[offset + 3];
        offset += 4;
        /* 读取`src_id`的值 */
        if (srcIdLen > bytes.size() - offset)
        {
            return;
        }
        src_id = std::string(bytes.begin() + offset, bytes.begin() + offset + srcIdLen);
        offset += srcIdLen;
        /* 读取`data`的长度(小端) */
        if (4 > bytes.size() - offset)
        {
            return;
        }
        size_t dataLen = 0;
        dataLen += bytes[offset + 0];
        dataLen += bytes[offset + 1];
        dataLen += bytes[offset + 2];
        dataLen += bytes[offset + 3];
        offset += 4;
        /* 读取`data`的值 */
        if (dataLen > bytes.size() - offset)
        {
            return;
        }
        data.insert(data.end(), bytes.begin() + offset, bytes.begin() + offset + dataLen);
    }

    std::string src_id;
    std::vector<unsigned char> data;
};
