#pragma once
#include <functional>
#include <string>

namespace utilitiy
{
class FileInfo
{
public:
    /**
     * @brief 构造函数
     * @param fullName 全路径文件名, 例如: /home/test/111.txt
     */
    FileInfo(const std::string& fullName);

    /**
     * @brief 获取全路径文件名
     * @return 全路径文件名
     */
    std::string name();

    /**
     * @brief 获取文件路径, 例如: /home/test/
     * @return 文件路径
     */
    std::string path();

    /**
     * @brief 获取文件名, 例如: 111.txt
     * @return 文件名
     */
    std::string filename();

    /**
     * @brief 获取文件基础名, 例如: 111
     * @return 文件基础名
     */
    std::string basename();

    /**
     * @brief 获取文件扩展名(不包含.), 例如: txt
     * @return 文件扩展名
     */
    std::string extname();

    /**
     * @brief 判断文件是否存在
     * @return true-存在, false-不存在
     */
    bool exist();

    /**
     * @brief 创建文件
     * @return true-成功, false-失败
     */
    bool create();

    /**
     * @brief 删除文件
     * @return true-成功, false-失败
     */
    bool remove();

    /**
     * @brief 拷贝文件
     * @param destFilename 目标文件(全路径)
     * @param progressCb 进度回调, now-已拷贝字节数, total-总字节数
     * @param blockMaxSize 设置拷贝块的最大单位(字节), 为0时表示不限制
     * @return true-成功, false-失败
     */
    bool copy(const std::string& destFilename, const std::function<void(size_t now, size_t total)>& progressCb = nullptr,
              size_t blockMaxSize = 0);

    /**
     * @brief 文件大小
     * @return -1-文件不存在, >=0-文件大小
     */
    long long size();

    /**
     * @brief 文件数据
     * @param fileSize [输出]文件大小
     * @param isText true-文本文件, false-二进制文件
     * @return 数据(要外部调用free释放内存)
     */
    char* data(long long& fileSize, bool isText = false);

private:
    std::string m_fullName; /* 全路径文件名 */
    std::string m_path; /* 路径 */
    std::string m_filename; /* 文件名 */
    std::string m_basename; /* 基础名 */
    std::string m_extname; /* 扩展名 */
};
} // namespace utilitiy
