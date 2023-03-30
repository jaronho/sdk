#pragma once
#include "inifile.h"

namespace ini
{
/**
 * @brief INI�ļ��鿴��
 */
class IniReader : protected IniFile
{
public:
    virtual ~IniReader() = default;

    /**
     * @brief ���ļ�
     * @param filename �ļ���, ����: "test.ini", "../test.ini", "temp\\test.ini"
     * @param errorDesc [���]������Ϣ
     * @return 0-�ɹ�, 1-�ļ��޷���, 2-��ƥ��']', 3-������Ϊ��, 4-�������ظ�, 5-���������, 6-�������ظ�
     */
    int open(const std::string& filename, std::string& errorDesc);

    /**
     * @brief �����ļ�
     * @param errorDesc [���]������Ϣ
     * @return 0-�ɹ�, 1-�ļ��޷���, 2-��ƥ��']', 3-������Ϊ��, 4-�������ظ�, 5-���������, 6-�������ظ�
     */
    int reload(std::string& errorDesc);

    /**
     * @brief ��ȡ���б�
     * @return ���б�
     */
    std::vector<IniSection> getSections() const;

    /**
     * @brief ��ȡע�ͱ�ʶ���б�
     * @return ��ʶ���б�
     */
    std::vector<std::string> getCommentFlags() const;

    /**
     * @brief �Ƿ���ڽ�
     * @param name ������
     * @return true-����, false-������
     */
    bool hasSection(const std::string& name) const;

    /**
     * @brief ��ȡ��ע��
     * @param name ������
     * @param comment [���]ע��
     * @return true-�ɹ�, flalse-ʧ��(������)
     */
    bool getSectionComment(const std::string& name, std::string& comment) const;

    /**
     * @brief �Ƿ������
     * @param name ������
     * @param key ��
     * @return true-����, false-������
     */
    bool hasItem(const std::string& name, const std::string& key) const;

    /**
     * @brief ��ȡ��ע��
     * @param name ������
     * @param key ��
     * @param comment [���]ע��
     * @return true-�ɹ�, flalse-ʧ��(������)
     */
    bool getComment(const std::string& name, const std::string& key, std::string& comment) const;

    /**
     * @brief ��ȡ������
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    bool getBool(const std::string& name, const std::string& key, bool defaultValue = false) const;

    /**
     * @brief ��ȡ����
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    int getInt(const std::string& name, const std::string& key, int defaultValue = 0) const;

    /**
     * @brief ��ȡ�޷�������
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    unsigned int getUInt(const std::string& name, const std::string& key, unsigned int defaultValue = 0) const;

    /**
     * @brief ��ȡ������
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    long getLong(const std::string& name, const std::string& key, long defaultValue = 0) const;

    /**
     * @brief ��ȡ�޷��ų�����
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    unsigned long getULong(const std::string& name, const std::string& key, unsigned long defaultValue = 0) const;

    /**
     * @brief ��ȡ64λ������
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    long long getLongLong(const std::string& name, const std::string& key, long long defaultValue = 0) const;

    /**
     * @brief ��ȡ�޷���64λ������
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    unsigned long long getULongLong(const std::string& name, const std::string& key, unsigned long long defaultValue = 0) const;

    /**
     * @brief ��ȡ�����ȸ�����
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    float getFloat(const std::string& name, const std::string& key, float defaultValue = 0.0f) const;

    /**
     * @brief ��ȡ˫���ȸ�����
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    double getDouble(const std::string& name, const std::string& key, double defaultValue = 0.0) const;

    /**
     * @brief ��ȡ�ַ���
     * @param name ������
     * @param key ��
     * @param defaultValue Ĭ��ֵ
     * @return ֵ
     */
    std::string getString(const std::string& name, const std::string& key, std::string defaultValue = std::string()) const;

    /**
     * @brief �Ƿ���������/��
     * @return false-������
     */
    bool isAllowAutoCreate() const override;

private:
    std::string m_filename; /* �����ļ��� */
};
} // namespace ini
