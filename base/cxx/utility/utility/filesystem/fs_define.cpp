#include "fs_define.h"

#include <algorithm>
#include <codecvt>
#include <string.h>
#include <sys/stat.h>
#include <vector>
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
static BOOL isShortcut(const std::string& filename)
{
    HRESULT hr = (HRESULT)-1;
    static const std::string INK_SUFFIX = ".lnk";
    if (filename.size() > INK_SUFFIX.size() && INK_SUFFIX == filename.substr(filename.size() - INK_SUFFIX.size()))
    {
        if (CoInitialize(NULL) >= 0)
        {
            IShellLink* psl;
            if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl) >= 0)
            {
                IPersistFile* ppf;
                if (psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf) >= 0)
                {
                    hr = ppf->Load(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(filename).c_str(),
                                   STGM_READ); /* 尝试加载快捷方式 */
                    ppf->Release();
                }
                psl->Release();
            }
            CoUninitialize();
        }
    }
    return (hr >= 0);
}
#else
bool isSymbolicLink(const std::string& filename)
{
    struct stat st;
    if (!filename.empty() && 0 == lstat(filename.c_str(), &st))
    {
        return S_ISLNK(st.st_mode);
    }
    return false;
}
#endif

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
    /* 需要使用宽字节, 避免包含非ASCII乱码文件名失败问题 */
    BOOL result = GetDiskFreeSpaceW(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(name).c_str(), &dwSectPerClust,
                                    &dwbytesPerSect, &dwFreeClusters, &dwTotalClusters);
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
    /* 需要使用宽字节, 避免包含非ASCII乱码文件名失败问题 */
    std::wstring wname = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(name);
    struct _stat64 st;
    int ret = _wstat64(wname.c_str(), &st);
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
    DWORD dwAttrib = GetFileAttributesW(wname.c_str());
    attr.isSymLink = isShortcut(name);
    if (INVALID_FILE_ATTRIBUTES != dwAttrib)
    {
        attr.isSystem = dwAttrib & FILE_ATTRIBUTE_SYSTEM;
        attr.isHidden = dwAttrib & FILE_ATTRIBUTE_HIDDEN;
        attr.isWritable = !(dwAttrib & FILE_ATTRIBUTE_READONLY);
    }
#else
    attr.isSymLink = isSymbolicLink(name);
    attr.isHidden = subName.empty() ? false : '.' == subName[0]; /* linux中文件名第1个字符为.表示隐藏 */
    attr.isWritable = S_IWUSR & st.st_mode;
#endif
    attr.isExecutable = S_IEXEC & st.st_mode;
    return true;
}

bool isValidFilename(std::string name, int platformType)
{
    platformType = (platformType >= 0 && platformType <= 2) ? platformType : 0;
    if (name.size() > 255)
    {
        return false;
    }
    if (0 == platformType || 1 == platformType)
    {
        /* Windows有特定的不允许使用的字符集和保留名称 */
        static const std::string INVALID_CHARS = "<>:\"/\\|?*";
        static const std::vector<std::string> RESERVED_NAMES = {"AUX",  "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7",
                                                                "COM8", "COM9", "CON",  "LPT1", "LPT2", "LPT3", "LPT4", "LPT5",
                                                                "LPT6", "LPT7", "LPT8", "LPT9", "NUL",  "PRN"};
        if (!INVALID_CHARS.empty() && std::string::npos != name.find_first_of(INVALID_CHARS))
        {
            return false;
        }
        std::transform(name.begin(), name.end(), name.begin(), toupper); /* 不区分大小写 */
        if (RESERVED_NAMES.end()
            != std::find_if(RESERVED_NAMES.begin(), RESERVED_NAMES.end(), [&](const std::string& item) { return (item == name); }))
        {
            return false;
        }
    }
    if (0 == platformType || 2 == platformType)
    {
        static const std::string INVALID_CHARS = "/";
        if (!INVALID_CHARS.empty() && std::string::npos != name.find_first_of(INVALID_CHARS))
        {
            return false;
        }
    }
    return true;
}
} // namespace utility
