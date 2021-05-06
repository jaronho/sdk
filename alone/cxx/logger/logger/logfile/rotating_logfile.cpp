#include "rotating_logfile.h"

#include <assert.h>
#include <iostream>
#include <stdio.h>

namespace logger
{
RotatingLogfile::RotatingLogfile(const std::string& path, const std::string& baseName, const std::string& extName, size_t maxSize, size_t maxFiles)
{
    assert(!path.empty());
    assert(!baseName.empty());
    assert(extName.size() > 1);
    assert(maxSize > 0);
    m_baseName = baseName;
    m_extName = extName;
    if ('.' != extName.at(0))
    {
        m_extName.insert(0, ".");
    }
    m_maxFiles = maxFiles > 0 ? maxFiles : 0;
    std::vector<int> indexList;
    m_index.store(findLastIndex(path, indexList));
    m_logfile = std::make_shared<Logfile>(path, calcFilenameByIndex(m_index.load()), maxSize);
}

size_t RotatingLogfile::getFileIndex() const
{
    return m_index;
}

bool RotatingLogfile::open()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_logfile->open();
}

void RotatingLogfile::close()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_logfile->close();
}

Logfile::Result RotatingLogfile::record(const std::string& content, bool newline)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    Logfile::Result ret = m_logfile->record(content, newline);
    if (Logfile::Result::WILL_FULL == ret)
    {
        if (rotateFileList())
        {
            return m_logfile->record(content, newline);
        }
    }
    return ret;
}

bool RotatingLogfile::findIndexList(const std::string& path, const std::regex& pattern, std::vector<int>& indexList)
{
    bool matchFlag = false;
    std::smatch results;
    Logfile::traverse(
        path, nullptr,
        [&](const std::string& name, long createTime, long writeTime, long accessTime, unsigned long size) {
            std::string filename = name;
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
    std::regex pattern(m_baseName + "-([0-9]+)\\" + m_extName);
    int lastIndex = 1;
    if (findIndexList(path, pattern, indexList))
    {
        lastIndex = indexList.at(indexList.size() - 1);
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
        size_t discardCount = indexList.size() - m_maxFiles;
        for (size_t i = 0; i < discardCount; ++i)
        {
            std::string descardFullName = path + calcFilenameByIndex(indexList.at(i));
            remove(descardFullName.c_str());
        }
        if (1 == m_maxFiles) /* 只允许一个文件, 则清空文件内容 */
        {
            m_logfile->clear();
        }
        else /* 允许多个文件, 删除最早的文件, 把后面的文件向前移动, 留出最新的文件 */
        {
            m_logfile->close();
            for (size_t i = discardCount + 1, count = indexList.size(); i < count; ++i)
            {
                std::string preFullName = path + calcFilenameByIndex(indexList.at(i - 1));
                std::string nowFullName = path + calcFilenameByIndex(indexList.at(i));
                remove(preFullName.c_str());
                if (0 != rename(nowFullName.c_str(), preFullName.c_str()))
                {
                    std::cout << "move file [" << nowFullName << "] to [" << preFullName << "] fail, errno[" << errno << "]" << std::endl;
                    return false;
                }
            }
            m_logfile = std::make_shared<Logfile>(path, calcFilenameByIndex(m_index.load()), maxSize);
            m_logfile->open();
        }
    }
    else /* 不限文件个数, 或未达到最大文件数 */
    {
        lastIndex += 1;
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
