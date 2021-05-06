#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include <string>

namespace logger
{
/**
 * @brief ��־�ļ�
 */
class Logfile
{
public:
    enum class Result
    {
        OK, /* �ɹ� */
        INVALID, /* ��־�ļ���Ч */
        DISABLED, /* ��־����ֹд�� */
        TOO_LARGE, /* Ҫд�������̫��(�����ļ�����) */
        WILL_FULL, /* ����д����ļ����� */
        NEWLINE_FAILED, /* д�뻻�г��� */
        CONTENT_FAILED, /* д�����ݳ��� */
        FLUSH_FAILED /* ˢ�³��� */
    };

public:
    /**
     * @brief ����·��
     * @param path ·��, ����: "/home/workdpace/logs" �� "/home/workdpace/logs/"
     * @return true-�����ɹ�, false-����ʧ��
     */
    static bool createPath(const std::string& path);

    /**
     * @brief ����ָ��·��
     * @param path ·��, ����: "/home/workdpace/logs" �� "/home/workdpace/logs/"
     * @param folderCallback Ŀ¼�ص�
     * @param fileCallback �ļ��ص�
     * @param recursive �Ƿ�ݹ����
     */
    static void traverse(std::string path, std::function<void(const std::string& name, long createTime, long writeTime, long accessTime)> folderCallback,
                         std::function<void(const std::string& name, long createTime, long writeTime, long accessTime, unsigned long size)> fileCallback,
                         bool recursive = true);

public:
    /**
     * @brief ���캯��
     * @param path ��־�ļ�·��, ����: "/home/workdpace/logs" �� "/home/workdpace/logs/"
     * @param filename ��־�ļ���, ����: "demo.log"
     * @param maxSize �ļ��������ֵ(�ֽ�), ����: 4M = 4 * 1024 * 1024
     */
    Logfile(const std::string& path, const std::string& filename, size_t maxSize);

    ~Logfile();

    /**
     * @brief �ļ��Ƿ��Ѵ�
     * @return true-��Ч, false-��Ч(��ʧ��)
     */
    bool isOpened();

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
     * @brief ��ȡ��־�ļ�·��
     * @return ��־�ļ�·��
     */
    std::string getPath() const;

    /**
     * @brief ��ȡ��־�ļ�����
     * @return ��־�ļ���
     */
    std::string getFilename() const;

    /**
     * @brief ��ȡ��־�ļ�ȫ��(����·��)
     * @return ��־�ļ�ȫ��
     */
    std::string getFullName() const;

    /**
     * @brief ��ȡ��־�ļ��������
     * @return �ļ��������(�ֽ�)
     */
    size_t getMaxSize() const;

    /**
     * @brief �Ƿ����ü�¼����
     * @return true-����, false-����
     */
    bool isEnable() const;

    /**
     * @brief �����Ƿ����ü�¼����
     * @param enable ���ñ�ʶ
     */
    void setEnable(bool enable);

    /**
     * @brief ��ȡ�ļ���ǰ��С(�ֽ�)
     * @return ��ǰ��С
     */
    size_t getSize();

    /**
     * @brief �����־�ļ�����
     */
    void clear();

    /**
     * @brief ��¼��־����
     * @param content ��־����
     * @param newline �Ƿ���
     * @return �������
     */
    Result record(const std::string& content, bool newline = true);

private:
    std::string m_path; /* ��־�ļ�·�� */
    std::string m_filename; /* ��־�ļ��� */
    std::string m_fullName; /* ��־�ļ�ȫ��(����·��) */
    size_t m_maxSize; /* �ļ��������ֵ */
    std::recursive_mutex m_mutex; /* ������ */
    FILE* m_fp = nullptr; /* �ļ�ָ�� */
    std::atomic_size_t m_size = 0; /* �ļ���ǰ��С */
    std::atomic_bool m_enable = true; /* �Ƿ�������־��¼���� */
};
} // namespace logger
