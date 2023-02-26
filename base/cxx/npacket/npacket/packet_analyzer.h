#pragma once
#include <functional>

#include "protocol.h"

namespace npacket
{
/**
 * @brief �����ݻص�
 * @param totalLen ���ݰ��ܳ���
 * @param header ��ͷ��
 * @param payload �㸺��
 * @param payloadLen �㸺�س���
 * @return true-����������һ��, false-ֹͣ��������
 */
using LAYER_CALLBACK =
    std::function<bool(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)>;

/**
 * @brief ��������
 */
class PacketAnalyzer
{
public:
    /**
     * @brief ���ò����ݻص�
     * @param ethernetLayerCb ��̫�������ݻص�
     * @param networkLayerCb ��������ݻص�
     * @param transportLayerCb ��������ݻص�
     */
    void setLayerCallback(const LAYER_CALLBACK& ethernetLayerCb, const LAYER_CALLBACK& networkLayerCb,
                          const LAYER_CALLBACK& transportLayerCb);

    /**
     * @brief ��������
     * @param data ����
     * @param dataLen ���ݳ���
     * @return -1-����Ϊ��, 0-�ɹ�, 1-������̫����ʧ��, 2-���������ʧ��, 3-���������ʧ��
     */
    int parse(const uint8_t* data, uint32_t dataLen);

private:
    /**
     * @brief ������̫��������
     * @param data ������
     * @param dataLen �����ݳ���
     * @param headerLen [���]Э��ͷ������
     * @param networkProtocol [���]�����Э������
     * @return Э��ͷ��
     */
    std::shared_ptr<ProtocolHeader> handleEthernetLayer(const uint8_t* data, uint32_t dataLen, uint32_t& headerLen,
                                                        uint32_t& networkProtocol);

    /**
     * @brief �������������
     * @param networkProtocol �����Э������
     * @param data ������
     * @param dataLen �����ݳ���
     * @param headerLen [���]Э��ͷ������
     * @param networkProtocol [���]�����Э������
     * @return Э��ͷ��
     */
    std::shared_ptr<ProtocolHeader> handleNetworkLayer(const uint32_t& networkProtocol, const uint8_t* data, uint32_t dataLen,
                                                       uint32_t& headerLen, uint32_t& transportProtocol);

    /**
     * @brief �����������
     * @param transportProtocol �����Э������
     * @param data ������
     * @param dataLen �����ݳ���
     * @param headerLen [���]Э��ͷ������
     * @return Э��ͷ��
     */
    std::shared_ptr<ProtocolHeader> handleTransportLayer(const uint32_t& transportProtocol, const uint8_t* data, uint32_t dataLen,
                                                         uint32_t& headerLen);

private:
    LAYER_CALLBACK m_ethernetLayerCb = nullptr; /* ��̫�������ݻص� */
    LAYER_CALLBACK m_networkLayerCb = nullptr; /* ��������ݻص� */
    LAYER_CALLBACK m_transportLayerCb = nullptr; /* ��������ݻص� */
};
} // namespace npacket
