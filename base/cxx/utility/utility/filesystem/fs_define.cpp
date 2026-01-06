#include "fs_define.h"

#include <algorithm>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>
#ifdef _WIN32
#include <Shlobj.h>
#include <Windows.h>
#include <shellapi.h>
#else
#include <sys/statfs.h>
#endif

namespace utility
{
#ifdef _WIN32
static bool isutf8(const std::string& str)
{
    size_t i = 0;
    if (str.size() >= 3 && (0xEF == (unsigned char)str[0] && 0xBB == (unsigned char)str[1] && 0xBF == (unsigned char)str[2])) /* BOM */
    {
        i = 3;
    }
    unsigned int byteCount = 0; /* UTF8可用1-6个字节编码, ASCII用1个字节 */
    for (; i < str.size(); ++i)
    {
        auto ch = (unsigned char)str[i];
        if ('\0' == ch)
        {
            break;
        }
        if (0 == byteCount)
        {
            if (ch >= 0x80) /* 如果不是ASCII码, 应该是多字节符, 计算字节数 */
            {
                if (ch >= 0xFC && ch <= 0xFD)
                {
                    byteCount = 6;
                }
                else if (ch >= 0xF8)
                {
                    byteCount = 5;
                }
                else if (ch >= 0xF0)
                {
                    byteCount = 4;
                }
                else if (ch >= 0xE0)
                {
                    byteCount = 3;
                }
                else if (ch >= 0xC0)
                {
                    byteCount = 2;
                }
                else
                {
                    return false;
                }
                byteCount--;
            }
        }
        else
        {
            if (0x80 != (ch & 0xC0)) /* 多字节符的非首字节, 应为10xxxxxx */
            {
                return false;
            }
            byteCount--; /* 减到为零为止 */
        }
    }
    return (0 == byteCount);
}

static std::wstring str2wstr(const std::string& str)
{
    if (!str.empty())
    {
        auto codePage = isutf8(str) ? CP_UTF8 : CP_ACP;
        int count = MultiByteToWideChar(codePage, 0, str.c_str(), str.size(), NULL, 0);
        if (count > 0)
        {
            std::wstring wstr(count, 0);
            MultiByteToWideChar(codePage, 0, str.c_str(), str.size(), &wstr[0], count);
            return wstr;
        }
    }
    return std::wstring();
}

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
                    hr = ppf->Load(str2wstr(filename).c_str(), STGM_READ); /* 尝试加载快捷方式 */
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
    BOOL result = GetDiskFreeSpaceW(str2wstr(name).c_str(), &dwSectPerClust, &dwbytesPerSect, &dwFreeClusters, &dwTotalClusters);
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
    std::wstring wname = str2wstr(name);
    struct _stat64 st;
    int ret = _wstat64(wname.c_str(), &st);
#else
    struct statx st;
    int ret = statx(AT_FDCWD, name.c_str(), AT_SYMLINK_NOFOLLOW, STATX_ALL, &st);
#endif
    if (0 != ret)
    {
        return false;
    }
#ifdef _WIN32
    attr.createTime = st.st_ctime;
    attr.modifyTime = st.st_mtime;
    attr.accessTime = st.st_atime;
    attr.size = st.st_size;
    attr.isDir = S_IFDIR & st.st_mode;
    attr.isFile = S_IFREG & st.st_mode;
#else
    /* Linux平台: 优先使用birth time作为创建时间 */
    if (st.stx_mask & STATX_BTIME)
    {
        attr.createTime = st.stx_btime.tv_sec; /* 真实的文件创建时间 */
    }
    else
    {
        attr.createTime = st.stx_ctime.tv_sec; /* 旧内核不支持则回退到状态改变时间 */
    }
    attr.modifyTime = st.stx_mtime.tv_sec;
    attr.accessTime = st.stx_atime.tv_sec;
    attr.size = st.stx_size;
    attr.isDir = S_ISDIR(st.stx_mode);
    attr.isFile = S_ISREG(st.stx_mode);
#endif
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesW(wname.c_str());
    attr.isSymLink = isShortcut(name);
    if (INVALID_FILE_ATTRIBUTES != dwAttrib)
    {
        attr.isSystem = dwAttrib & FILE_ATTRIBUTE_SYSTEM;
        attr.isHidden = dwAttrib & FILE_ATTRIBUTE_HIDDEN;
        attr.isWritable = !(dwAttrib & FILE_ATTRIBUTE_READONLY);
    }
    attr.isExecutable = S_IEXEC & st.st_mode;
#else
    attr.isSymLink = isSymbolicLink(name);
    attr.isHidden = subName.empty() ? false : '.' == subName[0]; /* linux中文件名第1个字符为.表示隐藏 */
    attr.isWritable = S_IWUSR & st.stx_mode;
    attr.isExecutable = S_IEXEC & st.stx_mode;
#endif
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
