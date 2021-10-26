#pragma once
#include <functional>
#include <string>
#include <unordered_map>

#include "multimap.hpp"

namespace nsocket
{
namespace http
{
/**
 * @brief HTTP����
 */
class Request
{
public:
    /**
     * @brief ͷ���ص�
     */
    using HEAD_CALLBACK = std::function<void()>;

    /**
     * @brief ���ݻص�
     * @param offset ��ǰ�ֶ����ݵ�ƫ��ֵ
     * @param data �ֶ�����
     * @param dataLen �ֶ����ݳ��� 
     */
    using CONTENT_CALLBACK = std::function<void(size_t offset, const unsigned char* data, int dataLen)>;

    /**
     * @brief �����ص�
     */
    using FINISH_CALLBACK = std::function<void()>;

public:
    /**
     * @brief ����
     * @param data �յ�������
     * @param length ���ݳ���
     * @param headCb ͷ���ص�
     * @param contentCb ���ݻص�
     * @param finishCb �����ص�
     * @return �ѽ��������ݳ���, <=0��ʾ��������
     */
    int parse(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const CONTENT_CALLBACK& contentCb,
              const FINISH_CALLBACK& finishCb);

    /**
     * @brief ��ȡ��������
     * @return ��������
     */
    std::string getContentType();

    /**
     * @brief ��ȡ���ݳ���
     * @return ���ݳ���
     */
    size_t getContentLength();

public:
    std::string method; /* ���� */
    std::string uri; /* URI */
    CaseInsensitiveMultimap queries; /* ��ѯ���� */
    std::string version; /* �汾 */
    CaseInsensitiveMultimap headers; /* ͷ�� */

private:
    /**
     * @brief ��������
     * @param data ����
     * @param length ���ݳ���
     * @return �ѽ��������ݳ���, <=0��ʾ��������
     */
    int parseMethod(const unsigned char* data, int length);

    /**
     * @brief ����URI
     * @param data ����
     * @param length ���ݳ���
     * @return �ѽ��������ݳ���, <=0��ʾ��������
     */
    int parseUri(const unsigned char* data, int length);

    /**
     * @brief ������ѯ����
     * @param data ����
     * @param length ���ݳ���
     * @return �ѽ��������ݳ���, <=0��ʾ��������
     */
    int parseQueries(const unsigned char* data, int length);

    /**
     * @brief �����汾
     * @param data ����
     * @param length ���ݳ���
     * @return �ѽ��������ݳ���, <=0��ʾ��������
     */
    int parseVersion(const unsigned char* data, int length);

    /**
     * @brief ����ͷ��
     * @param data ����
     * @param length ���ݳ���
     * @param headCb ͷ���ص�
     * @param finishCb �����ص�
     * @return �ѽ��������ݳ���, <=0��ʾ��������
     */
    int parseHeader(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const FINISH_CALLBACK& finishCb);

    /**
     * @brief �����������ͺͳ���
     */
    void parseContentTypeAndLength();

    /**
     * @brief ��������
     * @param data ����
     * @param length ���ݳ���
     * @param contentCb ���ݻص�
     * @param finishCb �����ص�
     * @return �ѽ��������ݳ���, <=0��ʾ��������
     */
    int parseContent(const unsigned char* data, int length, const CONTENT_CALLBACK& contentCb, const FINISH_CALLBACK& finishCb);

    /**
     * @brief ��ȡ��������󳤶�
     * @return ����
     */
    int maxMethodLength();

    /**
     * @brief ��鷽�����Ƿ�Ϸ�
     * @return true-�Ϸ�, false-�Ƿ�
     */
    bool checkMethod();

    /**
     * @brief ��ȡ�汾��󳤶�
     * @return ����
     */
    int maxVersionLength();

    /**
     * @brief ���汾�Ƿ�Ϸ�
     * @return true-�Ϸ�, false-�Ƿ�
     */
    bool checkVersion();

    /**
     * @brief ���ü�ֵ����
     */
    void resetTmpKV();

private:
    /**
     * @brief �ָ��
     */
    enum class SepFlag
    {
        NONE,
        R, /* \r */
        RN, /* \r\n */
        RNR /* \r\n\r */
    };

    /**
     * @brief ��������
     */
    enum class ParseStep
    {
        METHOD, /* ���� */
        URI, /* URI */
        QUERIES, /* ������� */
        VERSION, /* �汾 */
        HEADER, /* ͷ�� */
        CONTENT /* ���� */
    };

    SepFlag m_sepFlag = SepFlag::NONE; /* �ָ��� */
    ParseStep m_parseStep = ParseStep::METHOD; /* �������� */
    bool m_tmpKeyFlag = true; /* �Ƿ�� */
    std::string m_tmpKey; /* ���� */
    std::string m_tmpValue; /* ��ֵ */
    std::string m_contentType; /* �������� */
    size_t m_contentLength = 0; /* ���ݳ��� */
    size_t m_contentReceived = 0; /* �����ѽ��ճ��� */
};
using REQUEST_PTR = std::shared_ptr<Request>;
} // namespace http
} // namespace nsocket
