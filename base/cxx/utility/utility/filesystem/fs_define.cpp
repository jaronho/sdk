#include "fs_define.h"

#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <Shlobj.h>
#include <atlstr.h>
#include <shellapi.h>
#else
#include <sys/statfs.h>
#endif

namespace utility
{
#ifdef _WIN32
static std::wstring string2wstring(const std::string& str)
{
    if (str.empty())
    {
        return std::wstring();
    }
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    wchar_t* buf = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buf, len);
    buf[len] = '\0';
    std::wstring wstr(buf);
    free(buf);
    return wstr;
}
#endif

bool getFileAttribute(const std::string& name, FileAttribute& attr)
{
    memset(&attr, 0, sizeof(FileAttribute));
    if (name.empty() || "." == name || ".." == name)
    {
        return false;
    }
    auto temp = name;
    const char& lastChar = temp[temp.size() - 1];
    if (temp.size() > 1 && ('/' == lastChar || '\\' == lastChar))
    {
        temp.pop_back();
    }
    auto subName = temp.substr(temp.find_last_of("/\\") + 1, temp.size());
    if ("." == subName || ".." == subName)
    {
        return false;
    }
#ifdef _WIN32
    struct _stat64 st;
#ifdef UNICODE
    int ret = _wstat64(string2wstring(name).c_str(), &st);
#else
    int ret = _stat64(name.c_str(), &st);
#endif
#else
    struct stat64 st;
    int ret = stat64(name.c_str(), &st);
#endif
    if (0 != ret)
    {
        return false;
    }
    attr.createTime = st.st_ctime;
    attr.modifyTime = st.st_mtime;
    attr.accessTime = st.st_atime;
    attr.size = st.st_size;
    attr.isDir = S_IFDIR & st.st_mode;
    attr.isFile = S_IFREG & st.st_mode;
#ifdef _WIN32
    SHFILEINFO shFileInfo;
    memset(&shFileInfo, 0, sizeof(SHFILEINFO));
#ifdef UNICODE
    SHGetFileInfo(string2wstring(name).c_str(), 0, &shFileInfo, sizeof(SHFILEINFO),
                  SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_ATTRIBUTES);
#else
    SHGetFileInfo(name.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO),
                  SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_ATTRIBUTES);
#endif
    attr.isSystem = SFGAO_SYSTEM & shFileInfo.dwAttributes;
    attr.isSymLink = SFGAO_LINK & shFileInfo.dwAttributes;
    attr.isHidden = SFGAO_HIDDEN & shFileInfo.dwAttributes;
    attr.isWritable = !(SFGAO_READONLY & shFileInfo.dwAttributes);
#else
    attr.isSymLink = S_IFLNK & st.st_mode;
    attr.isHidden = subName.empty() ? false : '.' == subName[0]; /* linux中文件名第1个字符为.表示隐藏 */
    attr.isWritable = S_IWUSR & st.st_mode;
#endif
    attr.isExecutable = S_IEXEC & st.st_mode;
    return true;
}

bool getDiskAttribute(const std::string& name, DiskAttribute& attr)
{
    memset(&attr, 0, sizeof(DiskAttribute));
    if (name.empty() || "." == name || ".." == name)
    {
        return false;
    }
    auto temp = name;
    const char& lastChar = temp[temp.size() - 1];
    if (temp.size() > 1 && ('/' == lastChar || '\\' == lastChar))
    {
        temp.pop_back();
    }
    auto subName = temp.substr(temp.find_last_of("/\\") + 1, temp.size());
    if ("." == subName || ".." == subName)
    {
        return false;
    }
#ifdef _WIN32
    DWORD dwSectPerClust = 0, dwbytesPerSect = 0, dwFreeClusters = 0, dwTotalClusters = 0;
    BOOL result = FALSE;
#ifdef UNICODE
    result = GetDiskFreeSpaceW(string2wstring(name).c_str(), &dwSectPerClust, &dwbytesPerSect, &dwFreeClusters, &dwTotalClusters);
#else
    result = GetDiskFreeSpaceA(name.c_str(), &dwSectPerClust, &dwbytesPerSect, &dwFreeClusters, &dwTotalClusters);
#endif
    if (result)
    {
        attr.blockSize = ((size_t)dwSectPerClust * dwbytesPerSect); /* 簇大小 = 每簇扇区数 * 每扇区字节数 */
        attr.totalBlock = dwTotalClusters;
        attr.freeBlock = dwFreeClusters;
    }
#else
    struct statfs st;
    if (statfs(name.c_str(), &st) >= 0)
    {
        attr.blockSize = st.f_bsize;
        attr.totalBlock = st.f_blocks;
        attr.freeBlock = st.f_bfree;
    }
#endif
    return true;
}
} // namespace utility
