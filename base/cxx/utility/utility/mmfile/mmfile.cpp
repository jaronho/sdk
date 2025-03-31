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
MMFile::~MMFile()
{
    close();
}

bool MMFile::open(const std::string& fullName, const AccessMode& mode, size_t blockSize, size_t initialSize)
{
    close();
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
    m_fd = open(fullName.c_str(), flags, permission);
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
            close(m_fd);
            return false;
        }
    }
    else
    {
        struct stat st;
        if (fstat(m_fd, &st) < 0)
        {
            m_lastError = errno;
            close(m_fd);
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
    switch (whence)
    {
    case SEEK_SET:
        m_currentPositon = offset;
        break;
    case SEEK_CUR:
        m_currentPositon += offset;
        break;
    case SEEK_END:
        m_currentPositon = m_fileSize - offset;
        break;
    default:
        return false;
    }
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
        size_t blockSize = min(size, m_blockSize);
        if (!mapBlock(m_currentPositon, blockSize, AccessMode::read_write))
        {
            return written;
        }
        memcpy(static_cast<char*>(m_blockData), data, blockSize);
        if (!sync())
        {
            return written;
        }
        unmapBlock();
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
    while (size > 0)
    {
        size_t blockSize = min(size, m_blockSize);
        if (!mapBlock(m_currentPositon, blockSize, AccessMode::read_only))
        {
            return readSize;
        }
        memcpy(data, m_blockData, blockSize);
        unmapBlock();
        m_currentPositon += blockSize;
        data = static_cast<char*>(data) + blockSize;
        size -= blockSize;
        readSize += blockSize;
    }
    return readSize;
}

void MMFile::close()
{
    unmapBlock();
#ifdef _WIN32
    if (INVALID_HANDLE_VALUE != m_file)
    {
        CloseHandle(m_file);
        m_file = INVALID_HANDLE_VALUE;
    }
#else
    if (m_fd >= 0)
    {
        close(m_fd);
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
    unmapBlock();
#ifdef _WIN32
    m_mapping = CreateFileMapping(m_file, nullptr, AccessMode::read_write == mode ? PAGE_READWRITE : PAGE_READONLY, 0, blockSize, nullptr);
    if (!m_mapping)
    {
        m_lastError = GetLastError();
        return nullptr;
    }
    m_blockData = MapViewOfFile(m_mapping, AccessMode::read_write == mode ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, blockSize);
    if (!m_blockData)
    {
        m_lastError = GetLastError();
        CloseHandle(m_mapping);
        m_mapping = nullptr;
        return nullptr;
    }
#else
    m_blockData = mmap(nullptr, blockSize, AccessMode::read_write == mode ? PROT_READ | PROT_WRITE : PROT_READ, MAP_SHARED, m_fd, offset);
    if (MAP_FAILED == m_blockData)
    {
        m_lastError = errno;
        return nullptr;
    }
#endif
    return m_blockData;
}

void MMFile::unmapBlock()
{
    if (m_blockData)
    {
#ifdef _WIN32
        UnmapViewOfFile(m_blockData);
#else
        munmap(m_blockData, m_blockSize);
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

bool MMFile::sync()
{
    if (!m_blockData)
    {
        return false;
    }
#ifdef _WIN32
    return (FlushViewOfFile(m_blockData, m_currentPositon) && FlushFileBuffers(m_file));
#else
    return (0 == msync(m_blockData, m_currentPositon, MS_SYNC));
#endif
}
} // namespace utility
