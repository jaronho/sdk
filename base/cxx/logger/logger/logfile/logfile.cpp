#include "logfile.h"

#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <Windows.h>
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

void Logfile::getProcessPathAndName(std::string& path, std::string& name)
{
#ifdef _WIN32
    char exeFile[MAX_PATH + 1] = {0};
    GetModuleFileNameA(NULL, exeFile, MAX_PATH);
#else
    char exeFile[261] = {0};
    unsigned int exeFileLen = readlink("/proc/self/exe", exeFile, sizeof(exeFile) - 1);
    if (exeFileLen > 0 && exeFileLen < sizeof(exeFile) - 1)
    {
        exeFile[exeFileLen] = '\0';
    }
#endif
    std::string fullPath = exeFile;
    size_t lastSlashPos = fullPath.find_last_of("/\\");
    if (std::string::npos == lastSlashPos)
    {
        path.clear();
        name = fullPath;
    }
    else
    {
        path = fullPath.substr(0, lastSlashPos);
        name = fullPath.substr(lastSlashPos + 1);
    }
    size_t lastDotPos = name.find_last_of('.');
    if (std::string::npos != lastDotPos)
    {
        name = name.substr(0, lastDotPos);
    }
}

Logfile::Logfile(const std::string& path, const std::string& filename, size_t maxSize)
{
    std::string selfPath, selfName;
    if (path.empty() || filename.empty())
    {
        getProcessPathAndName(selfPath, selfName);
    }
    m_path = path.empty() ? selfPath : path;
    const char& lastPathChar = path[path.length() - 1];
    if ('/' == lastPathChar || '\\' == lastPathChar)
    {
        m_path.pop_back();
    }
    m_filename = filename.empty() ? (selfName + ".log") : filename;
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
    return (m_f ? true : false);
}

bool Logfile::open()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_f)
    {
        return true;
    }
    if (!createPath(m_path))
    {
        printf("create path [%s] fail, errno[%d], desc: %s\n", m_path.c_str(), errno, strerror(errno));
        return false;
    }
    m_f = fopen(m_fullName.c_str(), "ab+");
    if (!m_f)
    {
        printf("open file [%s] fail, errno[%d], desc: %s\n", m_fullName.c_str(), errno, strerror(errno));
        return false;
    }
#ifdef _WIN32
    _fseeki64(m_f, 0, SEEK_END);
    m_size = _ftelli64(m_f);
#else
    fseeko64(m_f, 0, SEEK_END);
    m_size = ftello64(m_f);
#endif
    return true;
}

void Logfile::close()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_f)
    {
        fflush(m_f);
        fclose(m_f);
        m_f = nullptr;
    }
    m_size = 0;
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

void Logfile::setMaxSize(size_t maxSize)
{
    m_maxSize = maxSize;
}

bool Logfile::isEnable() const
{
    return m_enable;
}

void Logfile::setEnable(bool enable)
{
    m_enable = enable;
}

size_t Logfile::getSize() const
{
    return m_size;
}

void Logfile::clear()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_f)
    {
        fclose(m_f);
        m_f = nullptr;
    }
    remove(m_fullName.c_str());
    m_f = fopen(m_fullName.c_str(), "ab+");
    if (m_f)
    {
#ifdef _WIN32
        _fseeki64(m_f, 0, SEEK_END);
        m_size = _ftelli64(m_f);
#else
        fseeko64(m_f, 0, SEEK_END);
        m_size = ftello64(m_f);
#endif
    }
    else
    {
        m_size = 0;
    }
}

Logfile::Result Logfile::record(const std::string& content, bool newline)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (!m_f)
    {
        return Result::invalid;
    }
    if (!m_enable)
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
        if (m_size + contentSize > m_maxSize)
        {
            return Result::will_full;
        }
    }
    bool needFlush = false;
    if (m_size > 0 && newline)
    {
        if (0 == fwrite("\n", 1, 1, m_f))
        {
            fclose(m_f);
            m_f = nullptr;
            return Result::newline_failed;
        }
        ++m_size;
        needFlush = true;
    }
    if (contentSize > 0)
    {
        if (0 == fwrite(content.c_str(), 1, contentSize, m_f))
        {
            fclose(m_f);
            m_f = nullptr;
            return Result::content_failed;
        }
        m_size = m_size + contentSize;
        needFlush = true;
    }
    if (needFlush)
    {
        if (0 != fflush(m_f))
        {
            fclose(m_f);
            m_f = nullptr;
            return Result::flush_failed;
        }
    }
    return Result::ok;
}
