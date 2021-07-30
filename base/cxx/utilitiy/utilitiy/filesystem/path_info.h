#pragma once
#include <functional>
#include <string>

namespace utilitiy
{
typedef struct
{
    long createTime; /* 创建时间 */
    long modifyTime; /* 修改时间 */
    long accessTime; /* 访问时间 */
#if _WIN32
    bool isReadOnly; /* 是否只读 */
    bool isHidden; /* 是否隐藏 */
    bool isSystem; /* 是否系统文件 */
    bool isSymLink; /* 是否快捷文件 */
#endif
} FileAttribure;

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
     * @brief 赋值拷贝
     * @return 对象自身
     */
    PathInfo& operator=(const PathInfo& src);

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
     * @param ioSync 是否同步I/O, 会降低效率(选填)
     * @return true-成功, false-失败
     */
    bool create(bool ioSync = false);

    /**
     * @brief 删除路径
     * @param ioSync 是否同步I/O, 会降低效率(选填)
     * @return true-成功, false-失败
     */
    bool remove(bool ioSync = false);

    /**
     * @brief 清空路径下的所有子项
     * @param continueIfRoot 如果路径是根目录是否继续清空(选填), 默认为不清空
     * @param ioSync 是否同步I/O, 会降低效率(选填)
     * @return true-成功, false-失败
     */
    bool clear(bool continueIfRoot = false, bool ioSync = false);

    /**
     * @brief 遍历文件夹和文件
     * @param folderCallback 文件夹回调, name-名称, attr-属性
     * @param fileCallback 文件回调, name-名称, attr-属性, size-文件大小
     * @param recursive 是否递归查找(选填), 默认递归
     */
    void traverse(std::function<void(const std::string& name, const FileAttribure& attr)> folderCallback,
                  std::function<void(const std::string& name, const FileAttribure& attr, long long size)> fileCallback,
                  bool recursive = true);

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

    /**
     * @brief 遍历文件夹和文件内部实现
     * @param path 路径
     * @param folderCallback 文件夹回调, name-名称, attr-属性
     * @param fileCallback 文件回调, name-名称, attr-属性, size-文件大小
     * @param recursive 是否递归查找
     */
    void traverseImpl(std::string path, std::function<void(const std::string& name, const FileAttribure& attr)> folderCallback,
                      std::function<void(const std::string& name, const FileAttribure& attr, long long size)> fileCallback, bool recursive);

private:
    std::string m_path; /* 路径 */
};
} // namespace utilitiy
