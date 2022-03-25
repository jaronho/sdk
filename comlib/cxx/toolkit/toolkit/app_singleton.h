#pragma once
#include <functional>
#include <string>

namespace toolkit
{
/**
 * @brief Ӧ�õ�����
 */
class AppSingleton final
{
private:
    AppSingleton() = default;

public:
    /**
     * @brief ����Ӧ�õ���, ��Ӧ�ó�������ڽ������ȵ���
     * @param pidFilePath pid�ļ�·��(ѡ��), Ĭ���ڳ����ļ�����Ŀ¼
     * @param pidFileName pid�ļ���(ѡ��), Ĭ��`������.pid`
     * @param exitFunc �����˳�ǰ�ص�(ѡ��)
     */
    static void create(const std::string& pidFilePath = "", const std::string& pidFileName = "",
                       const std::function<void()>& exitFunc = nullptr);
};
} // namespace toolkit
