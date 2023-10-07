#pragma once
#include <functional>
#include <stdio.h>
#include <string>
#include <vector>

#include "fs_define.h"

namespace utility
{
/*
 说明: 对于文件的操作, FILE* 会比 std::fstream 效率高, 特别是写文件
 (1)FILE 文件操作模式说明
  ------------------------------------------------------------------------------------------
 |  模式  |                 含义                 |             说明                         |
 |--------|--------------------------------------|------------------------------------------|
 |    r   | 打开文件进行读操作                   | 文件不存在时不会创建                     |
 |--------|--------------------------------------|------------------------------------------|
 |    w   | 打开文件进行写操作                   | 文件不存在时创建新文件, 反之清空文件内容 |
 |--------|--------------------------------------|------------------------------------------|
 |    a   | 打开文件并定位到末尾进行写操作(追加) | 文件不存在时创建新文件                   |
 |--------|--------------------------------------|------------------------------------------|
 |    b   | 指定文件内容为二进制方式             | 文件不存在时不会创建                     |
 |--------|--------------------------------------|------------------------------------------|
 |    +   | 指定文件支持读写模式                 | 文件不存在时不会创建                     |
  ------------------------------------------------------------------------------------------
 (2)std::fstream 文件操作模式说明
  ----------------------------------------------------------------------------------------------------
 |       模式       |                 含义                 |                   说明                   |
 |------------------|--------------------------------------|------------------------------------------|
 | std::ios::in     | 打开文件进行读操作                   | 文件不存在时不会创建                     |
 |------------------|--------------------------------------|------------------------------------------|
 | std::ios::out    | 打开文件进行写操作                   | 文件不存在时创建新文件, 反之清空文件内容 |
 |------------------|--------------------------------------|------------------------------------------|
 | std::ios::in |   | 打开文件进行读写操作                 | 文件不存在时不会创建                     |
 | std::ios::out    |                                      |                                          |
 |------------------|--------------------------------------|------------------------------------------|
 | std::ios::app    | 打开文件并定位到末尾进行写操作(追加) | 文件不存在时创建新文件                   |
 |------------------|--------------------------------------|------------------------------------------|
 | std::ios::ate    | 打开文件并定位到末尾                 | 文件不存在时不会创建                     |
 |------------------|--------------------------------------|------------------------------------------|
 | std::ios::trunc  | 打开文件若文件已存在则清空文件内容   | 文件不存在时不会创建                     |
 |------------------|--------------------------------------|------------------------------------------|
 | std::ios::binary | 以二进制方式打开文件进行读写         | 文件不存在时不会创建                     |
  ----------------------------------------------------------------------------------------------------
 */
class FileInfo
{
public:
    enum class CopyResult
    {
        ok, /* 成功 */
        src_open_failed, /* 源文件打开失败 */
        dest_open_failed, /* 目标文件打开失败 */
        memory_alloc_failed, /* 内存分配失败 */
        src_read_failed, /* 源文件读失败 */
        dest_write_failed, /* 目标文件写失败 */
        stop, /* 停止拷贝 */
        size_unequal /* 源文件和目标文件大小不一致 */
    };

    /**
     * @brief 文件拷贝块, 拷贝大于指定大小的文件时, 指定拷贝内存块大小
     */
    struct CopyBlock
    {
        CopyBlock() : fileSize(0), blockSize(0) {}

        size_t fileSize; /* 文件大小(单位:字节) */
        size_t blockSize; /* 块大小(单位:字节) */
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
     * @brief 获取文件路径, 例如: /home/test 或 /home/test/
     * @param endWithSlash 是否斜杠结尾(选填), 默认斜杠结尾
     * @return 文件路径
     */
    std::string path(bool endWithSlash = true) const;

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
     * @param blocks 拷贝块大小, 为空时表示使用默认(最大64Kb)
     * @param retryTime 读写失败时重试时间(毫秒)
     * @return 拷贝结果
     */
    CopyResult copy(const std::string& destFilename, int* errCode = nullptr,
                    const std::function<bool(size_t now, size_t total)>& progressCb = nullptr,
                    const std::vector<FileInfo::CopyBlock>& blocks = {}, unsigned int retryTime = 3000) const;

    /**
     * @brief 文件大小
     * @return -1-文件不存在, >=0-文件大小
     */
    long long size() const;

    /**
     * @brief 获取文件所有数据
     * @param fileSize [输出]文件大小, -1-文件不存在, >=0-文件大小
     * @param textFlag true-文本, false-二进制
     * @return 数据(需要外部调用free释放内存)
     */
    char* readAll(long long& fileSize, bool textFlag = false) const;

    /**
     * @brief 获取文件所有数据
     * @param textFlag true-文本, false-二进制
     * @return 数据
     */
    std::string readAll(bool textFlag = false) const;

