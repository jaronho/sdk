#include "file_info.h"

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace utility
{
#ifdef _WIN32
static bool isutf8(const std::string& str)
{
    size_t i = 0;
    if (str.size() >= 3 && (0xEF == (unsigned char)str[0] && 0xBB == (unsigned char)str[1] && 0xBF == (unsigned char)str[2])) /* BOM */
    {
        i = 3;
    }
    unsigned int byteCount = 0; /* UTF8可用1-6个字节编码, ASCII用1个字节 */
    for (; i < str.size(); ++i)
    {
        auto ch = (unsigned char)str[i];
        if ('\0' == ch)
        {
            break;
        }
        if (0 == byteCount)
        {
            if (ch >= 0x80) /* 如果不是ASCII码, 应该是多字节符, 计算字节数 */
            {
                if (ch >= 0xFC && ch <= 0xFD)
                {
                    byteCount = 6;
                }
                else if (ch >= 0xF8)
                {
                    byteCount = 5;
                }
                else if (ch >= 0xF0)
                {
                    byteCount = 4;
                }
                else if (ch >= 0xE0)
                {
                    byteCount = 3;
                }
                else if (ch >= 0xC0)
                {
                    byteCount = 2;
                }
                else
                {
                    return false;
                }
                byteCount--;
            }
        }
        else
        {
            if (0x80 != (ch & 0xC0)) /* 多字节符的非首字节, 应为10xxxxxx */
            {
                return false;
            }
            byteCount--; /* 减到为零为止 */
        }
    }
    return (0 == byteCount);
}

static std::wstring str2wstr(const std::string& str)
{
    if (!str.empty())
    {
        auto codePage = isutf8(str) ? CP_UTF8 : CP_ACP;
        int count = MultiByteToWideChar(codePage, 0, str.c_str(), str.size(), NULL, 0);
        if (count > 0)
        {
            std::wstring wstr(count, 0);
            MultiByteToWideChar(codePage, 0, str.c_str(), str.size(), &wstr[0], count);
            return wstr;
        }
    }
    return std::wstring();
}
#endif

/**
 * @brief 计算块大小 
 * @param fileSize 文件大小
 * @param blocks 文件块列表
 * @return 块大小, 为0表示文件为空
 */
static size_t calcCopyBlockSize(size_t fileSize, std::vector<FileInfo::CopyBlock> blocks)
{
    size_t blockSize = fileSize;
    if (blocks.empty())
    {
        static const int DEFAULT_MAX_BLOCK_SIZE = 64 * 1024; /* 默认不能超过64Kb */
        if (blockSize > DEFAULT_MAX_BLOCK_SIZE)
        {
            blockSize = DEFAULT_MAX_BLOCK_SIZE;
        }
    }
    else
    {
        /* 对块大小进行从降序排序 */
        std::sort(blocks.begin(), blocks.end(), [](FileInfo::CopyBlock a, FileInfo::CopyBlock b) { return a.fileSize > b.fileSize; });
        /* 根据文件大小等级计算块大小 */
        for (size_t i = 0; i < blocks.size(); ++i)
        {
            if (fileSize >= blocks[i].fileSize)
            {
                blockSize = blocks[i].blockSize;
                break;
            }
        }
    }
    return blockSize;
}

FileInfo::FileInfo(const std::string& fullName) : m_fullName(fullName)
{
    auto pos = fullName.find_last_of("/\\");
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

std::string FileInfo::path(bool endWithSlash) const
{
    if (endWithSlash)
    {
        return m_path;
    }
    auto pos = m_path.find_last_of("/\\");
    if (pos < m_path.size())
    {
        return m_path.substr(0, pos);
    }
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
#ifdef _WIN32
    auto f = _wfopen(str2wstr(m_fullName).c_str(), L"ab+");
#else
    auto f = fopen(m_fullName.c_str(), "ab+");
#endif
    if (f)
    {
        fclose(f);
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
#ifdef _WIN32
    if (0 == ::_wremove(str2wstr(m_fullName).c_str()))
#else
    if (0 == ::remove(m_fullName.c_str()))
#endif
    {
        return true;
    }
    return false;
}

FileInfo::CopyResult FileInfo::copy(const std::string& destFilename, int* errCode,
                                    const std::function<bool(size_t now, size_t total)>& progressCb,
                                    const std::vector<FileInfo::CopyBlock>& blocks, unsigned int retryTime) const
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
#ifdef _WIN32
    auto srcFile = _wfopen(str2wstr(m_fullName).c_str(), L"rb");
#else
    auto srcFile = fopen(m_fullName.c_str(), "rb");
#endif
    if (!srcFile)
    {
        if (errCode)
        {
            *errCode = errno;
        }
        return CopyResult::src_open_failed;
    }
#ifdef _WIN32
    _fseeki64(srcFile, 0, SEEK_END);
    auto srcFileSize = _ftelli64(srcFile);
    _fseeki64(srcFile, 0, SEEK_SET);
#else
    fseeko64(srcFile, 0, SEEK_END);
    auto srcFileSize = ftello64(srcFile);
    fseeko64(srcFile, 0, SEEK_SET);
#endif
    /* 打开目标文件 */
#ifdef _WIN32
    auto destFile = _wfopen(str2wstr(destFilename).c_str(), L"wb+");
#else
    auto destFile = fopen(destFilename.c_str(), "wb+");
#endif
    if (!destFile)
    {
        if (errCode)
        {
            *errCode = errno;
        }
        fclose(srcFile);
        return CopyResult::dest_open_failed;
    }
    /* 计算和分配内存块 */
    auto blockSize = calcCopyBlockSize(srcFileSize, blocks);
    if (0 == blockSize) /* 源文件为空 */
    {
        fclose(srcFile);
        fclose(destFile);
        return CopyResult::ok;
    }
    auto block = (char*)malloc(blockSize);
    if (!block)
    {
        if (errCode)
        {
            *errCode = errno;
        }
        fclose(srcFile);
        fclose(destFile);
        return CopyResult::memory_alloc_failed;
    }
    /* 拷贝文件内容 */
    size_t nowSize = 0, readSize = 0, writeSize = 0;
    auto tp = std::chrono::steady_clock::now();
    CopyResult result = CopyResult::ok;
    while (!feof(srcFile))
    {
        memset(block, 0, blockSize);
#ifdef _WIN32
        _fseeki64(srcFile, nowSize, SEEK_SET);
#else
        fseeko64(srcFile, nowSize, SEEK_SET);
#endif
        readSize = fread(block, 1, blockSize, srcFile);
        if (0 == readSize)
        {
            if (errCode)
            {
                *errCode = errno;
            }
            if (retryTime > 0
                && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp).count() >= retryTime)
            {
                result = CopyResult::src_read_failed;
                break;
            }
            continue;
        }
        writeSize = fwrite(block, 1, readSize, destFile);
        if (0 == writeSize)
        {
            if (errCode)
            {
                *errCode = errno;
            }
            if (retryTime > 0
                && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tp).count() >= retryTime)
            {
                result = CopyResult::dest_write_failed;
                break;
            }
            continue;
        }
        nowSize += writeSize;
        tp = std::chrono::steady_clock::now();
        if (progressCb && !progressCb(nowSize, srcFileSize))
        {
            result = CopyResult::stop;
            break;
        }
    }
    /* 关闭文件句柄 */
    free(block);
    fclose(srcFile);
    if (CopyResult::ok == result)
    {
        if (nowSize == srcFileSize)
        {
            fflush(destFile);
        }
        else
        {
            result = CopyResult::size_unequal;
        }
    }
    fclose(destFile);
    return result;
}

