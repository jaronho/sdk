#include "logfile.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

bool Logfile::createPath(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }
    std::string parentPath;
    for (size_t i = 0, pathLen = path.size(); i < pathLen; ++i)
    {
        const char& ch = path[i];
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

Logfile::Logfile(const std::string& path, const std::string& filename, size_t maxSize)
{
    if (path.empty())
    {
        throw std::exception(std::logic_error("arg 'path' is empty"));
    }
    if (filename.empty())
    {
        throw std::exception(std::logic_error("arg 'filename' is empty"));
    }
    m_path = path;
    const char& lastPathChar = path[path.length() - 1];
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
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_f.is_open();
}

bool Logfile::open()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_f.is_open())
    {
        return true;
    }
    if (!createPath(m_path))
    {
        printf("create path [%s] fail, errno[%d], desc: %s\n", m_path.c_str(), errno, strerror(errno));
        return false;
    }
    m_f.open(m_fullName, std::ios::out | std::ios::binary | std::ios::app);
    if (!m_f.is_open())
    {
        printf("open file [%s] fail, errno[%d], desc: %s\n", m_fullName.c_str(), errno, strerror(errno));
        return false;
    }
    m_f.seekg(0, std::ios::end);
    m_size.store(m_f.tellg());
    return true;
}

void Logfile::close()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_f.is_open())
    {
        m_f.flush();
        m_f.close();
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

size_t Logfile::getSize() const
{
    return m_size.load();
}

void Logfile::clear()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_f.is_open())
    {
        m_f.close();
    }
    remove(m_fullName.c_str());
    m_f.open(m_fullName, std::ios::out | std::ios::binary | std::ios::app);
    if (m_f.is_open())
    {
        m_f.seekg(0, std::ios::end);
        m_size.store(m_f.tellg());
    }
    else
    {
        m_size.store(0);
    }
}

Logfile::Result Logfile::record(const std::string& content, bool newline)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (!m_f.is_open())
    {
        return Result::invalid;
    }
    if (!m_enable.load())
    {
        return Result::disabled;
    }
    size_t contentSize = content.size();
    if (m_maxSize > 0) /* 有限制文件大小 */
    {
        if (contentSize > m_maxSize)
        {
            return Result::too_large;
        }
        if (m_size.load() + contentSize > m_maxSize)
        {
            return Result::will_full;
        }
    }
    bool needFlush = false;
    if (m_size.load() > 0 && newline)
    {
        m_f.write("\n", 1);
        if (!m_f.good())
        {
            m_f.close();
            return Result::newline_failed;
        }
        m_size.store(m_size.load() + 1);
        needFlush = true;
    }
    if (contentSize > 0)
    {
        m_f.write(content.c_str(), contentSize);
        if (!m_f.good())
        {
            m_f.close();
            return Result::content_failed;
        }
        m_size.store(m_size.load() + contentSize);
        needFlush = true;
    }
    if (needFlush)
    {
        m_f.flush();
        if (!m_f.good())
        {
            m_f.close();
            return Result::flush_failed;
        }
    }
    return Result::ok;
}