    /**
     * @brief 读取文件数据
     * @param offset 读取的偏移值, 为0时表示从头开始
     * @param count [输入/输出]要读取的字节数(返回实际读取的字节数)
     * @param textFlag true-文本, false-二进制
     * @return 数据(需要外部调用free释放内存)
     */
    char* read(size_t offset, size_t& count, bool textFlag = false) const;

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
     * @param data 数据
     * @param isAppend true-在文件末尾追加, false-替换全部
     * @param errCode [输出]错误码(选填), 可用于strerror函数获取描述信息
     * @return true-成功, false-失败
     */
    bool write(const std::string& data, bool isAppend = false, int* errCode = nullptr) const;

    /**
     * @brief 写文件数据
     * @param pos 写入的位置, 说明: 若写入位置大于原文件长度, 则原文件末尾到写入位置会被NUL占位
     * @param data 数据
     * @param length 数据长度
     * @param errCode [输出]错误码(选填), 可用于strerror函数获取描述信息
     * @return true-成功, false-失败
     */
    bool write(size_t pos, const char* data, size_t length, int* errCode = nullptr) const;

    /**
     * @brief 写文件数据
     * @param pos 写入的位置, 说明: 若写入位置大于原文件长度, 则原文件末尾到写入位置会被NUL占位
     * @param data 数据
     * @param errCode [输出]错误码(选填), 可用于strerror函数获取描述信息
     * @return true-成功, false-失败
     */
    bool write(size_t pos, const std::string& data, int* errCode = nullptr) const;

    /**
     * @brief 编辑文件中的数据
     * @param offset 指定的偏移值, 为0时表示从头开始
     * @param count 指定字节数
     * @param func 编辑函数(对buffer进行编辑, 注意: 不要改变buffer长度), 参数: buffer-读到的数据, count-读到的数据长度
     * @return true-成功, false-失败
     */
    bool edit(size_t offset, size_t count, const std::function<void(char* buffer, size_t count)>& func) const;

    /**
     * @brief 编辑文本文件中的行数据
     * @param func 编辑函数, 参数: lineNum-行号, line-行数据, 返回值: true-保留该行, false-删除该行
     * @return true-成功, false-失败
     */
    bool editLine(const std::function<bool(size_t num, std::string& line)>& func) const;

    /**
     * @brief 判断是否为文本文件
     * @return true-文本文件, false-非文本文件
     */
    bool isTextFile() const;

    /**
     * @brief 从文件中读取数据
     * @param f 文件指针
     * @param offset 读取的偏移值, 为0时表示从头开始
     * @param count [输入/输出]要读取的字节数(返回实际读取的字节数), 为0时表示读取所有
     * @param textFlag true-文本, false-二进制
     * @return 数据(需要外部调用free释放内存)
     */
    static char* read(FILE* f, size_t offset, size_t& count, bool textFlag);

    /**
     * @brief 从文本文件中读取一行
     * @param f 文件指针
     * @param line [输出]行数据
     * @param bomFlag [输出]BOM标识(3个字节), 如果文件为UTF-8带BOM编码则第1行有此标识
     * @param endFlag [输出]行结束标识, 一般为 \r\n 或 \n
     * @return true-成功, false-失败
     */
    static bool readLine(FILE* f, std::string& line, std::string& bomFlag, std::string& endFlag);

    /**
     * @brief 向文件中写入数据
     * @param f 文件指针
     * @param offset 写入的偏移值, 为0时表示从头开始
     * @param data 数据
     * @param count 要写入的字节数
     * @return true-成功, false-失败
     */
    static bool write(FILE* f, size_t offset, const char* data, size_t count);

    /**
     * @brief 向文件中写入数据
     * @param f 文件指针
     * @param offset 写入的偏移值, 为0时表示从头开始
     * @param data 数据
     * @return true-成功, false-失败
     */
    static bool write(FILE* f, size_t offset, const std::string& data);

    /**
     * @brief 编辑文件中的数据
     * @param f 文件指针
     * @param offset 指定的偏移值, 为0时表示从头开始
     * @param count 指定字节数
     * @param func 编辑函数(对buffer进行编辑, 注意: 不要改变buffer长度), 参数: buffer-读到的数据, count-读到的数据长度
     * @return true-成功, false-失败
     */
    static bool edit(FILE* f, size_t offset, size_t count, const std::function<void(char* buffer, size_t count)>& func);

    /**
     * @brief 判断文件是否为文本数据
     * @param f 文件指针
     * @return true-文本数据, false-二进制数据
     */
    static bool isTextData(FILE* f);

private:
    std::string m_fullName; /* 全路径文件名 */
    std::string m_path; /* 路径 */
    std::string m_filename; /* 文件名 */
    std::string m_basename; /* 基础名 */
    std::string m_extname; /* 扩展名 */
};
} // namespace utility