long long FileInfo::size() const
{
    long long fileSize = -1;
    if (m_fullName.empty())
    {
        return fileSize;
    }
#ifdef _WIN32
    auto f = _wfopen(str2wstr(m_fullName).c_str(), L"rb");
#else
    auto f = fopen(m_fullName.c_str(), "rb");
#endif
    if (f)
    {
#ifdef _WIN32
        _fseeki64(f, 0, SEEK_END);
        fileSize = _ftelli64(f);
#else
        fseeko64(f, 0, SEEK_END);
        fileSize = ftello64(f);
#endif
        fclose(f);
    }
    return fileSize;
}

char* FileInfo::readAll(long long& fileSize) const
{
    if (m_fullName.empty())
    {
        fileSize = -1;
        return NULL;
    }
#ifdef _WIN32
    auto f = _wfopen(str2wstr(m_fullName).c_str(), L"rb");
#else
    auto f = fopen(m_fullName.c_str(), "rb");
#endif
    if (!f)
    {
        fileSize = -1;
        return NULL;
    }
    size_t count = 0;
    auto fileData = read(f, 0, count);
    fileSize = count;
    fclose(f);
    return fileData;
}

std::string FileInfo::readAll() const
{
    std::string fileString;
    long long fileSize;
    char* fileData = readAll(fileSize);
    if (fileData)
    {
        fileString = std::string(fileData, fileData + fileSize);
        free(fileData);
    }
    return fileString;
}

