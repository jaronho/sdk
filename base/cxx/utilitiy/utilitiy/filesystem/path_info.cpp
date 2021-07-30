#include "path_info.h"

#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <Shlobj.h>
#include <atlstr.h>
#include <direct.h>
#include <io.h>
#pragma warning(disable : 6031)
#else
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace utilitiy
{
PathInfo::PathInfo(const std::string& path) : m_path(revise(path)) {}

std::string PathInfo::path()
{
    return m_path;
}

bool PathInfo::isAbsolute()
{
#ifdef _WIN32
    if (m_path.size() >= 2 && ((m_path[0] >= 'a' && m_path[0] <= 'z') || (m_path[0] >= 'A' && m_path[0] <= 'Z')) && (':' == m_path[1]))
    {
        return true;
    }
#else
    if (m_path.size() >= 1 && '/' == m_path[0])
    {
        return true;
    }
#endif
    return false;
}

bool PathInfo::isRoot()
{
    if (isAbsolute())
    {
#ifdef _WIN32
        if (2 == m_path.size() || (3 == m_path.size() && '\\' == m_path[2]))
        {
            return true;
        }
#else
        if (1 == m_path.size())
        {
            return true;
        }
#endif
    }
    return false;
}

bool PathInfo::exist()
{
    if (m_path.empty())
    {
        return false;
    }
#ifdef _WIN32
    struct _stat64 st;
    int ret = _stat64(m_path.c_str(), &st);
#else
    struct stat64 st;
    int ret = stat64(m_path.c_str(), &st);
#endif
    if (0 == ret && (S_IFDIR & st.st_mode))
    {
        return true;
    }
    return false;
}

bool PathInfo::create(bool ioSync)
{
    if (m_path.empty())
    {
        return false;
    }
    std::string parentPath;
    for (size_t i = 0, pathLen = m_path.size(); i < pathLen; ++i)
    {
        const char& ch = m_path[i];
        parentPath.push_back(ch);
        if ('/' == ch || '\\' == ch || pathLen - 1 == i)
        {
#ifdef _WIN32
            if (0 != _access(parentPath.c_str(), 0))
            {
                if (0 != _mkdir(parentPath.c_str()))
                {
                    return false;
                }
            }
#else
            if (0 != access(parentPath.c_str(), F_OK))
            {
                if (0 != mkdir(parentPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO))
                {
                    return false;
                }
            }
#endif
        }
    }
    if (ioSync)
    {
#ifndef _WIN32
        sync();
#endif
    }
    return true;
}

bool PathInfo::remove(bool ioSync)
{
    if (clearImpl(m_path, true))
    {
        if (ioSync)
        {
#ifndef _WIN32
            sync();
#endif
        }
        return true;
    }
    return false;
}

bool PathInfo::clear(bool continueIfRoot, bool ioSync)
{
    if (isRoot() && !continueIfRoot) /* 避免误清空根目录, 除非明确知道该操作 */
    {
        return false;
    }
    if (clearImpl(m_path, false))
    {
        if (ioSync)
        {
#ifndef _WIN32
            sync();
#endif
        }
        return true;
    }
    return false;
}

void PathInfo::traverse(std::function<void(const std::string& name, const FileAttribure& attr)> folderCallback,
                        std::function<void(const std::string& name, const FileAttribure& attr, long long size)> fileCallback,
                        bool recursive)
{
    traverseImpl(m_path, folderCallback, fileCallback, recursive);
}

bool PathInfo::clearImpl(const std::string& path, bool rmSelf)
{
    if (path.empty())
    {
        return false;
    }
#ifdef _WIN32
    struct _finddata_t fileData;
    intptr_t handle;
    if (-1 == (handle = _findfirst((path + "\\*").c_str(), &fileData)))
    {
        return false;
    }
    while (0 == _findnext(handle, &fileData))
    {
        if (0 == strcmp(".", fileData.name) || 0 == strcmp("..", fileData.name))
        {
            continue;
        }
        std::string subName = path + "\\" + fileData.name;
        if (_A_SUBDIR & fileData.attrib)
        {
            clearImpl(subName, true);
        }
        else
        {
            ::remove(subName.c_str());
        }
    }
    _findclose(handle);
#else
    DIR* dir = opendir(path.c_str());
    if (!dir)
    {
        return false;
    }
    struct dirent* dirp = NULL;
    while ((dirp = readdir(dir)))
    {
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name))
        {
            continue;
        }
        std::string subName = path + "/" + dirp->d_name;
        DIR* subDir = opendir(subName.c_str());
        if (!subDir)
        {
            ::remove(subName.c_str());
        }
        else
        {
            closedir(subDir);
            clearImpl(subName, true);
        }
    }
    closedir(dir);
