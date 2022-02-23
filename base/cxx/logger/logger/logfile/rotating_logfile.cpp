#include "rotating_logfile.h"

#include <iostream>
#include <stdexcept>
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

namespace logger
{
RotatingLogfile::RotatingLogfile(const std::string& path, const std::string& baseName, const std::string& extName, size_t maxSize,
                                 size_t maxFiles, bool indexFixed)
{
    if (path.empty())
    {
        throw std::exception(std::logic_error("arg 'path' is empty"));
    }
    if (baseName.empty())
    {
        throw std::exception(std::logic_error("arg 'baseName' is empty"));
    }
    if (maxSize <= 0)
    {
        throw std::exception(std::logic_error("arg 'maxSize' <= 0"));
    }
    m_baseName = baseName;
    m_extName = extName;
    if (!extName.empty() && '.' != extName[0])
    {
        m_extName.insert(0, ".");
    }
    m_maxFiles = maxFiles > 0 ? maxFiles : 0;
    m_indexFixed = indexFixed;
    std::vector<int> indexList;
    m_index.store(findLastIndex(path, indexList));
    m_logfile = std::make_shared<Logfile>(path, calcFilenameByIndex(m_index.load()), maxSize);
}

size_t RotatingLogfile::getFileIndex() const
{
    return m_index.load();
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

Logfile::Result RotatingLogfile::record(const std::string& content, bool newline)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    Logfile::Result ret = m_logfile->record(content, newline);
    if (Logfile::Result::will_full == ret)
    {
        if (rotateFileList())
        {
            return m_logfile->record(content, newline);
        }
    }
    return ret;
}

void RotatingLogfile::traverseFile(std::string path, std::function<void(const std::string& fullName)> callback, bool recursive)
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
        std::string subName = path + fileData.name;
        if (_A_ARCH & fileData.attrib) /* 文件 */
        {
            if (callback)
            {
                callback(subName);
            }
        }
        if (recursive)
        {
            traverseFile(subName, callback, true);
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
        struct stat64 subStat;
        if (0 == stat64(subName.c_str(), &subStat))
        {
            if (S_IFREG & subStat.st_mode) /* 文件 */
            {
                if (callback)
                {
                    callback(subName);
                }
            }
            if (recursive)
            {
                traverseFile(subName, callback, true);
            }
        }
    }
    closedir(dir);
#endif
}

bool RotatingLogfile::findIndexList(const std::string& path, const std::regex& pattern, std::vector<int>& indexList)
{
    bool matchFlag = false;
    std::smatch results;
    traverseFile(
        path,
        [&](const std::string& fullName) {
            std::string filename = fullName;
            size_t pos = filename.find_last_of("/\\");
            if (pos < filename.size())
            {
                filename = filename.substr(pos + 1, filename.size() - 1);
            }
            if (std::regex_match(filename, results, pattern))
            {
                indexList.emplace_back(atoi(results[1].str().c_str()));
                matchFlag = true;
            }
        },
        false);
    std::sort(indexList.begin(), indexList.end(), [](int a, int b) { return a < b; });
    return matchFlag;
}

int RotatingLogfile::findLastIndex(const std::string& path, std::vector<int>& indexList)
{
    std::regex pattern(m_baseName + "-([0-9]+)" + (m_extName.empty() ? "" : "\\") + m_extName);
    int lastIndex = 1;
    if (findIndexList(path, pattern, indexList))
    {
        lastIndex = indexList[indexList.size() - 1];
    }
    return lastIndex;
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
        for (size_t i = 0; i < discardCount; ++i)
        {
            std::string descardFullName = path + calcFilenameByIndex(indexList[i]);
            remove(descardFullName.c_str());
        }
        if (1 == m_maxFiles) /* 只允许一个文件, 则清空文件内容 */
        {
            m_logfile->clear();
        }
        else /* 允许多个文件 */
        {
            m_logfile->close();
            if (m_indexFixed) /* 把后面的文件向前移 */
            {
                for (size_t i = discardCount + 1, count = indexList.size(); i < count; ++i)
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
            else /* 文件索引值固定 */
            {
                std::string descardFullName = path + calcFilenameByIndex(indexList[discardCount]);
                remove(descardFullName.c_str());
                lastIndex += 1;
            }
            m_logfile = std::make_shared<Logfile>(path, calcFilenameByIndex(lastIndex), maxSize);
            if (m_logfile->open())
            {
                m_index.store(lastIndex);
            }
        }
    }
    else /* 不限文件个数, 或未达到最大文件数 */
    {
        lastIndex += 1;
        m_logfile->close();
        m_logfile = std::make_shared<Logfile>(path, calcFilenameByIndex(lastIndex), maxSize);
        if (m_logfile->open())
        {
            m_index.store(lastIndex);
        }
    }
    if (!m_logfile->isOpened())
    {
        return false;
    }
    return true;
}
} // namespace logger