char* FileInfo::read(size_t offset, size_t& count) const
{
    if (m_fullName.empty())
    {
        return NULL;
    }
#ifdef _WIN32
    auto f = _wfopen(str2wstr(m_fullName).c_str(), L"rb");
#else
    auto f = fopen(m_fullName.c_str(), "rb");
#endif
    if (!f)
    {
        return NULL;
    }
    auto buffer = read(f, offset, count);
    fclose(f);
    return buffer;
}

bool FileInfo::write(const char* data, size_t length, bool isAppend, int* errCode) const
{
    if (errCode)
    {
        *errCode = 0;
    }
    if (!data)
    {
        return false;
    }
    if (length <= 0 && isAppend)
    {
        return false;
    }
    if (m_fullName.empty())
    {
        return false;
    }
#ifdef _WIN32
    auto f = _wfopen(str2wstr(m_fullName).c_str(), isAppend ? L"ab+" : L"wb+");
#else
    auto f = fopen(m_fullName.c_str(), isAppend ? "ab+" : "wb+");
#endif
    if (!f)
    {
        if (errCode)
        {
            *errCode = errno;
        }
        return false;
    }
    auto ret = write(f, 0, data, length);
    if (ret)
    {
        fflush(f);
    }
    else
    {
        if (errCode)
        {
            *errCode = errno;
        }
    }
    fclose(f);
    return ret;
}

bool FileInfo::write(const std::string& data, bool isAppend, int* errCode) const
{
    return write(data.c_str(), data.size(), isAppend, errCode);
}

bool FileInfo::write(size_t pos, const char* data, size_t length, int* errCode) const
{
    if (errCode)
    {
        *errCode = 0;
    }
    if (!data)
    {
        return false;
    }
    if (m_fullName.empty())
    {
        return false;
    }
    /* 如果文件不存在则需要先创建文件 */
#ifdef _WIN32
    auto f = _wfopen(str2wstr(m_fullName).c_str(), L"ab+");
#else
    auto f = fopen(m_fullName.c_str(), "ab+");
#endif
    if (!f)
    {
        if (errCode)
        {
            *errCode = errno;
        }
        return false;
    }
    fclose(f);
    /* 该模式允许在指定位置写入数据, 但需要文件已经创建 */
#ifdef _WIN32
    f = _wfopen(str2wstr(m_fullName).c_str(), L"rb+");
#else
    f = fopen(m_fullName.c_str(), "rb+");
#endif
    if (!f)
    {
        if (errCode)
        {
            *errCode = errno;
        }
        return false;
    }
    auto ret = write(f, pos, data, length);
    if (ret)
    {
        fflush(f);
    }
    else
    {
        if (errCode)
        {
            *errCode = errno;
        }
    }
    fclose(f);
    return ret;
}

bool FileInfo::write(size_t pos, const std::string& data, int* errCode) const
{
    return write(pos, data.c_str(), data.size(), errCode);
}

