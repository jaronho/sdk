#pragma once
#include <string>

namespace utilitiy
{
/**
 * @brief 文件属性 
 */
typedef struct
{
    long createTime = 0; /* 创建时间 */
    long modifyTime = 0; /* 修改时间 */
    long accessTime = 0; /* 访问时间 */
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
bool getFileAttribute(const std::string& name, FileAttribute& attr);
} // namespace utilitiy
