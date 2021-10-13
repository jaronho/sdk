#pragma once
#include <vector>

#include "net_msg_type.hpp"

/**
 * @brief �����: ������������ID 
 */
struct req_set_self_id
{
    /**
     * @brief ��ȡ��Ϣ���� 
     */
    NetMsgType getMsgType()
    {
        return MSG_REQ_SET_SELF_ID;
    }

    /**
     * @brief ����(�ṹ��תΪ�ֽ���) 
     */
    std::vector<unsigned char> encode()
    {
        std::vector<unsigned char> bytes;
        /* ����ʱ��Ҫ������Ϣ����(С��) */
        size_t msgType = getMsgType();
        bytes.emplace_back((msgType >> 0) & 0xFF);
        bytes.emplace_back((msgType >> 8) & 0xFF);
        bytes.emplace_back((msgType >> 16) & 0xFF);
        bytes.emplace_back((msgType >> 24) & 0xFF);
        /* ����`self_id`�ĳ���(С��) */
        size_t selfIdLen = self_id.size();
        bytes.emplace_back((selfIdLen >> 0) & 0xFF);
        bytes.emplace_back((selfIdLen >> 8) & 0xFF);
        bytes.emplace_back((selfIdLen >> 16) & 0xFF);
        bytes.emplace_back((selfIdLen >> 24) & 0xFF);
        /* ����`self_id`��ֵ */
        bytes.insert(bytes.end(), self_id.begin(), self_id.end());
        return bytes;
    };

    /**
     * @brief ����(�ֽ���תΪ�ṹ��)
     */
    void decode(const std::vector<unsigned char>& bytes)
    {
        if (bytes.size() < 4)
        {
            return;
        }
        size_t offset = 0;
        /* ��ȡ`self_id`�ĳ���(С��) */
        size_t selfIdLen = 0;
        selfIdLen += bytes[offset + 0];
        selfIdLen += bytes[offset + 1];
        selfIdLen += bytes[offset + 2];
        selfIdLen += bytes[offset + 3];
        offset += 4;
        /* ��ȡ`self_id`��ֵ */
        if (selfIdLen > bytes.size() - offset)
        {
            return;
        }
        self_id = std::string(bytes.begin() + offset, bytes.begin() + offset + selfIdLen);
    };

    std::string self_id;
};

/**
 * @brief �����: ���������� 
 */
struct req_send_data
{
    /**
     * @brief ��ȡ��Ϣ���� 
     */
    NetMsgType getMsgType()
    {
        return MSG_REQ_SEND_DATA;
    }

    /**
     * @brief ����(�ṹ��תΪ�ֽ���) 
     */
    std::vector<unsigned char> encode()
    {
        std::vector<unsigned char> bytes;
        /* ����ʱ��Ҫ������Ϣ����(С��) */
        size_t msgType = getMsgType();
        bytes.emplace_back((msgType >> 0) & 0xFF);
        bytes.emplace_back((msgType >> 8) & 0xFF);
        bytes.emplace_back((msgType >> 16) & 0xFF);
        bytes.emplace_back((msgType >> 24) & 0xFF);
        /* ����`target_id`�ĳ���(С��) */
        size_t targetIdLen = target_id.size();
        bytes.emplace_back((targetIdLen >> 0) & 0xFF);
        bytes.emplace_back((targetIdLen >> 8) & 0xFF);
        bytes.emplace_back((targetIdLen >> 16) & 0xFF);
        bytes.emplace_back((targetIdLen >> 24) & 0xFF);
        /* ����`target_id`��ֵ */
        bytes.insert(bytes.end(), target_id.begin(), target_id.end());
        /* ����`data`�ĳ���(С��) */
        size_t dataLen = data.size();
        bytes.emplace_back((dataLen >> 0) & 0xFF);
        bytes.emplace_back((dataLen >> 8) & 0xFF);
        bytes.emplace_back((dataLen >> 16) & 0xFF);
        bytes.emplace_back((dataLen >> 24) & 0xFF);
        /* ����`data`��ֵ */
        bytes.insert(bytes.end(), data.begin(), data.end());
        return bytes;
    }