bool FileInfo::edit(size_t offset, size_t count, const std::function<void(char* buffer, size_t count)>& func) const
{
    if (m_fullName.empty())
    {
        return false;
    }
#ifdef _WIN32
    auto f = _wfopen(str2wstr(m_fullName).c_str(), L"rb+");
#else
    auto f = fopen(m_fullName.c_str(), "rb+");
#endif
    if (!f)
    {
        return false;
    }
    auto ret = edit(f, offset, count, func);
    if (ret)
    {
        fflush(f);
    }
    fclose(f);
    return ret;
}

bool FileInfo::editLine(const std::function<bool(size_t num, std::string& line)>& func) const
{
    if (m_fullName.empty())
    {
        return false;
    }
#ifdef _WIN32
    auto f = _wfopen(str2wstr(m_fullName).c_str(), L"rb+");
#else
    auto f = fopen(m_fullName.c_str(), "rb+");
#endif
    if (!f)
    {
        return false;
    }
    std::string buffer;
    size_t num = 0;
    bool changed = false;
    std::string line, bomFlag, endFlag;
    while (readLine(f, line, bomFlag, endFlag))
    {
        ++num;
        if (!bomFlag.empty())
        {
            buffer.append(bomFlag);
        }
        std::string temp = line;
        bool keepLine = true;
        if (func)
        {
            keepLine = func(num, temp);
        }
        if (keepLine)
        {
            buffer.append(temp).append(endFlag);
        }
        if (!keepLine || line != temp)
        {
            changed = true;
        }
    }
    bool ret = true;
    if (changed)
    {
        fclose(f);
#ifdef _WIN32
        f = _wfopen(str2wstr(m_fullName).c_str(), L"wb+");
#else
        f = fopen(m_fullName.c_str(), "wb+");
#endif
        if (!f)
        {
            return false;
        }
        ret = write(f, 0, buffer);
        if (ret)
        {
            fflush(f);
        }
    }
    fclose(f);
    return ret;
}

bool FileInfo::isTextFile(float ratio, size_t maxSampleSize, size_t bufSize) const
{
    if (m_fullName.empty())
    {
        return false;
    }
#ifdef _WIN32
    auto f = _wfopen(str2wstr(m_fullName).c_str(), L"rb");
#else
    auto f = fopen(m_fullName.c_str(), "rb");
#endif
    if (!f)
    {
        return false;
    }
    auto ret = isTextData(f, ratio, maxSampleSize, bufSize);
    fclose(f);
    return ret;
}

char* FileInfo::read(FILE* f, size_t offset, size_t& count)
{
    if (!f)
    {
        count = 0;
        return NULL;
    }
    char* buffer = NULL;
#ifdef _WIN32
    _fseeki64(f, 0, SEEK_END);
    auto fileSize = _ftelli64(f);
#else
    fseeko64(f, 0, SEEK_END);
    auto fileSize = ftello64(f);
#endif
    if (0 == count)
    {
        count = fileSize;
    }
    if (offset < fileSize)
    {
        if (offset + count > fileSize)
        {
            count = fileSize - offset;
        }
        auto buffSize = count;
        buffer = (char*)malloc(buffSize);
        if (buffer)
        {
            memset(buffer, 0, buffSize);
#ifdef _WIN32
            _fseeki64(f, offset, SEEK_SET);
#else
            fseeko64(f, offset, SEEK_SET);
#endif
            count = fread(buffer, 1, buffSize, f);
        }
    }
    else
    {
        count = 0;
    }
    return buffer;
}

bool FileInfo::readLine(FILE* f, std::string& line, std::string& bomFlag, std::string& endFlag)
{
    line.clear();
    bomFlag.clear();
    endFlag.clear();
    if (f && !feof(f))
    {
        char ch = 0;
        while (!feof(f) && '\n' != (ch = fgetc(f)))
        {
            line.push_back(ch);
        }
        if ('\n' == ch)
        {
            endFlag.push_back(ch);
        }
        /* BOM字符检测 */
        if (line.size() >= 3 && (0xEF == (unsigned char)line[0] && 0xBB == (unsigned char)line[1] && 0xBF == (unsigned char)line[2]))
        {
            bomFlag = line.substr(0, 3);
            line = line.substr(3);
        }
        /* 非显示字符检测 */
        long long lastIndex = line.size() - 1;
        if (lastIndex >= 0 && ('\r' == line[lastIndex] || 0xFF == (unsigned char)line[lastIndex]))
        {
            if ('\r' == line[lastIndex])
            {
                endFlag.insert(0, 1, '\r');
            }
            line.erase(lastIndex);
        }
        return true;
    }
    return false;
}

