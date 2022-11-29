#pragma once
#include <functional>
#include <string>

#include "fs_define.h"

namespace utility
{
class PathInfo
{
public:
    /**
     * @brief 构造函数
     * @param path 路径, 例如: /home/test
     * @param autoEndWithSlash 是否自动斜杠结尾(选填), 默认保留原始输入
     */
    PathInfo(const std::string& path, bool autoEndWithSlash = false);
    PathInfo() = default;
    virtual ~PathInfo() = default;

    /**
     * @brief 获取路径, 例如: /home/test/ 或 /home/test
     * @return 路径
     */
    std::string path() const;

    /**
     * @brief 获取子树路径, 例如: 全路径为/home/test/11/22/33/
     * @param parentPath 父路径, 例如: /home/test 或 test
     * @return 子树路径(头尾无斜杠), 例如: 11/22/33
     */
    std::string treePath(const std::string& parentPath) const;

    /**
     * @brief 判断路径是否斜杠结尾
     * @return true-是, false-否
     */
    bool isEndWithSlash() const;

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
     * @brief 创建路径(注: 执行后可能需要同步下磁盘I/O, 该操作耗时, linux下为"sync")
     * @return true-成功, false-失败
     */
    bool create() const;

    /**
     * @brief 删除路径(注: 执行后可能需要同步下磁盘I/O, 该操作耗时, linux下为"sync")
     * @return true-成功, false-失败
     */
    bool remove() const;

    /**
     * @brief 清空路径下的所有子项(注: 执行后可能需要同步下磁盘I/O, 该操作耗时, linux下为"sync")
     * @param continueIfRoot 如果路径是根目录是否继续清空(选填), 默认为不清空
     * @return true-成功, false-失败
     */
    bool clear(bool continueIfRoot = false) const;

    /**
     * @brief 判断是否为空文件夹
     * @return true-空, false-非空
     */
    bool empty() const;

    /**
     * @brief 遍历文件夹和文件
     * @param folderCb 文件夹回调, 参数: name-名称, attr-属性, depth-深度(从1开始), 返回值: true-进入子目录, false-不进入
     * @param fileCb 文件回调, 参数: name-名称, attr-属性, depth-深度(从1开始)
     * @param stopCb 停止回调, 返回值: true-停止, false-不停止
     * @param recursive 是否递归查找(选填), 默认递归
     * @param bfs 是否广度优先遍历(选填), true-是(广度优先遍历), false-否(深度优先遍历), 默认深度优先
     */
    void traverse(const std::function<bool(const std::string& name, const FileAttribute& attr, int depth)>& folderCb,
                  const std::function<void(const std::string& name, const FileAttribute& attr, int depth)>& fileCb,
                  const std::function<bool()>& stopCb, bool recursive = true, bool bfs = false) const;

    /**
     * @brief 校正路径(去除多余的斜杠, 反斜杠, 左右空格)
     * @return 新的路径
     */
    static std::string revise(const std::string& path);

    /**
     * @brief 获取当前工作目录
     * @param autoEndWithSlash 是否自动斜杠结尾(选填), 默认保留原始
     * @return 当前工作目录
     */
    static std::string getcwd(bool autoEndWithSlash = false);

private:
    /**
     * @brief 清空内部实现
     * @param path 路径
     * @param rmSelf true-删除path, false-只清空path
     * @return true-成功, false-失败
     */
    static bool clearImpl(std::string path, bool rmSelf);

    /**
     * @brief 广度优先遍历文件夹和文件内部实现
     * @param path 路径
     * @param depth 深度(首次调用传入0)
     * @param folderCb 文件夹回调, 参数: name-名称, attr-属性, depth-深度(从1开始), 返回值: true-允许遍历子目录, false-不允许
     * @param fileCb 文件回调, 参数: name-名称, attr-属性, depth-深度(从1开始)
     * @param stopCb 停止回调, 返回值: true-停止, false-不停止
     * @param recursive 是否递归查找
     */
    static void traverseBFS(const std::string& path, int depth,
                            const std::function<bool(const std::string& name, const FileAttribute& attr, int depth)>& folderCb,
                            const std::function<void(const std::string& name, const FileAttribute& attr, int depth)>& fileCb,
                            const std::function<bool()>& stopCb, bool recursive);

    /**
     * @brief 深度优先遍历文件夹和文件内部实现
     * @param path 路径
     * @param depth 深度(首次调用传入0)
     * @param folderCb 文件夹回调, 参数: name-名称, attr-属性, depth-深度(从1开始), 返回值: true-允许遍历子目录, false-不允许
     * @param fileCb 文件回调, 参数: name-名称, attr-属性, depth-深度(从1开始)
     * @param stopCb 停止回调, 返回值: true-停止, false-不停止
     * @param recursive 是否递归查找
     */
    static void traverseDFS(std::string path, int depth,
                            const std::function<bool(const std::string& name, const FileAttribute& attr, int depth)>& folderCb,
                            const std::function<void(const std::string& name, const FileAttribute& attr, int depth)>& fileCb,
                            const std::function<bool()>& stopCb, bool recursive);

private:
    std::string m_path; /* 路径 */
};
} // namespace utility
