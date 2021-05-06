#pragma once
#include "logfile.h"

#include <atomic>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

namespace logger
{
/**
 * @brief ������־�ļ�
 */
class RotatingLogfile final
{
public:
    /**
     * @brief ���캯��
     * @param path ��־�ļ�·��, ����: "/home/workspace/logs" �� "/home/workspace/logs/"
     * @param baseName ��־�ļ���, ����: "demo"
     * @param extName ��־�ļ���׺��, ����: "log" �� ".log"
     * @param maxSize �ļ��������ֵ(�ֽ�), ����: 4M = 4 * 1024 * 1024
     * @param maxFiles ����ļ�����, Ϊ0ʱ��ʾ������������
     */
    RotatingLogfile(const std::string& path, const std::string& baseName, const std::string& extName, size_t maxSize, size_t maxFiles = 0);

    virtual ~RotatingLogfile() = default;

    /**
     * @brief ��
     * @return true-�ɹ�, false-ʧ��
     */
    bool open();

    /**
     * @brief �ر�
     */
    void close();

    /**
     * @brief ��ȡ��ǰ��־�ļ�����ֵ
     * @return ����ֵ
     */
    size_t getFileIndex() const;

    /**
     * @brief ��¼��־����
     * @param content ��־����
     * @param newline �Ƿ���
     * @return �������
     */
    Logfile::Result record(const std::string& content, bool newline = true);

private:
    /**
     * @brief ����ָ��·����ƥ����ļ��������б�
     * @param path ·��
     * @param pattern ƥ��ģʽ(������ʽ)
     * @param indexList ƥ�䵽�������б�(����������)
     * @return true-���ҵ�, false-δ�ҵ�
     */
    bool findIndexList(const std::string& path, const std::regex& pattern, std::vector<int>& indexList);

    /**
     * @brief ����ָ��·���µ���������ֵ
     * @param path ·��
     * @param indexList ƥ�䵽�������б�(����������)
     * @return ����ֵ
     */
    int findLastIndex(const std::string& path, std::vector<int>& indexList);

    /**
     * @brief ��������ֵ�����ļ���
     * @param index ����ֵ
     * @return �ļ���
     */
    std::string calcFilenameByIndex(int index);

    /**
     * @brief �����ļ�
     * @return true-�ɹ�, false-ʧ��
     */
    bool rotateFileList();

private:
    std::string m_baseName; /* ��־�ļ��� */
    std::string m_extName; /* ��־�ļ���׺�� */
    size_t m_maxFiles; /* ����ļ����� */
    std::atomic_size_t m_index; /* ��ǰ��־�ļ�����ֵ */
    std::recursive_mutex m_mutex; /* ������ */
    std::shared_ptr<Logfile> m_logfile; /* ������־�ļ� */
};
} // namespace logger