bool FileInfo::write(FILE* f, size_t offset, const char* data, size_t count)
{
    if (!f || !data)
    {
        return false;
    }
#ifdef _WIN32
    _fseeki64(f, offset, SEEK_SET);
#else
    fseeko64(f, offset, SEEK_SET);
#endif
    if (fwrite(data, 1, count, f) > 0)
    {
        return true;
    }
    return false;
}

bool FileInfo::write(FILE* f, size_t offset, const std::string& data)
{
    return write(f, offset, data.c_str(), data.size());
}

bool FileInfo::edit(FILE* f, size_t offset, size_t count, const std::function<void(char* srcData, size_t count)>& func)
{
    if (!f || !func)
    {
        return false;
    }
#ifdef _WIN32
    _fseeki64(f, 0, SEEK_END);
    auto fileSize = _ftelli64(f);
#else
    fseeko64(f, 0, SEEK_END);
    auto fileSize = ftello64(f);
#endif
    if (offset < fileSize)
    {
        if (offset + count > fileSize)
        {
            count = fileSize - offset;
        }
        auto buffSize = count;
        auto buffer = (char*)malloc(buffSize);
        if (buffer)
        {
            memset(buffer, 0, buffSize);
#ifdef _WIN32
            _fseeki64(f, offset, SEEK_SET);
#else
            fseeko64(f, offset, SEEK_SET);
#endif
            count = fread(buffer, 1, count, f);
            func(buffer, count);
            if (buffer)
            {
#ifdef _WIN32
                _fseeki64(f, offset, SEEK_SET);
#else
                fseeko64(f, offset, SEEK_SET);
#endif
                fwrite(buffer, 1, count, f);
                free(buffer);
                return true;
            }
        }
    }
    return false;
}

bool FileInfo::isTextData(FILE* f, float ratio, size_t maxSampleSize, size_t bufSize)
{
    if (!f)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] arg 'f' is not opened");
    }
    bufSize = bufSize > 1024 ? bufSize : 1024;
    char* buffer = (char*)malloc(bufSize);
    if (buffer)
    {
        fpos_t oriPos;
        fgetpos(f, &oriPos); /* 保存当前文件指针位置, 以便恢复 */
#ifdef _WIN32
        _fseeki64(f, 0, SEEK_SET);
#else
        fseeko64(f, 0, SEEK_SET);
#endif
        size_t totalRead = 0, illegalCount = 0; /* 当前总读写节数, 非法字符数 */
        while (!feof(f)) /* 检查文件内容 */
        {
            size_t count = fread(buffer, 1, bufSize, f);
            if (0 == count)
            {
                break;
            }
            totalRead += count;
            for (size_t i = 0; i < count; ++i)
            {
                unsigned char ch = (buffer[i]);
                if (0x00 == ch || (ch < 0x20 && !('\t' == ch || '\n' == ch || '\r' == ch))) /* 检测非文本特征 */
                {
                    if (ratio <= 1e-6f)
                    {
                        fsetpos(f, &oriPos); /* 恢复文件指针到原始位置 */
                        free(buffer);
                        return false;
                    }
                    ++illegalCount;
                    break;
                }
            }
            if (maxSampleSize > 0 && totalRead >= maxSampleSize) /* 采样足够后提前退出 */
            {
                break;
            }
        }
        fsetpos(f, &oriPos); /* 恢复文件指针到原始位置 */
        free(buffer);
        if ((double)illegalCount / totalRead >= ratio)
        {
            return false;
        }
    }
    return true;
}
} // namespace utility
