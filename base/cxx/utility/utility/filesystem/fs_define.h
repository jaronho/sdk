#pragma once
#include <string>
#include <time.h>

namespace utility
{
/**
 * @brief 磁盘属性
 */
typedef struct
{
    size_t blockSize = 0; /* 块大小(单位: 字节), Windows下称为簇, 簇大小=每簇扇区数*每扇区字节数 */
    size_t totalBlock = 0; /* 磁盘总块数, 磁盘总大小=总块数*块大小 */
    size_t freeBlock = 0; /* 磁盘空闲块数, 磁盘空闲大小=空闲块数*块大小 */
} DiskAttribute;

/**
 * @brief 文件(目录)属性
 */
typedef struct
{
    std::string createTimeFmt(const std::string& fmtStr = "%Y-%m-%d %H:%M:%S") const
    {
        char str[64] = {0};
        strftime(str, sizeof(str), fmtStr.c_str(), localtime(&createTime));
        return str;
    };

    std::string modifyTimeFmt(const std::string& fmtStr = "%Y-%m-%d %H:%M:%S") const
    {
        char str[64] = {0};
        strftime(str, sizeof(str), fmtStr.c_str(), localtime(&modifyTime));
        return str;
    };

    std::string accessTimeFmt(const std::string& fmtStr = "%Y-%m-%d %H:%M:%S") const
    {
        char str[64] = {0};
        strftime(str, sizeof(str), fmtStr.c_str(), localtime(&accessTime));
        return str;
    };

    time_t createTime = 0; /* 创建时间(1900年至今的秒数) */
    time_t modifyTime = 0; /* 修改时间(1900年至今的秒数) */
    time_t accessTime = 0; /* 访问时间(1900年至今的秒数) */
    size_t size = 0; /* 文件大小(注:为目录时该字段无效) */
    bool isDir; /* 是否目录 */
    bool isFile; /* 是否文件 */
    bool isSymLink; /* 是否链接文件 */
#ifdef _WIN32
    bool isSystem; /* 是否系统文件 */
#endif
    bool isHidden; /* 是否隐藏 */
    bool isWritable; /* 是否可写 */
    bool isExecutable; /* 是否可执行 */
} FileAttribute;

/**
 * @brief 获取磁盘属性
 * @param name 磁盘目录名(挂载点), Linux平台如: "/mnt/udisk", Windows平台如: "D:\\"
 * @param attr [输出]属性
 * @return true-成功, false-失败
 */
bool getDiskAttribute(const std::string& name, DiskAttribute& attr);

/**
 * @brief 获取文件(目录)属性
 * @param name 文件(目录)名
 * @param attr [输出]属性
 * @return true-成功, false-失败
 */
bool getFileAttribute(const std::string& name, FileAttribute& attr);

/**
 * @brief 是否有效的文件(目录)名
 * @param name 文件(目录)名
 * @param platformType 判断平台类型: 0-所有, 1-Windows, 2-Linux
 * @return true-有效, false-无效
 */
bool isValidFilename(std::string name, int platformType = 0);
} // namespace utility
