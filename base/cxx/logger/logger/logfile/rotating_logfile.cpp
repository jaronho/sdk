#include "rotating_logfile.h"

#include <regex>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

RotatingLogfile::RotatingLogfile(const std::string& path, const std::string& baseName, const std::string& extName, size_t maxSize,
                                 size_t maxFiles, bool indexFixed)
{
    std::string selfPath, selfName;
    if (path.empty() || baseName.empty())
    {
        Logfile::getProcessPathAndName(selfPath, selfName);
    }
    std::string logPath = path.empty() ? selfPath : path;
    m_baseName = baseName.empty() ? selfName : baseName;
    m_extName = extName.empty() ? ".log" : extName;
    if (!extName.empty() && '.' != extName[0])
    {
        m_extName.insert(0, ".");
    }
    m_maxFiles = maxFiles > 0 ? maxFiles : 0;
    m_indexFixed = indexFixed;
    std::vector<int> indexList;
    m_index = findLastIndex(logPath, indexList);
    m_logfile = std::make_shared<Logfile>(logPath, calcFilenameByIndex(m_index), maxSize);
}

int RotatingLogfile::getFileIndex() const
{
    return m_index;
}

bool RotatingLogfile::open()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_logfile->open();
}

void RotatingLogfile::close()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_logfile->close();
}

std::string RotatingLogfile::getPath()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_logfile->getPath();
}

std::string RotatingLogfile::getFilename()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_logfile->getFilename();
}

std::string RotatingLogfile::getFullName()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_logfile->getFullName();
}

size_t RotatingLogfile::getMaxSize()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_logfile->getMaxSize();
}

void RotatingLogfile::setMaxSize(size_t maxSize)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_logfile->setMaxSize(maxSize);
}

bool RotatingLogfile::isEnable()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_logfile->isEnable();
}

void RotatingLogfile::setEnable(bool enable)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_logfile->setEnable(enable);
}

size_t RotatingLogfile::getSize()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_logfile->getSize();
}

void RotatingLogfile::clear()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_logfile->clear();
}

size_t RotatingLogfile::getMaxFiles() const
{
    return m_maxFiles;
}

void RotatingLogfile::setMaxFiles(size_t maxFiles)
{
    m_maxFiles = maxFiles;
}

Logfile::Result RotatingLogfile::record(const char* content, size_t contentSize, bool newline, bool immediateFlush)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    Logfile::Result ret = m_logfile->record(content, contentSize, newline, immediateFlush);
    if (Logfile::Result::will_full == ret)
    {
        if (rotateFileList())
        {
            return m_logfile->record(content, contentSize, newline, immediateFlush);
        }
    }
    return ret;
}

Logfile::Result RotatingLogfile::record(const std::string& content, bool newline, bool immediateFlush)
{
    return record(content.data(), content.size(), newline, immediateFlush);
}

bool RotatingLogfile::forceFlush()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_logfile->forceFlush();
}

void RotatingLogfile::traverseFile(std::string path, std::function<void(const std::string& fullName)> callback)
{
    if (path.empty())
    {
        return;
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
            if (0 == strcmp(".", fileData.name) || 0 == strcmp("..", fileData.name))
            {
                continue;
            }
            std::string subName = path + fileData.name;
            struct _stat64 st;
            if (0 != _stat64(subName.c_str(), &st))
            {
                continue;
            }
            if (S_IFREG & st.st_mode) /* 文件 */
            {
                if (callback)
                {
                    callback(subName);
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
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name))
        {
            continue;
        }
        std::string subName = path + dirp->d_name;
        struct stat64 st;
        if (0 != stat64(subName.c_str(), &st))
        {
            continue;
        }
        if (S_IFREG & st.st_mode) /* 文件 */
        {
            if (callback)
            {
                callback(subName);
            }
        }
    }
    closedir(dir);
#endif
}

std::vector<int> RotatingLogfile::findIndexList(const std::string& path, const std::string& pattern)
{
    std::vector<int> indexList;
    std::regex re(pattern);
    std::smatch results;
    traverseFile(path, [&](const std::string& fullName) {
        std::string filename = fullName;
        size_t pos = filename.find_last_of("/\\");
        if (pos < filename.size())
        {
            filename = filename.substr(pos + 1, filename.size() - 1);
        }
        if (std::regex_match(filename, results, re))
        {
            indexList.emplace_back(atoi(results[1].str().c_str()));
        }
    });
    std::sort(indexList.begin(), indexList.end(), [](int a, int b) { return a < b; });
    return indexList;
}

int RotatingLogfile::findLastIndex(const std::string& path, std::vector<int>& indexList)
{
#ifdef _WIN32
    const std::string SLASH = "\\";
#else
    const std::string SLASH = "/";
#endif
    std::string pattern = m_baseName + "-([0-9]+)" + m_extName;
    indexList = findIndexList(path, pattern);
    if (indexList.empty())
    {
        return 1;
    }
    return indexList[indexList.size() - 1];
}

std::string RotatingLogfile::calcFilenameByIndex(int index)
{
    std::string filename;
    filename.append(m_baseName).append("-").append(std::to_string(index).append(m_extName));
    return filename;
}

bool RotatingLogfile::rotateFileList()
{
    std::string path = m_logfile->getPath();
#ifdef _WIN32
    path.append("\\");
#else
    path.append("/");
#endif
    size_t maxSize = m_logfile->getMaxSize();
    std::vector<int> indexList;
    int lastIndex = findLastIndex(path, indexList);
    if (m_maxFiles > 0 && indexList.size() >= m_maxFiles) /* 已达到文件最大数 */
    {
        /* 删除最早的文件 */
        size_t discardCount = indexList.size() - m_maxFiles;
        if (discardCount > 0)
        {
            for (size_t i = 0; i < discardCount; ++i)
            {
                std::string descardFullName = path + calcFilenameByIndex(indexList[i]);
                remove(descardFullName.c_str());
            }
            indexList.erase(indexList.begin(), indexList.begin() + discardCount);
        }
        if (1 == m_maxFiles) /* 只允许一个文件, 则清空文件内容 */
        {
            m_logfile->clear();
        }
        else /* 允许多个文件 */
        {
            m_logfile->close();
            if (m_indexFixed) /* 文件索引值固定, 把后面的文件向前移 */
            {
                for (size_t i = 1; i < indexList.size(); ++i)
                {
                    std::string prevFullName = path + calcFilenameByIndex(indexList[i - 1]);
                    std::string currFullName = path + calcFilenameByIndex(indexList[i]);
                    remove(prevFullName.c_str());
                    if (0 != rename(currFullName.c_str(), prevFullName.c_str()))
                    {
                        printf("move file [%s] to [%s] fail, errno[%d], desc: %s\n", currFullName.c_str(), prevFullName.c_str(), errno,
                               strerror(errno));
                        return false;
                    }
                }
            }
            else /* 文件索引值递增, 删除最早的文件 */
            {
                std::string descardFullName = path + calcFilenameByIndex(indexList[0]);
                remove(descardFullName.c_str());
                m_index = lastIndex + 1;
            }
            m_logfile = std::make_shared<Logfile>(path, calcFilenameByIndex(m_index), maxSize);
            m_logfile->open();
        }
    }
    else /* 不限文件个数, 或未达到最大文件数 */
    {
        m_index = lastIndex + 1;
        m_logfile->close();
        m_logfile = std::make_shared<Logfile>(path, calcFilenameByIndex(m_index), maxSize);
        m_logfile->open();
    }
    if (!m_logfile->isOpened())
    {
        return false;
    }
    return true;
}
