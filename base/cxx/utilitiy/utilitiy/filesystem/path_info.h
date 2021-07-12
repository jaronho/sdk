#pragma once
#include <string>

namespace utilitiy
{
class PathInfo
{
public:
    /**
     * @brief 构造函数
     * @param path 路径, 例如: /home/test
     */
    PathInfo(const std::string& path);

    /**
     * @brief 获取路径, 例如: /home/test/
     * @return 路径
     */
    std::string path();

    /**
     * @brief 判断路径是否为绝对路径
     * @return true-绝对路径, false-非绝对路径(相对路径)
     */
    bool isAbsolute();

    /**
     * @brief 判断路径是否为根路径
     * @return true-根路径, false-非根路径
     */
    bool isRoot();

    /**
     * @brief 判断路径是否存在
     * @return true-存在, false-不存在
     */
    bool exist();

    /**
     * @brief 创建路径
     * @return true-成功, false-失败
     */
    bool create();

    /**
     * @brief 删除路径
     * @return true-成功, false-失败
     */
    bool remove();

    /**
     * @brief 清空路径下的所有子项
     * @return true-成功, false-失败
     */
    bool clear();

    /**
     * @brief 校正路径(去除多余的斜杠, 反斜杠, 左右空格)
     * @return 新的路径
     */
    static std::string revise(const std::string& path);

private:
    /**
     * @brief 清空内部实现
     * @param path 路径
     * @param rmSelf true-删除path, false-只清空path
     * @return true-成功, false-失败
     */
    bool clearImpl(const std::string& path, bool rmSelf);

private:
    std::string m_path; /* 路径 */
};
} // namespace utilitiy