#endif
    if (rmSelf)
    {
        rmdir(path.c_str());
    }
    return true;
}

#if _WIN32
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

void PathInfo::traverseImpl(std::string path, std::function<void(const std::string& name, const FileAttribure& attr)> folderCallback,
                            std::function<void(const std::string& name, const FileAttribure& attr, long long size)> fileCallback,
                            bool recursive)
{
    if (path.empty())
    {
        return;
    }
#ifdef _WIN32
    struct _finddata_t fileData;
    intptr_t handle = _findfirst((path + "\\*.*").c_str(), &fileData);
    if (-1 == handle || !(_A_SUBDIR & fileData.attrib))
    {
        return;
    }
    while (0 == _findnext(handle, &fileData))
    {
        if (0 == strcmp(".", fileData.name) || 0 == strcmp("..", fileData.name))
        {
            continue;
        }
        std::string subName = path + "\\" + fileData.name;
        FileAttribure attr;
        attr.createTime = fileData.time_create;
        attr.modifyTime = fileData.time_write;
        attr.accessTime = fileData.time_access;
#if 1 /* 方法一 */
        SHFILEINFO shFileInfo;
        memset(&shFileInfo, 0, sizeof(SHFILEINFO));
#ifdef UNICODE
        std::wstring subNameW = string2wstring(subName);
        if (subNameW.empty())
        {
            continue;
        }
        SHGetFileInfo(subNameW.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO),
                      SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_ATTRIBUTES);
#else
        SHGetFileInfo(subName.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO),
                      SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_ATTRIBUTES);
#endif
        attr.isReadOnly = SFGAO_READONLY & shFileInfo.dwAttributes;
        attr.isHidden = SFGAO_HIDDEN & shFileInfo.dwAttributes;
        attr.isSystem = SFGAO_SYSTEM & shFileInfo.dwAttributes;
        attr.isSymLink = SFGAO_LINK & shFileInfo.dwAttributes;
#else /* 方法二 */
        attr.isReadOnly = _A_RDONLY & fileData.attrib;
        attr.isHidden = _A_HIDDEN & fileData.attrib;
        attr.isSystem = _A_SYSTEM & fileData.attrib;
#endif
        if (_A_SUBDIR & fileData.attrib) /* 目录 */
        {
            if (folderCallback)
            {
                folderCallback(subName, attr);
            }
        }
        else /* 文件 */
        {
            if (fileCallback)
            {
                fileCallback(subName, attr, fileData.size);
            }
        }
        if (recursive)
        {
            traverseImpl(subName, folderCallback, fileCallback, true);
        }
    }
    _findclose(handle);
#else
    DIR* dir = opendir(path.c_str());
    if (!dir)
    {
        return;
    }
    struct dirent* dirp = NULL;
    while ((dirp = readdir(dir)))
    {
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name))
        {
            continue;
        }
        std::string subName = path + "/" + dirp->d_name;
        struct stat subStat;
        if (0 != stat(subName.c_str(), &subStat))
        {
            continue;
        }
        FileAttribure attr;
        attr.createTime = subStat.st_ctime;
        attr.modifyTime = subStat.st_mtime;
        attr.accessTime = subStat.st_atime;
        DIR* subDir = opendir(subName.c_str());
        if (subDir) /* 目录*/
        {
            closedir(subDir);
            if (folderCallback)
            {
                folderCallback(subName, attr);
            }
        }
        else /* 文件 */
        {
            if (fileCallback)
            {
                fileCallback(subName, attr, subStat.st_size);
            }
        }
        if (recursive)
        {
            traverseImpl(subName, folderCallback, fileCallback, true);
        }
    }
    closedir(dir);
#endif
}

std::string PathInfo::revise(const std::string& path)
{
    if (path.empty())
    {
        return path;
    }
    std::string newPath;
    bool isPreSlash = false;
    for (size_t i = 0; i < path.size(); ++i)
    {
        const char& ch = path[i];
        if ('/' == ch || '\\' == ch)
        {
            if (!isPreSlash)
            {
#ifdef _WIN32
                newPath.push_back('\\');
#else
                newPath.push_back('/');
#endif
            }
            isPreSlash = true;
        }
        else
        {
            newPath.push_back(ch);
            isPreSlash = false;
        }
    }
    newPath.erase(0, newPath.find_first_not_of(' '));
    newPath.erase(newPath.find_last_not_of(' ') + 1);
    return newPath;
}
} // namespace utilitiy
