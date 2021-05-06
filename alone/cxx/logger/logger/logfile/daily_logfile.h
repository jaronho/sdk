#pragma once
#include "rotating_logfile.h"

#include <atomic>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

namespace logger
{
/**
 * @brief ÿ����־�ļ�
 */
class DailyLogfile final
{
public:
    /**
     * @brief ���캯��
     * @param path ��־�ļ�·��, ����: "/home/workspace/logs" �� "/home/workspace/logs/"
     * @param prefixName ��־�ļ�ǰ׺��(����Ϊ��), ����: "demo"
     * @param extName ��־�ļ���׺��, ����: "log" �� ".log"
     * @param maxSize �ļ��������ֵ(�ֽ�), ����: 4M = 4 * 1024 * 1024
     * @param maxFiles ����ļ�����, Ϊ0ʱ��ʾ������������
     */
    DailyLogfile(const std::string& path, const std::string& prefixName, const std::string& extName, size_t maxSize, size_t maxFiles = 0);

    virtual ~DailyLogfile() = default;

    /**
     * @brief ��¼��־����
     * @param content ��־����
     * @param newline �Ƿ���
     * @return �������
     */
    Logfile::Result record(const std::string& content, bool newline = true);

private:
    std::string m_path; /* ��־�ļ�·�� */
    std::string m_prefixName; /* ��־�ļ�ǰ׺�� */
    std::string m_baseName; /* ��־�ļ��� */
    std::string m_extName; /* ��־�ļ���׺�� */
    size_t m_maxSize; /* �ļ��������ֵ */
    size_t m_maxFiles; /* ����ļ����� */
    std::recursive_mutex m_mutex; /* ������ */
    std::shared_ptr<RotatingLogfile> m_rotatingLogfile; /* ������־�ļ� */
};
} // namespace logger
