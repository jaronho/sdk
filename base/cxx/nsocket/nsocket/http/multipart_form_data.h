#pragma once
#include <functional>
#include <string>
#include <unordered_map>

namespace nsocket
{
namespace http
{
/**
 * @brief ������
 */
class MultipartFormData
{
public:
    /**
     * @brief �ı��ص�
     * @param name ����
     * @param contentType ��������
     * @param text �ı�����
     */
    using TEXT_CALLBACK = std::function<void(const std::string& name, const std::string& contentType, const std::string& text)>;

    /**
     * @brief �ļ��ص�
     * @param name ����
     * @param filename �ļ���
     * @param contentType ��������
     * @param offset �ļ��������ļ��е�ƫ��ֵ
     * @param data �ļ�����
     * @param dataLen ���ݳ���
     * @param finish �Ƿ����
     */
    using FILE_CALLBACK = std::function<void(const std::string& name, const std::string& filename, const std::string& contentType,
                                             size_t offset, const unsigned char* data, int dataLen, bool finish)>;

public:
    /**
     * @brief ���캯��
     * @param boundary �߽���
     */
    MultipartFormData(const std::string& boundary);

    /**
     * @brief ����
     * @param data ������
     * @param length ���ݳ���
     * @param textCb �ı��ص�
     * @param fileCb �ļ��ص�
     * @return �ѽ��������ݳ���, <=0��ʾ��������
     */
    int parse(const unsigned char* data, int length, const TEXT_CALLBACK& textCb, const FILE_CALLBACK& fileCb);

private:
    /**
     * @brief �����߽���
     */
    int parseBoundary(const unsigned char* data, int length);

    /**
     * @brief �������ݴ�����Ϣ
     */
    int parseContentDisposition(const unsigned char* data, int length);

    /**
     * @brief �������ƺ��ļ���
     */
    bool parseNameAndFilename();

    /**
     * @brief ������������
     */
    int parseContentType(const unsigned char* data, int length);

    /**
     * @brief ��������
     */
    int parseEmptyLine(const unsigned char* data, int length);

    /**
     * @brief ��������
     */
    int parseContent(const unsigned char* data, int length, const TEXT_CALLBACK& textCb, const FILE_CALLBACK& fileCb);

    /**
     * @brief �����ı�����
     */
    int parseTextContent(const unsigned char* data, int length, const TEXT_CALLBACK& textCb);

    /**
     * @brief �����ļ�����
     */
    int parseFileContent(const unsigned char* data, int length, const FILE_CALLBACK& fileCb);

    /**
     * @brief ����ǰһ����������
     */
    void handlePrevLine(std::string& prevLine, const FILE_CALLBACK& fileCb);

private:
    /**
     * @brief �ָ��
     */
    enum class SepFlag
    {
        NONE,
        R, /* \r */
        RN /* \r\n */
    };

    /**
     * @brief ��������
     */
    enum class ParseStep
    {
        BOUNDARY, /* �߽��� */
        CONTENT_DISPOSITION, /* ���ݴ�����Ϣ */
        CONTENT_TYPE, /* �������� */
        EMPTY_LINE, /* ���� */
        CONTENT, /* ���� */
        ENDING /* ������ */
    };

    std::string m_boundary; /* �߽��� */
    SepFlag m_sepFlag = SepFlag::NONE; /* �ָ��� */
    ParseStep m_parseStep = ParseStep::BOUNDARY; /* �������� */
    std::string m_nowLine; /* ��ǰ������ */
    std::string m_name; /* ������ */
    std::string m_filename; /* �ļ��� */
    std::string m_contentType; /* ������������ */
    size_t m_fileOffset = 0; /* �ļ�ƫ�� */
};
} // namespace http
} // namespace nsocket
