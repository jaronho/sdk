#include "file_info.h"

#include <fstream>
#include <string.h>
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

/* 文件/块大小映射 */
static const int MB = 1024 * 1024;
static const FileBlockSize s_fileBlockSizeMap[] = {{1024 * MB, 4 * MB}, {512 * MB, 4 * MB}, {256 * MB, 4 * MB},
                                                   {128 * MB, 2 * MB},  {64 * MB, 2 * MB},  {32 * MB, 2 * MB},
                                                   {16 * MB, 1 * MB},   {4 * MB, 1 * MB},   {1 * MB, 1 * MB}};

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
#ifdef _WIN32
    return (0 == _access(m_fullName.c_str(), 0));
#else
    return (0 == access(m_fullName.c_str(), F_OK));
#endif
}

bool FileInfo::create()
{
    std::fstream f(m_fullName, std::ios::out | std::ios::app);
    if (f.is_open())
    {
        f.close();
        return true;
    }
    return false;
}

bool FileInfo::remove()
{
    return 0 == ::remove(m_fullName.c_str());
}

bool FileInfo::copy(const std::string& destFilename, const std::function<void(size_t now, size_t total)>& progressCb, size_t blockMaxSize)
{
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
    /* 拷贝文件内容 */
    size_t blockSize = (MB / 64); /* 16Kb */
    const int MAP_COUNT = sizeof(s_fileBlockSizeMap) / sizeof(FileBlockSize);
    for (int i = 0; i < MAP_COUNT; ++i)
    {
        if (srcFileSize >= s_fileBlockSizeMap[i].fileSize)
        {
            blockSize = s_fileBlockSizeMap[i].blockSize;
            break;
        }
    }
    if (blockMaxSize > 0 && blockSize > blockMaxSize)
    {
        blockSize = blockMaxSize;
    }
    char* block = (char*)malloc(blockSize * sizeof(char));
    if (!block)
    {
        return false;
    }
    size_t nowSize = 0, readSize = 0;
    while (!srcFile.eof())
    {
        memset(block, 0, blockSize);
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
    std::fstream f(m_fullName, std::ios::in | std::ios::binary | std::ios::ate);
    if (!f.is_open())
    {
        fileSize = -1;
        return NULL;
    }
    fileSize = f.tellg();
    f.seekg(0, std::ios::beg);
    char* fileData = (char*)malloc((fileSize + (isText ? 1 : 0)) * sizeof(char));
    if (fileData)
    {
        memset(fileData, 0, fileSize);
        f.read(fileData, fileSize);
        if (isText)
        {
            fileData[fileSize] = '\0';
        }
    }
    f.close();
    return fileData;
}
} // namespace utilitiy
