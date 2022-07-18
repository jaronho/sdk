#include "file_info.h"

#include <algorithm>
#include <stdexcept>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif

namespace utility
{
struct FileBlockSize
{
    size_t fileSize; /* 文件大小(单位:字节) */
    size_t blockSize; /* 块大小(单位:字节) */
};

/**
 * @brief 计算块大小 
 * @param fileSize 文件大小
 * @param maxBlockSize 最大的块大小
 * @return 块大小, 为0表示文件为空
 */
static size_t calcBlockSize(size_t fileSize, size_t maxBlockSize)
{
    /* 文件/块大小映射 */
    static const int MB = 1024 * 1024;
    static const FileBlockSize FILE_BLOCK_SIZE_MAP[] = {{1024 * MB, 4 * MB}, {512 * MB, 4 * MB}, {256 * MB, 4 * MB},
                                                        {128 * MB, 2 * MB},  {64 * MB, 2 * MB},  {32 * MB, 2 * MB},
                                                        {16 * MB, 1 * MB},   {4 * MB, 1 * MB},   {1 * MB, 1 * MB}};
    /* 根据文件大小等级计算块大小 */
    size_t blockSize = fileSize;
    const int MAP_COUNT = sizeof(FILE_BLOCK_SIZE_MAP) / sizeof(FileBlockSize);
    for (int i = 0; i < MAP_COUNT; ++i)
    {
        if (fileSize >= FILE_BLOCK_SIZE_MAP[i].fileSize)
        {
            blockSize = FILE_BLOCK_SIZE_MAP[i].blockSize;
            break;
        }
    }
    if (maxBlockSize > 0 && blockSize > maxBlockSize)
    {
        blockSize = maxBlockSize;
    }
    return blockSize;
}

FileInfo::FileInfo(const std::string& fullName) : m_fullName(fullName)
{
    size_t pos = fullName.find_last_of("/\\");
    if (pos < fullName.size())
    {
        m_path = fullName.substr(0, pos + 1);
        m_filename = fullName.substr(pos + 1, fullName.size() - 1);
    }
    else
    {
        m_filename = fullName;
    }
    pos = m_filename.find_last_of(".");
    if (pos < m_filename.size())
    {
        m_basename = m_filename.substr(0, pos);
        m_extname = m_filename.substr(pos + 1, m_filename.size() - 1);
    }
    else
    {
        m_basename = m_filename;
    }
}

std::string FileInfo::name() const
{
    return m_fullName;
}

std::string FileInfo::path() const
{
    return m_path;
}

std::string FileInfo::filename() const
{
    return m_filename;
}

std::string FileInfo::basename() const
{
    return m_basename;
}

std::string FileInfo::extname() const
{
    return m_extname;
}

bool FileInfo::isExtName(std::string extName) const
{
    if (m_extname.empty() || extName.empty())
    {
        return false;
    }
    if ('.' == extName[0])
    {
        extName.erase(0, 1);
    }
    std::transform(extName.begin(), extName.end(), extName.begin(), tolower);
    auto tmpExtName = m_extname; /* 缓存避免修改到原始值 */
    std::transform(tmpExtName.begin(), tmpExtName.end(), tmpExtName.begin(), tolower);
    return (0 == tmpExtName.compare(extName));
}

FileAttribute FileInfo::attribute() const
{
    FileAttribute attr;
    getFileAttribute(m_fullName, attr);
    return attr;
}

bool FileInfo::exist() const
{
    FileAttribute attr;
    if (getFileAttribute(m_fullName, attr))
    {
        if (attr.isFile)
        {
            return true;
        }
    }
    return false;
}

bool FileInfo::create() const
{
    if (m_fullName.empty())
    {
        return false;
    }
    std::fstream f(m_fullName, std::ios::app);
    if (f.is_open())
    {
        f.close();
        return true;
    }
    return false;
}

bool FileInfo::remove() const
{
    if (m_fullName.empty())
    {
        return false;
    }
    if (0 == ::remove(m_fullName.c_str()))
    {
        return true;
    }
    return false;
}

FileInfo::CopyResult FileInfo::copy(const std::string& destFilename, int* errCode,
                                    const std::function<bool(size_t now, size_t total)>& progressCb, size_t maxBlockSize) const
{
    if (errCode)
    {
        *errCode = 0;
    }
    if (m_fullName.empty())
    {
        return CopyResult::src_open_failed;
    }
    /* 打开源文件 */
    std::fstream srcFile(m_fullName, std::ios::in | std::ios::binary | std::ios::ate);
    if (!srcFile.is_open())
    {
        if (errCode)
        {
            *errCode = errno;
        }
        return CopyResult::src_open_failed;
    }
    size_t srcFileSize = srcFile.tellg();
    srcFile.seekg(0, std::ios::beg);
    /* 打开目标文件 */
    std::fstream destFile(destFilename, std::ios::out | std::ios::binary);
    if (!destFile.is_open())
    {
        if (errCode)
        {
            *errCode = errno;
        }
        srcFile.close();
        return CopyResult::dest_open_failed;
    }
    /* 计算和分配块 */
    size_t blockSize = calcBlockSize(srcFileSize, maxBlockSize);
    if (0 == blockSize) /* 源文件为空 */
    {
        srcFile.close();
        destFile.close();
        return CopyResult::ok;
    }
    char* block = (char*)malloc(sizeof(char) * blockSize);
    if (!block)
    {
        if (errCode)
        {
            *errCode = errno;
        }
        srcFile.close();
        destFile.close();
        return CopyResult::memory_alloc_failed;
    }
    /* 拷贝文件内容 */
    size_t nowSize = 0, readSize = 0;
    bool stopFlag = false;
    while (!srcFile.eof())
    {
        memset(block, 0, sizeof(char) * blockSize);
        srcFile.read(block, blockSize);
        readSize = srcFile.gcount();
        if (0 == readSize)
        {
            if (errCode)
            {
                *errCode = errno;
            }
            break;
        }
        destFile.write(block, readSize);
        nowSize += readSize;
        if (progressCb)
        {
            if (!progressCb(nowSize, srcFileSize))
            {
                stopFlag = true;
                break;
            }
        }
    }
    free(block);
    destFile.flush();
    /* 关闭文件句柄 */
    srcFile.close();
    destFile.close();
    if (stopFlag)
    {
        return CopyResult::stop;
    }
    if (nowSize != srcFileSize)
    {
        return CopyResult::size_unequal;
    }
    return CopyResult::ok;
}

long long FileInfo::size() const
{
    long long fileSize = -1;
    if (m_fullName.empty())
    {
        return fileSize;
    }
    std::fstream f(m_fullName, std::ios::in | std::ios::ate);
    if (f.is_open())
    {
        fileSize = f.tellg();
        f.close();
    }
    return fileSize;
}

char* FileInfo::readAll(long long& fileSize, bool textFlag) const
{
    if (m_fullName.empty())
    {
        fileSize = -1;
        return NULL;
    }
    std::fstream f(m_fullName, std::ios::in | std::ios::binary | std::ios::ate);
    if (!f.is_open())
    {
        fileSize = -1;
        return NULL;
    }
    fileSize = f.tellg();
    f.seekg(0, std::ios::beg);
    size_t dataSize = sizeof(char) * fileSize + (textFlag ? 1 : 0);
    char* fileData = (char*)malloc(dataSize);
    if (fileData)
    {
        memset(fileData, 0, dataSize);
        f.read(fileData, fileSize);
        if (textFlag)
        {
            fileData[fileSize] = '\0';
        }
    }
    f.close();
    return fileData;
}

char* FileInfo::read(size_t offset, size_t& count, bool textFlag) const
{
    if (m_fullName.empty())
    {
        return NULL;
    }
    std::fstream f(m_fullName, std::ios::in | std::ios::binary);
    if (!f.is_open())
    {
        return NULL;
    }
    char* buffer = read(f, offset, count);
    if (buffer && textFlag)
    {
        buffer[count] = '\0';
    }
    f.close();
    return buffer;
}

bool FileInfo::replace(size_t offset, size_t count, const std::function<void(char* buffer, size_t count)>& callback) const
{
    if (m_fullName.empty())
    {
        return false;
    }
    std::fstream f(m_fullName, std::ios::in | std::ios::out);
    if (!f.is_open())
    {
        return false;
    }
    bool ret = replace(f, offset, count, callback);
    if (ret)
    {
        f.flush();
    }
    f.close();
    return ret;
}

bool FileInfo::write(const char* data, size_t length, bool isAppend, int* errCode) const
{
    if (errCode)
    {
        *errCode = 0;
    }
    if (!data || length <= 0)
    {
        return false;
    }
    if (m_fullName.empty())
    {
        return false;
    }
    std::fstream f(m_fullName, std::ios::out | (isAppend ? (std::ios::binary | std::ios::app) : std::ios::binary));
    if (!f.is_open())
    {
        if (errCode)
        {
            *errCode = errno;
        }
        return false;
    }
    f.write(data, length);
    f.flush();
    f.close();
    return true;
}

bool FileInfo::write(size_t pos, const char* data, size_t length, int* errCode)
{
    if (errCode)
    {
        *errCode = 0;
    }
    if (!data || length <= 0)
    {
        return false;
    }
    if (m_fullName.empty())
    {
        return false;
    }
    {
        /* 文件不存在则需要先创建 */
        FileAttribute attr;
        if (!getFileAttribute(m_fullName, attr) || !attr.isFile)
        {
            std::fstream f(m_fullName, std::ios::app);
            if (!f.is_open())
            {
                if (errCode)
                {
                    *errCode = errno;
                }
                return false;
            }
            f.close();
        }
    }
    std::fstream f(m_fullName, std::ios::in | std::ios::out); /* 该模式下需要文件已经存在 */
    if (!f.is_open())
    {
        if (errCode)
        {
            *errCode = errno;
        }
        return false;
    }
    f.seekp(pos, std::ios::beg);
    f.write(data, length);
    f.flush();
    f.close();
    return true;
}

bool FileInfo::isTextFile() const
{
    if (m_fullName.empty())
    {
        return false;
    }
    std::fstream f(m_fullName, std::ios::in | std::ios::binary);
    if (!f.is_open())
    {
        return false;
    }
    bool ret = isTextData(f);
    f.close();
    return ret;
}

char* FileInfo::read(std::fstream& f, size_t offset, size_t& count)
{
    if (!f.is_open())
    {
        return NULL;
    }
    char* buffer = NULL;
    f.seekg(0, std::ios::end);
    size_t fileSize = f.tellg();
    if (offset < fileSize)
    {
        if (offset + count > fileSize)
        {
            count = fileSize - offset;
        }
        size_t buffSize = sizeof(char) * count;
        buffer = (char*)malloc(buffSize);
        if (buffer)
        {
            memset(buffer, 0, buffSize);
            f.seekg(offset, std::ios::beg);
            f.read(buffer, count);
        }
    }
    else
    {
        count = 0;
    }
    return buffer;
}

bool FileInfo::replace(std::fstream& f, size_t offset, size_t count, const std::function<void(char* srcData, size_t count)>& callback)
{
    if (!f.is_open() || !callback)
    {
        return false;
    }
    char* buffer = NULL;
    f.seekg(0, std::ios::end);
    size_t fileSize = f.tellg();
    if (offset < fileSize)
    {
        if (offset + count > fileSize)
        {
            count = fileSize - offset;
        }
        size_t buffSize = sizeof(char) * count;
        buffer = (char*)malloc(buffSize);
        if (buffer)
        {
            memset(buffer, 0, buffSize);
            f.seekg(offset, std::ios::beg);
            f.read(buffer, count);
            callback(buffer, count);
            if (buffer)
            {
                f.seekg(offset, std::ios::beg);
                f.write(buffer, count);
                free(buffer);
                return true;
            }
        }
    }
    return false;
}

bool FileInfo::isTextData(std::fstream& f)
{
    if (!f.is_open())
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] arg 'f' is not opened");
    }
    f.seekg(0, std::ios::beg);
    char ch[1] = {0};
    while (!f.eof())
    {
        memset(ch, 0, sizeof(char));
        f.read(ch, 1);
        if (0 == f.gcount())
        {
            break;
        }
        if (0x00 == ch[0] || 0xFF == ch[0])
        {
            return false;
        }
    }
    return true;
}
} // namespace utility
