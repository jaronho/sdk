#pragma once
#include <string>
#include <time.h>

namespace utilitiy
{
/**
 * @brief 文件属性 
 */
typedef struct
{
    std::string createTimeFmt(const std::string& fmtStr = "%Y-%m-%d %H:%M:%S")
    {
        char str[64] = {0};
        strftime(str, sizeof(str), fmtStr.c_str(), localtime(&createTime));
        return str;
    };

    std::string modifyTimeFmt(const std::string& fmtStr = "%Y-%m-%d %H:%M:%S")
    {
        char str[64] = {0};
        strftime(str, sizeof(str), fmtStr.c_str(), localtime(&modifyTime));
        return str;
    };

    std::string accessTimeFmt(const std::string& fmtStr = "%Y-%m-%d %H:%M:%S")
    {
        char str[64] = {0};
        strftime(str, sizeof(str), fmtStr.c_str(), localtime(&accessTime));
        return str;
    };

    time_t createTime = 0; /* 创建时间(1900年至今的秒数) */
    time_t modifyTime = 0; /* 修改时间(1900年至今的秒数) */
    time_t accessTime = 0; /* 访问时间(1900年至今的秒数) */
    long long size = 0; /* 文件大小(注:为目录时该字段无效) */
    bool isDir; /* 是否目录 */
    bool isFile; /* 是否文件 */
#ifdef _WIN32
    bool isSystem; /* 是否系统文件 */
#endif
    bool isSymLink; /* 是否链接文件 */
    bool isHidden; /* 是否隐藏 */
    bool isWritable; /* 是否可写 */
    bool isExecutable; /* 是否可执行 */
} FileAttribute;

/**
 * @brief 获取文件(目录)属性
 * @param name 文件名
 * @param attr [输出]属性
 * @return true-成功, false-失败
 */
bool getFileAttribute(std::string name, FileAttribute& attr);
} // namespace utilitiy
