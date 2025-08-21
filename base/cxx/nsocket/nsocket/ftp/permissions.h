#pragma once

namespace nsocket
{
namespace ftp
{
/**
 * @brief 权限类型
 */
enum class Permission
{
    None = 0, /* 无 */

    FileRead = (1 << 0), /* 下载文件 */
    FileWrite = (1 << 1), /* 上传文件 */
    FileAppend = (1 << 2), /* 附加文件 */
    FileDelete = (1 << 3), /* 删除文件 */
    FileRename = (1 << 4), /* 重命名文件 */

    DirList = (1 << 5), /* 检索目录 */
    DirCreate = (1 << 6), /* 创建目录 */
    DirDelete = (1 << 7), /* 删除目录 */
    DirRename = (1 << 8), /* 重命名目录 */

    ReadOnly = (FileRead | DirList), /* 只读 */

    All = (FileRead | FileWrite | FileAppend | FileDelete | FileRename | DirList | DirCreate | DirDelete | DirRename) /* 所有 */

};

inline Permission operator~(Permission a)
{
    return static_cast<Permission>(~static_cast<int>(a));
}

inline Permission operator|(Permission a, Permission b)
{
    return static_cast<Permission>(static_cast<int>(a) | static_cast<int>(b));
}

inline Permission operator&(Permission a, Permission b)
{
    return static_cast<Permission>(static_cast<int>(a) & static_cast<int>(b));
}

inline Permission operator^(Permission a, Permission b)
{
    return static_cast<Permission>(static_cast<int>(a) ^ static_cast<int>(b));
}

inline Permission& operator|=(Permission& a, Permission b)
{
    return reinterpret_cast<Permission&>(reinterpret_cast<int&>(a) |= static_cast<int>(b));
}

inline Permission& operator&=(Permission& a, Permission b)
{
    return reinterpret_cast<Permission&>(reinterpret_cast<int&>(a) &= static_cast<int>(b));
}

inline Permission& operator^=(Permission& a, Permission b)
{
    return reinterpret_cast<Permission&>(reinterpret_cast<int&>(a) ^= static_cast<int>(b));
}
} // namespace ftp
} // namespace nsocket
