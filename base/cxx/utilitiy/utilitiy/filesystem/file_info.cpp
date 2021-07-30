#include "file_info.h"

#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace utilitiy
{
struct FileBlockSize
{
    size_t fileSize; /* 文件大小(单位:Kb) */
    size_t blockSize; /* 块大小(单位:Kb) */
};

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

FileInfo& FileInfo::operator=(const FileInfo& src)
{
    m_fullName = src.m_fullName;
    m_path = src.m_path;
    m_filename = src.m_filename;
    m_basename = src.m_basename;
    m_extname = src.m_extname;
    return (*this);
}

std::string FileInfo::name()
{
    return m_fullName;
}

std::string FileInfo::path()
{
    return m_path;
}

std::string FileInfo::filename()
{
    return m_filename;
}

std::string FileInfo::basename()
{
    return m_basename;
}

std::string FileInfo::extname()
{
    return m_extname;
}

bool FileInfo::exist()
{
    if (m_fullName.empty())
    {
        return false;
    }
#ifdef _WIN32
    struct _stat64 st;
    int ret = _stat64(m_fullName.c_str(), &st);
#else
    struct stat64 st;
    int ret = stat64(m_fullName.c_str(), &st);
#endif
    if (0 == ret && (S_IFREG & st.st_mode))
    {
        return true;
    }
    return false;
}

bool FileInfo::create()
{
    if (m_fullName.empty())
    {
        return false;
    }
    std::fstream f(m_fullName, std::ios::out | std::ios::app);
    if (f.is_open())
    {
        f.flush();
        f.close();
        return true;
    }
    return false;
}

bool FileInfo::remove(bool ioSync)
{
    if (m_fullName.empty())
    {
        return false;
    }
    if (0 == ::remove(m_fullName.c_str()))
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

/**
 * @brief 计算块大小 
 * @param fileSize 文件大小
 * @param maxBlockSize 最大的块大小
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

bool FileInfo::copy(const std::string& destFilename, const std::function<void(size_t now, size_t total)>& progressCb, size_t maxBlockSize)
{
    if (m_fullName.empty())
    {
        return false;
    }
    /* 打开源文件 */
    std::fstream srcFile(m_fullName, std::ios::in | std::ios::binary | std::ios::ate);
    if (!srcFile.is_open())
    {
        return false;
    }
    size_t srcFileSize = srcFile.tellg();
    srcFile.seekg(0, std::ios::beg);
    /* 打开目标文件 */
    std::fstream destFile(destFilename, std::ios::out | std::ios::binary);
    if (!destFile.is_open())
    {
        srcFile.close();
        return false;
    }
    /* 计算和分配块 */
    size_t blockSize = calcBlockSize(srcFileSize, maxBlockSize);
    if (0 == blockSize)
    {
        srcFile.close();
        destFile.close();
        return false;
    }
    char* block = (char*)malloc(sizeof(char) * blockSize);
    if (!block)
    {
        srcFile.close();
        destFile.close();
        return false;
    }
    /* 拷贝文件内容 */
    size_t nowSize = 0, readSize = 0;
    while (!srcFile.eof())
    {
        memset(block, 0, sizeof(char) * blockSize);
        srcFile.read(block, blockSize);
        readSize = srcFile.gcount();
        destFile.write(block, readSize);
        nowSize += readSize;
        if (progressCb)
        {
            progressCb(nowSize, srcFileSize);
        }
    }
    free(block);
    destFile.flush();
    /* 关闭文件句柄 */
    srcFile.close();
    destFile.close();
    return true;
}

long long FileInfo::size()
{
    long long fileSize = -1;
    if (m_fullName.empty())
    {
        return fileSize;
    }
    std::fstream f(m_fullName, std::ios::in);
    if (f.is_open())
    {
        f.seekg(0, std::ios::end);
        fileSize = f.tellg();
        f.close();
    }
    return fileSize;
}

char* FileInfo::data(long long& fileSize, bool isText)
{
    if (m_fullName.empty())
    {
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
    long long dataSize = (fileSize + (isText ? 1 : 0));
    char* fileData = (char*)malloc(sizeof(char) * dataSize);
    if (fileData)
    {
        memset(fileData, 0, sizeof(char) * dataSize);
        f.read(fileData, fileSize);
        if (isText)
        {
            fileData[fileSize] = '\0';
        }
    }
    f.close();
    return fileData;
}

char* FileInfo::read(long long offset, long long& count)
{
    if (m_fullName.empty())
    {
        return NULL;
    }
    std::fstream f(m_fullName, std::ios::in | std::ios::binary | std::ios::ate);
    if (!f.is_open())
    {
        return NULL;
    }
    char* buffer = read(f, offset, count);
    f.close();
    return buffer;
}

bool FileInfo::write(const char* data, long long length, bool isAppend)
{
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
        return false;
    }
    f.write(data, length);
    f.flush();
    f.close();
    return true;
}

char* FileInfo::read(std::fstream& f, long long offset, long long& count)
{
    if (!f.is_open())
    {
        return NULL;
    }
    char* buffer = NULL;
    long long fileSize = f.tellg();
    if (offset < fileSize)
    {
        if (offset + count > fileSize)
        {
            count = fileSize - offset;
        }
        buffer = (char*)malloc(sizeof(char) * count);
        if (buffer)
        {
            memset(buffer, 0, sizeof(char) * count);
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
} // namespace utilitiy
