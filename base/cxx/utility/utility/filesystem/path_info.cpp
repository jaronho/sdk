#include "path_info.h"

#include <stdexcept>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#pragma warning(disable : 6031)
#else
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace utility
{
PathInfo::PathInfo(const std::string& path, bool autoEndWithSlash) : m_path(revise(path))
{
    if (m_path.empty())
    {
        throw std::exception(std::logic_error("var 'm_path' is empty"));
    }
    if (autoEndWithSlash)
    {
        const char& lastPathChar = m_path[m_path.size() - 1];
        if ('/' != lastPathChar && '\\' != lastPathChar)
        {
#ifdef _WIN32
            m_path.push_back('\\');
#else
            m_path.push_back('/');
#endif
        }
    }
}

std::string PathInfo::path() const
{
    return m_path;
}

bool PathInfo::isEndWithSlash() const
{
    const char& lastPathChar = m_path[m_path.size() - 1];
    if ('/' == lastPathChar || '\\' == lastPathChar)
    {
        return true;
    }
    return false;
}

bool PathInfo::isAbsolute() const
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

bool PathInfo::isRoot() const
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

FileAttribute PathInfo::attribute() const
{
    FileAttribute attr;
    getFileAttribute(m_path, attr);
    return attr;
}

bool PathInfo::exist() const
{
    FileAttribute attr;
    if (getFileAttribute(m_path, attr))
    {
        if (attr.isDir)
        {
            return true;
        }
    }
    return false;
}

bool PathInfo::create() const
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
    return true;
}

bool PathInfo::remove() const
{
    if (clearImpl(m_path, true))
    {
        return true;
    }
    return false;
}

bool PathInfo::clear(bool continueIfRoot) const
{
    if (isRoot() && !continueIfRoot) /* 避免误清空根目录, 除非明确知道该操作 */
    {
        return false;
    }
    if (clearImpl(m_path, false))
    {
        return true;
    }
    return false;
}

bool PathInfo::empty() const
{
    size_t childCount = 0;
    traverseImpl(
        m_path, 0,
        [&](const std::string& name, const FileAttribute& attr, int depth) {
            ++childCount;
            return true;
        },
        [&](const std::string& name, const FileAttribute& attr, int depth) { ++childCount; },
        [&]() {
            if (childCount > 0)
            {
                return true;
            }
            return false;
        },
        false);
    return (0 == childCount);
}

void PathInfo::traverse(const std::function<bool(const std::string& name, const FileAttribute& attr, int depth)>& folderCb,
                        const std::function<void(const std::string& name, const FileAttribute& attr, int depth)>& fileCb,
                        const std::function<bool()>& stopCb, bool recursive) const
{
    traverseImpl(m_path, 0, folderCb, fileCb, stopCb, recursive);
}

bool PathInfo::clearImpl(std::string path, bool rmSelf)
{
    if (path.empty())
    {
        return false;
    }
    const char& lastPathChar = path[path.size() - 1];
    if ('/' != lastPathChar && '\\' != lastPathChar)
    {
#ifdef _WIN32
        path.push_back('\\');
#else
        path.push_back('/');
#endif
    }
#ifdef _WIN32
#ifdef _WIN64
    _finddatai64_t fileData;
    __int64 handle = _findfirsti64((path + "*.*").c_str(), &fileData);
#else
    _finddata_t fileData;
    int handle = _findfirst((path + "*.*").c_str(), &fileData);
#endif
    if (-1 == handle)
    {
        return false;
    }
    if (!(_A_SUBDIR & fileData.attrib))
    {
        _findclose(handle);
        return false;
    }
#ifdef _WIN64
    while (0 == _findnexti64(handle, &fileData))
#else
    while (0 == _findnext(handle, &fileData))
#endif
    {
        if (0 == strcmp(".", fileData.name) || 0 == strcmp("..", fileData.name))
        {
            continue;
        }
        std::string subName = path + fileData.name;
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
        std::string subName = path + dirp->d_name;
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

void PathInfo::traverseImpl(std::string path, int depth,
                            const std::function<bool(const std::string& name, const FileAttribute& attr, int depth)>& folderCb,
                            const std::function<void(const std::string& name, const FileAttribute& attr, int depth)>& fileCb,
                            const std::function<bool()>& stopCb, bool recursive)
{
    if (path.empty())
    {
        return;
    }
    if (stopCb && stopCb())
    {
        return;
    }
    depth += 1;
    const char& lastPathChar = path[path.size() - 1];
    if ('/' != lastPathChar && '\\' != lastPathChar)
    {
#ifdef _WIN32
        path.push_back('\\');
#else
        path.push_back('/');
#endif
    }
#ifdef _WIN32
#ifdef _WIN64
    _finddatai64_t fileData;
    __int64 handle = _findfirsti64((path + "*.*").c_str(), &fileData);
#else
    _finddata_t fileData;
    int handle = _findfirst((path + "*.*").c_str(), &fileData);
#endif
    if (-1 == handle)
    {
        return;
    }
    if (_A_SUBDIR & fileData.attrib)
    {
#ifdef _WIN64
        while (0 == _findnexti64(handle, &fileData))
#else
        while (0 == _findnext(handle, &fileData))
#endif
        {
            if (stopCb && stopCb())
            {
                break;
            }
            if (0 == strcmp(".", fileData.name) || 0 == strcmp("..", fileData.name))
            {
                continue;
            }
            std::string subName = path + fileData.name;
            FileAttribute attr;
            if (getFileAttribute(subName, attr))
            {
                if (attr.isDir) /* 目录 */
                {
                    bool allowEnterSub = true;
                    if (folderCb)
                    {
                        allowEnterSub = folderCb(subName, attr, depth);
                    }
                    if (recursive && allowEnterSub)
                    {
                        traverseImpl(subName, depth, folderCb, fileCb, stopCb, true);
                    }
                }
                else if (attr.isFile) /* 文件 */
                {
                    if (fileCb)
                    {
                        fileCb(subName, attr, depth);
                    }
                }
            }
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
        if (stopCb && stopCb())
        {
            break;
        }
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name))
        {
            continue;
        }
        std::string subName = path + dirp->d_name;
        FileAttribute attr;
        if (getFileAttribute(subName, attr))
        {
            if (attr.isDir) /* 目录*/
            {
                bool allowEnterSub = true;
                if (folderCb)
                {
                    allowEnterSub = folderCb(subName, attr, depth);
                }
                if (recursive && allowEnterSub)
                {
                    traverseImpl(subName, depth, folderCb, fileCb, stopCb, true);
                }
            }
            else if (attr.isFile) /* 文件 */
            {
                if (fileCb)
                {
                    fileCb(subName, attr, depth);
                }
            }
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

std::string PathInfo::getcwd(bool autoEndWithSlash)
{
#ifdef _WIN32
    char* buffer = ::_getcwd(NULL, 0);
#else
    char* buffer = ::getcwd(NULL, 0);
#endif
    std::string currDir;
    if (buffer)
    {
        currDir = buffer;
        free(buffer);
        if (!currDir.empty() && autoEndWithSlash)
        {
            const char& lastPathChar = currDir[currDir.size() - 1];
            if ('/' != lastPathChar && '\\' != lastPathChar)
            {
#ifdef _WIN32
                currDir.push_back('\\');
#else
                currDir.push_back('/');
#endif
            }
        }
    }
    return currDir;
}
} // namespace utility