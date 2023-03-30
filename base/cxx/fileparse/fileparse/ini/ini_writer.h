#pragma once
#include "ini_reader.h"

namespace ini
{
/**
 * @brief INI�ļ���д��
 */
class IniWriter final : public IniReader
{
public:
    virtual ~IniWriter() = default;

    /**
     * @brief ���ļ�
     * @param filename �ļ���, ����: "test.ini", "../test.ini", "temp\\test.ini"
     * @param errorDesc [���]������Ϣ
     * @return 0-�ɹ�, 1-�ļ��޷���, 2-��ƥ��']', 3-������Ϊ��, 4-�������ظ�, 5-���������, 6-�������ظ�
     */
    int open(const std::string& filename, std::string& errorDesc);

    /**
     * @brief ���������Զ�������/��
     */
    void setAllowAutoCreate();

    /**
     * @brief �����ļ�
     * @param sortType ��������: 0-Ĭ��, 1-����, 2-����
     * @return 0-�ɹ�, 1-δ���, 2-�ļ���ʧ��, 3-д�ļ�ʧ��
     */
    int save(int sortType = 0);

    /**
     * @brief ���
     */
    void clear();

    /**
     * @brief ����ע�ͱ�ʶ���б�
     * @param flags ��ʶ���б�
     * @return true-�ɹ�, flalse-ʧ��
     */
    bool setCommentFlags(const std::vector<std::string>& flags);

    /**
     * @brief ɾ����
     * @param nameName ������
     * @return true-�ɹ�, flalse-ʧ��(������)
     */
    bool removeSection(const std::string& name);

    /**
     * @brief ���ý�ע��
     * @param name ������
     * @param comment ע��
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setSectionComment(const std::string& name, const std::string& comment);

    /**
     * @brief ɾ����
     * @param nameName ������
     * @param key ��
     * @return true-�ɹ�, flalse-ʧ��(������)
     */
    bool removeItem(const std::string& name, const std::string& key);

    /**
     * @brief ������ע��
     * @param name ������
     * @param key ��
     * @param comment ע��
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setComment(const std::string& name, const std::string& key, const std::string& comment);

    /**
     * @brief ���ò�����
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, bool value);

    /**
     * @brief ��������
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, int value);

    /**
     * @brief �����޷�������
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, unsigned int value);

    /**
     * @brief ���ó�����
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, long value);

    /**
     * @brief �����޷��ų�����
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, unsigned long value);

    /**
     * @brief ����64λ������
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, long long value);

    /**
     * @brief �����޷���64λ������
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, unsigned long long value);

    /**
     * @brief ���õ����ȸ�����
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, float value);

    /**
     * @brief ����˫���ȸ�����
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, double value);

    /**
     * @brief �����ַ���
     * @param name ������
     * @param key ��
     * @param value ֵ
     * @return 0-�ɹ�, 1-δ���, 2-��������, 3-�������Զ�����
     */
    int setValue(const std::string& name, const std::string& key, std::string value);

    /**
     * @brief �Ƿ���������/��
     * @return true-����, false-������
     */
    bool isAllowAutoCreate() const override;

private:
    bool m_allowAutoCreate = false; /* �Ƿ������Զ�������/�� */
};
} // namespace ini