    /**
     * @brief ����(�ֽ���תΪ�ṹ��)
     */
    void decode(const std::vector<unsigned char>& bytes)
    {
        if (bytes.size() < 8)
        {
            return;
        }
        size_t offset = 0;
        /* ��ȡ`target_id`�ĳ���(С��) */
        size_t targetIdLen = 0;
        targetIdLen += bytes[offset + 0];
        targetIdLen += bytes[offset + 1];
        targetIdLen += bytes[offset + 2];
        targetIdLen += bytes[offset + 3];
        offset += 4;
        /* ��ȡ`target_id`��ֵ */
        if (targetIdLen > bytes.size() - offset)
        {
            return;
        }
        target_id = std::string(bytes.begin() + offset, bytes.begin() + offset + targetIdLen);
        offset += targetIdLen;
        /* ��ȡ`data`�ĳ���(С��) */
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
        /* ��ȡ`data`��ֵ */
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
 * @brief �����: ֪ͨ�������� 
 */
struct notify_recv_data
{
    /**
     * @brief ��ȡ��Ϣ���� 
     */
    NetMsgType getMsgType()
    {
        return MSG_NOTIFY_RECV_DATA;
    }

    /**
     * @brief ����(�ṹ��תΪ�ֽ���) 
     */
    std::vector<unsigned char> encode()
    {
        std::vector<unsigned char> bytes;
        /* ����ʱ��Ҫ������Ϣ����(С��) */
        size_t msgType = getMsgType();
        bytes.emplace_back((msgType >> 0) & 0xFF);
        bytes.emplace_back((msgType >> 8) & 0xFF);
        bytes.emplace_back((msgType >> 16) & 0xFF);
        bytes.emplace_back((msgType >> 24) & 0xFF);
        /* ����`src_id`�ĳ���(С��) */
        size_t srcIdLen = src_id.size();
        bytes.emplace_back((srcIdLen >> 0) & 0xFF);
        bytes.emplace_back((srcIdLen >> 8) & 0xFF);
        bytes.emplace_back((srcIdLen >> 16) & 0xFF);
        bytes.emplace_back((srcIdLen >> 24) & 0xFF);
        /* ����`src_id`��ֵ */
        bytes.insert(bytes.end(), src_id.begin(), src_id.end());
        /* ����`data`�ĳ���(С��) */
        size_t dataLen = data.size();
        bytes.emplace_back((dataLen >> 0) & 0xFF);
        bytes.emplace_back((dataLen >> 8) & 0xFF);
        bytes.emplace_back((dataLen >> 16) & 0xFF);
        bytes.emplace_back((dataLen >> 24) & 0xFF);
        /* ����`data`��ֵ */
        bytes.insert(bytes.end(), data.begin(), data.end());
        return bytes;
    }

    /**
     * @brief ����(�ֽ���תΪ�ṹ��)
     */
    void decode(const std::vector<unsigned char>& bytes)
    {
        if (bytes.size() < 8)
        {
            return;
        }
        size_t offset = 0;
        /* ��ȡ`src_id`�ĳ���(С��) */
        size_t srcIdLen = 0;
        srcIdLen += bytes[offset + 0];
        srcIdLen += bytes[offset + 1];
        srcIdLen += bytes[offset + 2];
        srcIdLen += bytes[offset + 3];
        offset += 4;
        /* ��ȡ`src_id`��ֵ */
        if (srcIdLen > bytes.size() - offset)
        {
            return;
        }
        src_id = std::string(bytes.begin() + offset, bytes.begin() + offset + srcIdLen);
        offset += srcIdLen;
        /* ��ȡ`data`�ĳ���(С��) */
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
        /* ��ȡ`data`��ֵ */
        if (dataLen > bytes.size() - offset)
        {
            return;
        }
        data.insert(data.end(), bytes.begin() + offset, bytes.begin() + offset + dataLen);
    }

    std::string src_id;
    std::vector<unsigned char> data;
};
