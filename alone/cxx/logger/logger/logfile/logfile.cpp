#include "logfile.h"

#include <assert.h>
#include <iostream>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace logger
{
bool Logfile::createPath(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }
    std::string pathNode;
    for (size_t i = 0, pathLen = path.length(); i < pathLen; ++i)
    {
        char pathChar = path.at(i);
        bool newPath = false;
        if ('/' == pathChar || '\\' == pathChar)
        {
#ifdef _WIN32
            pathChar = '\\';
#else
            pathChar = '/';
#endif
            newPath = true;
        }
        else if (pathLen - 1 == i)
        {
            newPath = true;
        }
        pathNode += pathChar;
        if (newPath)
        {
#ifdef _WIN32
            if (0 != _access(pathNode.c_str(), 0))
            {
                if (0 != _mkdir(pathNode.c_str()))
                {
                    return false;
                }
            }
#else
            if (0 != access(pathNode.c_str(), F_OK))
            {
                if (0 != mkdir(pathNode.c_str(), S_IRWXU | S_IRWXG | S_IRWXO))
                {
                    return false;
                }
            }
#endif
        }
    }
    return true;
}

void Logfile::traverse(
    std::string path, std::function<void(const std::string& name, long createTime, long writeTime, long accessTime)> folderCallback,
    std::function<void(const std::string& name, long createTime, long writeTime, long accessTime, unsigned long size)> fileCallback,
    bool recursive)
{
    char lastPathChar = path.at(path.length() - 1);
    if ('/' == lastPathChar || '\\' == lastPathChar)
    {
        path.pop_back();
    }
#ifdef _WIN32
#ifdef _WIN64
    _finddatai64_t fileData;
    __int64 handle = _findfirsti64((path + "\\*.*").c_str(), &fileData);
#else
    _finddata_t fileData;
    int handle = _findfirst((path + "\\*.*").c_str(), &fileData);
#endif
    if (-1 == handle || !(_A_SUBDIR & fileData.attrib))
    {
        return;
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
        std::string subName = path + "\\" + fileData.name;
        if (_A_SUBDIR & fileData.attrib)
        { /* is sub directory */
            if (folderCallback)
            {
                folderCallback(subName, (long)(fileData.time_create), (long)(fileData.time_write), (long)(fileData.time_access));
            }
        }
        else
        {
            if (fileCallback)
            {
                fileCallback(subName, (long)(fileData.time_create), (long)(fileData.time_write), (long)(fileData.time_access),
                             fileData.size);
            }
        }
        if (recursive)
        {
            traverse(subName, folderCallback, fileCallback, true);
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
        DIR* subDir = opendir(subName.c_str());
        if (subDir)
        { /* is sub directory */
            closedir(subDir);
            if (folderCallback)
            {
                folderCallback(subName, subStat.st_ctime, subStat.st_mtime, subStat.st_atime);
            }
        }
        else
        {
            if (fileCallback)
            {
                fileCallback(subName, subStat.st_ctime, subStat.st_mtime, subStat.st_atime, subStat.st_size);
            }
        }
        if (recursive)
        {
            traverse(subName, folderCallback, fileCallback, true);
        }
    }
    closedir(dir);
#endif
}

Logfile::Logfile(const std::string& path, const std::string& filename, size_t maxSize)
{
    assert(!path.empty());
    assert(!filename.empty());
    assert(maxSize > 0);
    m_path = path;
    char lastPathChar = path.at(path.length() - 1);
    if ('/' == lastPathChar || '\\' == lastPathChar)
    {
        m_path.pop_back();
    }
    m_filename = filename;
#ifdef _WIN32
    m_fullName = m_path + "\\" + m_filename;
#else
    m_fullName = m_path + "/" + m_filename;
#endif
    m_maxSize = maxSize;
}

Logfile::~Logfile()
{
    close();
}

bool Logfile::isOpened()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_fp)
    {
        return true;
    }
    return false;
}

bool Logfile::open()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_fp)
    {
        return true;
    }
    if (!createPath(m_path))
    {
        std::cout << "create path [" << m_path << "] fail, errno[" << errno << "]" << std::endl;
        return false;
    }
#ifdef _WIN32
    if (0 != fopen_s(&m_fp, m_fullName.c_str(), "a+"))
#else
    m_fp = fopen(m_fullName.c_str(), "a+");
    if (!m_fp)
#endif
    {
        std::cout << "open file [" << m_fullName << "] fail, errno[" << errno << "]" << std::endl;
        return false;
    }
    fseek(m_fp, 0, SEEK_END);
    m_size.store(ftell(m_fp));
    return true;
}

void Logfile::close()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_fp)
    {
        fclose(m_fp);
        m_fp = nullptr;
    }
    m_size.store(0);
}

std::string Logfile::getPath() const
{
    return m_path;
}

std::string Logfile::getFilename() const
{
    return m_filename;
}

std::string Logfile::getFullName() const
{
    return m_fullName;
}

size_t Logfile::getMaxSize() const
{
    return m_maxSize;
}

bool Logfile::isEnable() const
{
    return m_enable.load();
}

void Logfile::setEnable(bool enable)
{
    m_enable.store(enable);
}

size_t Logfile::getSize()
{
    return m_size.load();
}

void Logfile::clear()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_fp)
    {
        fclose(m_fp);
        m_fp = nullptr;
    }
    remove(m_fullName.c_str());
#ifdef _WIN32
    if (0 != fopen_s(&m_fp, m_fullName.c_str(), "a+"))
#else
    m_fp = fopen(m_fullName.c_str(), "a+");
    if (!m_fp)
#endif
    {
        fseek(m_fp, 0, SEEK_END);
        m_size.store(ftell(m_fp));
    }
    else
    {
        m_size.store(0);
    }
}

Logfile::Result Logfile::record(const std::string& content, bool newline)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (!m_fp)
    {
        return Result::INVALID;
    }
    if (!m_enable.load())
    {
        return Result::DISABLED;
    }
    size_t contentSize = content.size();
    if (contentSize > m_maxSize)
    {
        return Result::TOO_LARGE;
    }
    if (m_size.load() + contentSize > m_maxSize)
    {
        return Result::WILL_FULL;
    }
    bool needFlush = false;
    if (m_size.load() > 0 && newline)
    {
        if (1 != fwrite("\n", 1, 1, m_fp))
        {
            fclose(m_fp);
            m_fp = nullptr;
            return Result::NEWLINE_FAILED;
        }
        m_size.store(m_size.load() + 1);
        needFlush = true;
    }
    if (contentSize > 0)
    {
        if (contentSize != fwrite(content.c_str(), 1, contentSize, m_fp))
        {
            fclose(m_fp);
            m_fp = nullptr;
            return Result::CONTENT_FAILED;
        }
        m_size.store(m_size.load() + contentSize);
        needFlush = true;
    }
    if (needFlush)
    {
        if (0 != fflush(m_fp))
        {
            fclose(m_fp);
            m_fp = nullptr;
            return Result::FLUSH_FAILED;
        }
    }
    return Result::OK;
}
} // namespace logger
