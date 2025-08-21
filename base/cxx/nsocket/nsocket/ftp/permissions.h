#pragma once

namespace nsocket
{
namespace ftp
{
/**
 * @brief Ȩ������
 */
enum class Permission
{
    None = 0, /* �� */

    FileRead = (1 << 0), /* �����ļ� */
    FileWrite = (1 << 1), /* �ϴ��ļ� */
    FileAppend = (1 << 2), /* �����ļ� */
    FileDelete = (1 << 3), /* ɾ���ļ� */
    FileRename = (1 << 4), /* �������ļ� */

    DirList = (1 << 5), /* ����Ŀ¼ */
    DirCreate = (1 << 6), /* ����Ŀ¼ */
    DirDelete = (1 << 7), /* ɾ��Ŀ¼ */
    DirRename = (1 << 8), /* ������Ŀ¼ */

    ReadOnly = (FileRead | DirList), /* ֻ�� */

    All = (FileRead | FileWrite | FileAppend | FileDelete | FileRename | DirList | DirCreate | DirDelete | DirRename) /* ���� */

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
