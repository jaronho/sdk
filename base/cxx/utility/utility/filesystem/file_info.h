#pragma once
#include <fstream>
#include <functional>
#include <string>

#include "fs_define.h"

namespace utilitiy
{
class FileInfo
{
public:
    enum class CopyResult
    {
        ok, /* 成功 */
        src_open_failed, /* 源文件打开失败 */
        dest_open_failed, /* 目标文件打开失败 */
        memory_alloc_failed, /* 内存分配失败 */
        stop, /* 停止拷贝 */
        size_unequal /* 源文件和目标文件大小不一致 */
    };

public:
    /**
     * @brief 构造函数
     * @param fullName 全路径文件名, 例如: /home/test/111.txt
     */
    FileInfo(const std::string& fullName);
    FileInfo() = default;
    virtual ~FileInfo() = default;

    /**
     * @brief 获取全路径文件名
     * @return 全路径文件名
     */
    std::string name() const;

    /**
     * @brief 获取文件路径, 例如: /home/test/
     * @return 文件路径
     */
    std::string path() const;

    /**
     * @brief 获取文件名, 例如: 111.txt
     * @return 文件名
     */
    std::string filename() const;

    /**
     * @brief 获取文件基础名, 例如: 111
     * @return 文件基础名
     */
    std::string basename() const;

    /**
     * @brief 获取文件扩展名(不包含.), 例如: txt
     * @return 文件扩展名
     */
    std::string extname() const;

    /**
     * @brief 判断文件扩展名是否一致(不区分大小写和.)
     * @param extName 文件扩展名, 例如: txt 或 .txt 或 TXT 或 .TXT 等
     * @return true-一致, false-不一致
     */
    bool isExtName(std::string extName) const;

    /**
     * @brief 获取文件属性
     * @return 属性
     */
    FileAttribute attribute() const;

    /**
     * @brief 判断文件是否存在
     * @return true-存在, false-不存在
     */
    bool exist() const;

    /**
     * @brief 创建文件
     * @return true-成功, false-失败
     */
    bool create() const;

    /**
     * @brief 删除文件(注: 执行后可能需要同步下磁盘I/O, 该操作耗时, linux下为"sync")
     * @return true-成功, false-失败
     */
    bool remove() const;

    /**
     * @brief 拷贝文件
     * @param destFilename 目标文件(全路径)
     * @param errCode [输出]错误码(选填), 可用于strerror函数获取描述信息
     * @param progressCb 进度回调, 参数: now-已拷贝字节数, total-总字节数, 返回值: true-继续, false-停止拷贝
     * @param maxBlockSize 设置拷贝块的最大单位(字节), 为0时表示不限制
     * @return 拷贝结果
     */
    CopyResult copy(const std::string& destFilename, int* errCode = nullptr,
                    const std::function<bool(size_t now, size_t total)>& progressCb = nullptr, size_t maxBlockSize = 0) const;

    /**
     * @brief 文件大小
     * @return -1-文件不存在, >=0-文件大小
     */
    long long size() const;

    /**
     * @brief 获取文件数据
     * @param fileSize [输出]文件大小, -1-文件不存在, >=0-文件大小
     * @param textFlag true-文本, false-二进制
     * @return 数据(需要外部调用free释放内存)
     */
    char* data(long long& fileSize, bool textFlag = false) const;

    /**
     * @brief 读取文件数据
     * @param offset 读取的偏移值, 为0时表示从头开始
     * @param count [输入/输出]要读取的字节数(返回实际读取的字节数)
     * @param textFlag true-文本, false-二进制
     * @return 数据(需要外部调用free释放内存)
     */
    char* read(size_t offset, size_t& count, bool textFlag = false) const;

    /**
     * @brief 判断是否为文本文件
     * @return true-文本文件, false-非文本文件
     */
    bool isTextFile() const;

    /**
     * @brief 写文件数据
     * @param data 数据
     * @param length 数据长度
     * @param isAppend true-在文件末尾追加, false-替换全部
     * @param errCode [输出]错误码(选填), 可用于strerror函数获取描述信息
     * @return true-成功, false-失败
     */
    bool write(const char* data, size_t length, bool isAppend = false, int* errCode = nullptr) const;

    /**
     * @brief 写文件数据
     * @param pos 写入的位置, 说明: 若写入位置大于原文件长度, 则原文件末尾到写入位置会被NUL占位
     * @param data 数据
     * @param length 数据长度
     * @param errCode [输出]错误码(选填), 可用于strerror函数获取描述信息
     * @return true-成功, false-失败
     */
    bool write(size_t pos, const char* data, size_t length, int* errCode = nullptr);

    /**
     * @brief 从文件流中读取数据
     * @param f 文件流
     * @param offset 读取的偏移值, 为0时表示从头开始
     * @param count [输入/输出]要读取的字节数(返回实际读取的字节数)
     * @param textFlag true-文本, false-二进制
     * @return 数据(需要外部调用free释放内存)
     */
    static char* read(std::fstream& f, size_t offset, size_t& count, bool textFlag = false);

    /**
     * @brief 判断文件流是否为文本数据
     * @param f 文件流
     * @return true-文本数据, false-二进制数据
     */
    static bool isTextData(std::fstream& f);

private:
    std::string m_fullName; /* 全路径文件名 */
    std::string m_path; /* 路径 */
    std::string m_filename; /* 文件名 */
    std::string m_basename; /* 基础名 */
    std::string m_extname; /* 扩展名 */
};
} // namespace utilitiy
