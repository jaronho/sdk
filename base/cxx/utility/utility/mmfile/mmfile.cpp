#include "mmfile.h"

#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace utility
{
size_t MMFile::getPageSize()
{
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwAllocationGranularity;
#else
    return sysconf(_SC_PAGESIZE);
#endif
}

MMFile::~MMFile()
{
    close();
}

bool MMFile::open(const std::string& fullName, const AccessMode& mode, size_t blockSize, size_t initialSize)
{
    close();
    m_pageSize = getPageSize();
    m_blockSize = blockSize > 0 ? blockSize : 1024;
    m_fileSize = initialSize > 0 ? initialSize : 1024;
#ifdef _WIN32
    DWORD dwDesiredAccess = 0;
    DWORD dwProtection = PAGE_READONLY;
    DWORD dwMapAccess = FILE_MAP_READ;
    DWORD dwCreationDisposition = OPEN_EXISTING;
    switch (mode)
    {
    case AccessMode::read_only:
        dwDesiredAccess = GENERIC_READ;
        break;
    case AccessMode::read_write:
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        dwProtection = PAGE_READWRITE;
        dwMapAccess = FILE_MAP_WRITE;
        break;
    case AccessMode::create:
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        dwProtection = PAGE_READWRITE;
        dwMapAccess = FILE_MAP_WRITE;
        dwCreationDisposition = CREATE_ALWAYS;
        break;
    }
    m_file =
        CreateFileA(fullName.c_str(), dwDesiredAccess, FILE_SHARE_READ, nullptr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == m_file)
    {
        m_lastError = GetLastError();
        return false;
    }
    LARGE_INTEGER li;
    if (AccessMode::create == mode)
    {
        li.QuadPart = m_fileSize;
        if (!SetFilePointerEx(m_file, li, nullptr, FILE_BEGIN) || !SetEndOfFile(m_file))
        {
            m_lastError = GetLastError();
            CloseHandle(m_file);
            return false;
        }
    }
    else
    {
        if (!GetFileSizeEx(m_file, &li))
        {
            CloseHandle(m_file);
            return false;
        }
        m_fileSize = li.QuadPart;
    }
#else
    int flags = O_CLOEXEC;
    mode_t permission = 0666;
    switch (mode)
    {
    case AccessMode::read_only:
        flags |= O_RDONLY;
        break;
    case AccessMode::read_write:
        flags |= O_RDWR;
        break;
    case AccessMode::create:
        flags |= O_RDWR | O_CREAT | O_TRUNC;
        break;
    }
    m_fd = ::open(fullName.c_str(), flags, permission);
    if (m_fd < 0)
    {
        m_lastError = errno;
        return false;
    }
    if (AccessMode::create == mode)
    {
        if (ftruncate(m_fd, m_fileSize) < 0)
        {
            m_lastError = errno;
            ::close(m_fd);
            return false;
        }
    }
    else
    {
        struct stat st;
        if (fstat(m_fd, &st) < 0)
        {
            m_lastError = errno;
            ::close(m_fd);
            return false;
        }
        m_fileSize = st.st_size;
    }
#endif
    return true;
}

bool MMFile::seek(size_t offset, int whence)
{
    if (!isOpen())
    {
        return false;
    }
    size_t newPos = 0;
    switch (whence)
    {
    case SEEK_SET:
        newPos = offset;
        break;
    case SEEK_CUR:
        newPos = m_currentPositon + offset;
        break;
    case SEEK_END:
        newPos = m_fileSize - offset;
        break;
    default:
        return false;
    }
    if (newPos < 0 || newPos > m_fileSize)
    {
        return false;
    }
    m_currentPositon = newPos;
    return true;
}

size_t MMFile::write(const void* data, size_t size)
{
    if (!isOpen())
    {
        return 0;
    }
    size_t written = 0;
    while (size > 0)
    {
        size_t blockSize = size < m_blockSize ? size : m_blockSize;
        if (!mapBlock(m_currentPositon, blockSize, AccessMode::read_write))
        {
            return written;
        }
        memcpy(static_cast<char*>(m_blockData), data, blockSize);
        if (!sync(blockSize))
        {
            unmapBlock(blockSize);
            return written;
        }
        unmapBlock(blockSize);
        m_currentPositon += blockSize;
        data = static_cast<const char*>(data) + blockSize;
        size -= blockSize;
        written += blockSize;
    }
    return written;
}

size_t MMFile::read(void* data, size_t size)
{
    if (!isOpen())
    {
        return 0;
    }
    size_t readSize = 0;
    while (size > 0 && m_currentPositon < m_fileSize)
    {
        size_t blockSize = size < m_blockSize ? size : m_blockSize;
        auto remainSize = m_fileSize - m_currentPositon;
        if (blockSize > remainSize)
        {
            blockSize = remainSize;
        }
        if (!mapBlock(m_currentPositon, blockSize, AccessMode::read_only))
        {
            return readSize;
        }
        /* 计算实际偏移 */
        size_t adjustedOffset = (m_currentPositon / m_pageSize) * m_pageSize;
        size_t offsetInBlock = m_currentPositon - adjustedOffset;
        memcpy(data, static_cast<char*>(m_blockData) + offsetInBlock, blockSize);
        unmapBlock(blockSize);
        m_currentPositon += blockSize;
        data = static_cast<char*>(data) + blockSize;
        size -= blockSize;
        readSize += blockSize;
    }
    return readSize;
}

void MMFile::close()
{
#ifdef _WIN32
    if (INVALID_HANDLE_VALUE != m_file)
    {
        CloseHandle(m_file);
        m_file = INVALID_HANDLE_VALUE;
    }
#else
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
#endif
    m_fileSize = 0;
    m_currentPositon = 0;
}

bool MMFile::isOpen() const
{
#ifdef _WIN32
    return (INVALID_HANDLE_VALUE != m_file);
#else
    return (m_fd >= 0);
#endif
}

size_t MMFile::getFileSize() const
{
    return m_fileSize;
}

void* MMFile::getBlockData() const
{
    return m_blockData;
}

size_t MMFile::getCurrentPositon() const
{
    return m_currentPositon;
}

int MMFile::getLastError() const
{
    return m_lastError;
}

void* MMFile::mapBlock(size_t offset, size_t blockSize, const AccessMode& mode)
{
    if (offset + blockSize > m_fileSize)
    {
        return nullptr;
    }
    size_t adjustedOffset = (offset / m_pageSize) * m_pageSize; /* 调整offset为页面大小的倍数 */
    size_t adjustedSize = blockSize + (offset - adjustedOffset); /* 调整blockSize */
    if (adjustedOffset + adjustedSize > m_fileSize)
    {
        adjustedSize = m_fileSize - adjustedOffset; /* 确保不超出文件末尾 */
    }
#ifdef _WIN32
    DWORD protection = (mode == AccessMode::read_write) ? PAGE_READWRITE : PAGE_READONLY;
    DWORD mapAccess = (mode == AccessMode::read_write) ? FILE_MAP_WRITE : FILE_MAP_READ;
    DWORD offsetHigh = static_cast<DWORD>((adjustedOffset >> 32) & 0xFFFFFFFF);
    DWORD offsetLow = static_cast<DWORD>(adjustedOffset & 0xFFFFFFFF);
    /* dwMaximumSizeHigh和dwMaximumSizeLow设置为0表示映射对象的大小由文件的实际大小决定 */
    m_mapping = CreateFileMapping(m_file, nullptr, protection, 0, 0, nullptr);
    if (!m_mapping)
    {
        m_lastError = GetLastError();
        return nullptr;
    }
    m_blockData = MapViewOfFile(m_mapping, mapAccess, offsetHigh, offsetLow, adjustedSize);
    if (!m_blockData)
    {
        m_lastError = GetLastError();
        CloseHandle(m_mapping);
        m_mapping = nullptr;
        return nullptr;
    }
#else
    m_blockData =
        mmap(nullptr, adjustedSize, AccessMode::read_write == mode ? PROT_READ | PROT_WRITE : PROT_READ, MAP_SHARED, m_fd, adjustedOffset);
    if (MAP_FAILED == m_blockData)
    {
        m_lastError = errno;
        return nullptr;
    }
#endif
    return m_blockData;
}

void MMFile::unmapBlock(size_t blockSize)
{
    if (m_blockData)
    {
#ifdef _WIN32
        UnmapViewOfFile(m_blockData);
#else
        munmap(m_blockData, blockSize);
#endif
        m_blockData = nullptr;
    }
#ifdef _WIN32
    if (m_mapping)
    {
        CloseHandle(m_mapping);
        m_mapping = nullptr;
    }
#endif
}

bool MMFile::sync(size_t blockSize)
{
    if (!m_blockData)
    {
        return false;
    }
#ifdef _WIN32
    return (FlushViewOfFile(m_blockData, blockSize) && FlushFileBuffers(m_file));
#else
    return (0 == msync(m_blockData, blockSize, MS_SYNC));
#endif
}
} // namespace utility
