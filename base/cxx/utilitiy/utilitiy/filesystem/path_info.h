#pragma once
#include <functional>
#include <string>

#include "fs_define.h"

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
    PathInfo() = default;
    virtual ~PathInfo() = default;

    /**
     * @brief 获取路径, 例如: /home/test/
     * @return 路径
     */
    std::string path() const;

    /**
     * @brief 判断路径是否为绝对路径
     * @return true-绝对路径, false-非绝对路径(相对路径)
     */
    bool isAbsolute() const;

    /**
     * @brief 判断路径是否为根路径
     * @return true-根路径, false-非根路径
     */
    bool isRoot() const;

    /**
     * @brief 获取目录属性
     * @return 属性
     */
    FileAttribute attribute() const;

    /**
     * @brief 判断路径是否存在
     * @return true-存在, false-不存在
     */
    bool exist() const;

    /**
     * @brief 创建路径
     * @param ioSync 是否同步I/O, 会降低效率(选填)
     * @return true-成功, false-失败
     */
    bool create(bool ioSync = false) const;

    /**
     * @brief 删除路径
     * @param ioSync 是否同步I/O, 会降低效率(选填)
     * @return true-成功, false-失败
     */
    bool remove(bool ioSync = false) const;

    /**
     * @brief 清空路径下的所有子项
     * @param continueIfRoot 如果路径是根目录是否继续清空(选填), 默认为不清空
     * @param ioSync 是否同步I/O, 会降低效率(选填)
     * @return true-成功, false-失败
     */
    bool clear(bool continueIfRoot = false, bool ioSync = false) const;

    /**
     * @brief 遍历文件夹和文件
     * @param folderCallback 文件夹回调, 参数: name-名称, attr-属性, depth-深度(从1开始), 返回值: true-允许遍历子目录, false-不允许
     * @param fileCallback 文件回调, 参数: name-名称, attr-属性, depth-深度(从1开始)
     * @param recursive 是否递归查找(选填), 默认递归
     */
    void traverse(std::function<bool(const std::string& name, const FileAttribute& attr, int depth)> folderCallback,
                  std::function<void(const std::string& name, const FileAttribute& attr, int depth)> fileCallback,
                  bool recursive = true) const;

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
    static bool clearImpl(std::string path, bool rmSelf);

    /**
     * @brief 遍历文件夹和文件内部实现
     * @param path 路径
     * @param depth 深度(首次调用传入0)
     * @param folderCallback 文件夹回调, 参数: name-名称, attr-属性, depth-深度(从1开始), 返回值: true-允许遍历子目录, false-不允许
     * @param fileCallback 文件回调, 参数: name-名称, attr-属性, depth-深度(从1开始)
     * @param recursive 是否递归查找
     */
    static void traverseImpl(std::string path, int depth,
                             std::function<bool(const std::string& name, const FileAttribute& attr, int depth)> folderCallback,
                             std::function<void(const std::string& name, const FileAttribute& attr, int depth)> fileCallback,
                             bool recursive);

private:
    std::string m_path; /* 路径 */
};
} // namespace utilitiy
