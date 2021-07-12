#include "path_info.h"

#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#pragma warning(disable : 6031)
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
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
#ifdef _WIN32
    return (0 == _access(m_path.c_str(), 0));
#else
    return (0 == access(m_path.c_str(), F_OK));
#endif
}

bool PathInfo::create()
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

bool PathInfo::remove()
{
    return clearImpl(m_path, true);
}

bool PathInfo::clear()
{
    return clearImpl(m_path, false);
}

bool PathInfo::clearImpl(const std::string& path, bool rmSelf)
{
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

std::string PathInfo::revise(const std::string& path)
{
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
