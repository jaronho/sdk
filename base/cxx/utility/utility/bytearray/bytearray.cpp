#include "bytearray.h"

#include <stdexcept>
#include <string.h>

namespace utility
{
BOOL ByteArray::isBigEndium()
{
    /**
     *  int(整型, 4个字节), 值为1时的大端存储方式: 00 00 00 01, 强制转为char时, 如果主机是大端, 则值为: 00
     *  int(整型, 4个字节), 值为1时的小端存储方式: 01 00 00 00, 强制转为char时, 如果主机是小端, 则值为: 01
     */
    INT32 a = 1;
    if (0 == *(CHAR*)&a) /* 大端 */
    {
        return true;
    }
    return false; /* 小端 */
}

INT16 ByteArray::swab16(INT16 n)
{
    return (((n & 0x00FF) << 8) | ((n & 0xFF00) >> 8));
}

INT16 ByteArray::swab16(const UCHAR p[2])
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    return ((p[0] << 8) | p[1]);
}

INT32 ByteArray::swab32(INT32 n)
{
    return (((n & 0x000000FF) << 24) | ((n & 0x0000FF00) << 8) | ((n & 0x00FF0000) >> 8) | ((n & 0xFF000000) >> 24));
}

INT32 ByteArray::swab32(const UCHAR p[4])
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

INT64 ByteArray::swab64(INT64 n)
{
    INT32 low = n & 0xFFFFFFFF;
    INT32 high = (n >> 32) & 0xFFFFFFFF;
    low = ((low & 0x000000FF) << 24) | ((low & 0x0000FF00) << 8) | ((low & 0x00FF0000) >> 8) | ((low & 0xFF000000) >> 24);
    high = ((high & 0x000000FF) << 24) | ((high & 0x0000FF00) << 8) | ((high & 0x00FF0000) >> 8) | ((high & 0xFF000000) >> 24);
    return ((INT64)low << 32) | high;
}

INT64 ByteArray::swab64(const UCHAR p[8])
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    INT32 low = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    INT32 high = p[4] | (p[5] << 8) | (p[6] << 16) | (p[7] << 24);
    return ((INT64)high << 32) | low;
}

UINT32 ByteArray::bcount(const std::string& value)
{
    return sizeof(UINT32) + value.size();
}

UINT32 ByteArray::bcount(const UCHAR* value, UINT32 len)
{
    if (len >= 0)
    {
        return sizeof(UINT32) + len;
    }
    return sizeof(UINT32);
}

UINT32 ByteArray::bcount(const std::vector<UCHAR>& value)
{
    return sizeof(UINT32) + value.size();
}

ByteArray::ByteArray(UINT32 totalSize)
{
    if (!allocate(totalSize))
    {
        m_buffer = nullptr;
        m_totalSize = 0;
    }
    m_readIndex = 0;
    m_writeIndex = 0;
}

ByteArray::~ByteArray()
{
    if (m_buffer)
    {
        free(m_buffer);
        m_buffer = nullptr;
    }
}

BOOL ByteArray::allocate(UINT32 totalSize)
{
    if (totalSize <= 0)
    {
        return false;
    }
    if (m_buffer)
    {
        if (totalSize != m_totalSize)
        {
            void* p = realloc(m_buffer, totalSize);
            if (!p)
            {
                throw std::exception(std::logic_error("var 'p' is null"));
            }
            m_buffer = (UCHAR*)p;
        }
    }
    else
    {
        m_buffer = (UCHAR*)malloc(totalSize);
    }
    if (!m_buffer)
    {
        throw std::exception(std::logic_error("var 'm_buffer' is null"));
    }
    memset(m_buffer, 0, totalSize);
    m_totalSize = totalSize;
    return true;
}

void ByteArray::print()
{
    printf("==================== byte array: max=%d, length=%d, space=%d\n", m_totalSize, getCurrentSize(), getSpaceSize());
    for (UINT32 i = 0, len = getCurrentSize(); i < len; ++i)
    {
        if (i > 0)
        {
            printf(" ");
        }
        printf("%02x", *(m_buffer + i));
    }
    printf("\n");
    printf("========================================\n");
}

void ByteArray::reset()
{
    if (m_buffer)
    {
        memset(m_buffer, 0, m_totalSize);
    }
    m_readIndex = 0;
    m_writeIndex = 0;
}

UINT32 ByteArray::getTotalSize()
{
    return m_totalSize;
}

UINT32 ByteArray::getCurrentSize()
{
    return abs(int(m_writeIndex - m_readIndex));
}

UINT32 ByteArray::getSpaceSize()
{
    return m_totalSize - m_writeIndex;
}

const UCHAR* ByteArray::getBuffer()
{
    return m_buffer;
}

BOOL ByteArray::setBuffer(const UCHAR* buffer, UINT32 len)
{
    if (!buffer || len <= 0)
    {
        return false;
    }
    if (allocate(len))
    {
        memcpy(m_buffer, buffer, len);
        m_writeIndex = len;
        return true;
    }
    return false;
}

BOOL ByteArray::readBool()
{
    UCHAR* p = read(sizeof(BOOL));
    if (!p)
    {
        return false;
    }
    BOOL r = *((BOOL*)p);
    return r;
}

BOOL ByteArray::writeBool(BOOL value)
{
    UCHAR* p = write(sizeof(BOOL));
    if (!p)
    {
        return false;
    }
    BOOL* w = (BOOL*)p;
    *w = value;
    return true;
}

CHAR ByteArray::readChar()
{
    UCHAR* p = read(sizeof(CHAR));
    if (!p)
    {
        return false;
    }
    CHAR r = *((CHAR*)p);
    return r;
}

BOOL ByteArray::writeChar(CHAR value)
{
    UCHAR* p = write(sizeof(CHAR));
    if (!p)
    {
        return false;
    }
    CHAR* w = (CHAR*)p;
    *w = value;
    return true;
}

UCHAR ByteArray::readUchar()
{
    UCHAR* p = read(sizeof(UCHAR));
    if (!p)
    {
        return false;
    }
    UCHAR r = *((UCHAR*)p);
    return r;
}

BOOL ByteArray::writeUchar(UCHAR value)
{
    UCHAR* p = write(sizeof(UCHAR));
    if (!p)
    {
        return false;
    }
    UCHAR* w = (UCHAR*)p;
    *w = value;
    return true;
}

INT16 ByteArray::readInt16()
{
    UCHAR* p = read(sizeof(INT16));
    if (!p)
    {
        return 0;
    }
    INT16 r = *((INT16*)p);
    return r;
}

BOOL ByteArray::writeInt16(INT16 value)
{
    UCHAR* p = write(sizeof(INT16));
    if (!p)
    {
        return false;
    }
    INT16* w = (INT16*)p;
    *w = value;
    return true;
}

UINT16 ByteArray::readUint16()
{
    UCHAR* p = read(sizeof(UINT16));
    if (!p)
    {
        return 0;
    }
    UINT16 r = *((UINT16*)p);
    return r;
}

BOOL ByteArray::writeUint16(UINT16 value)
{
    UCHAR* p = write(sizeof(UINT16));
    if (!p)
    {
        return false;
    }
    UINT16* w = (UINT16*)p;
    *w = value;
    return true;
}

INT32 ByteArray::readInt32()
{
    UCHAR* p = read(sizeof(INT32));
    if (!p)
    {
        return 0;
    }
    INT32 r = *((INT32*)p);
    return r;
}

BOOL ByteArray::writeInt32(INT32 value)
{
    UCHAR* p = write(sizeof(INT32));
    if (!p)
    {
        return false;
    }
    INT32* w = (INT32*)p;
    *w = value;
    return true;
}

UINT32 ByteArray::readUint32()
{
    UCHAR* p = read(sizeof(UINT32));
    if (!p)
    {
        return 0;
    }
    UINT32 r = *((UINT32*)p);
    return r;
}

BOOL ByteArray::writeUint32(UINT32 value)
{
    UCHAR* p = write(sizeof(UINT32));
    if (!p)
    {
        return false;
    }
    UINT32* w = (UINT32*)p;
    *w = value;
    return true;
}

INT64 ByteArray::readInt64()
{
    UCHAR* p = read(sizeof(INT64));
    if (!p)
    {
        return 0;
    }
    INT64 r = *((INT64*)p);
    return r;
}

BOOL ByteArray::writeInt64(INT64 value)
{
    UCHAR* p = write(sizeof(INT64));
    if (!p)
    {
        return false;
    }
    INT64* w = (INT64*)p;
    *w = value;
    return true;
}

UINT64 ByteArray::readUint64()
{
    UCHAR* p = read(sizeof(UINT64));
    if (!p)
    {
        return 0;
    }
    UINT64 r = *((UINT64*)p);
    return r;
}

BOOL ByteArray::writeUint64(UINT64 value)
{
    UCHAR* p = write(sizeof(UINT64));
    if (!p)
    {
        return false;
    }
    UINT64* w = (UINT64*)p;
    *w = value;
    return true;
}

FLOAT ByteArray::readFloat()
{
    UCHAR* p = read(sizeof(FLOAT));
    if (!p)
    {
        return 0.f;
    }
    FLOAT r = *((FLOAT*)p);
    return r;
}

BOOL ByteArray::writeFloat(FLOAT value)
{
    UCHAR* p = write(sizeof(FLOAT));
    if (!p)
    {
        return false;
    }
    FLOAT* w = (FLOAT*)p;
    *w = value;
    return true;
}

DOUBLE ByteArray::readDouble()
{
    UCHAR* p = read(sizeof(DOUBLE));
    if (!p)
    {
        return 0.0;
    }
    DOUBLE r = *((DOUBLE*)p);
    return r;
}

BOOL ByteArray::writeDouble(DOUBLE value)
{
    UCHAR* p = write(sizeof(DOUBLE));
    if (!p)
    {
        return false;
    }
    DOUBLE* w = (DOUBLE*)p;
    *w = value;
    return true;
}

UCHAR* ByteArray::readBytes(UINT32& len)
{
    len = readUint32();
    if (0 == len)
    {
        return nullptr;
    }
    UCHAR* p = read(len);
    if (!p)
    {
        return nullptr;
    }
    UCHAR* r = (UCHAR*)malloc(len);
    if (!r)
    {
        throw std::exception(std::logic_error("var 'r' is null"));
    }
    memcpy(r, p, len);
    return r;
}

BOOL ByteArray::writeBytes(const UCHAR* value, UINT32 len)
{
    if (!writeUint32(len))
    {
        return false;
    }
    if (!value)
    {
        return false;
    }
    return copy(value, len);
}

void ByteArray::readBytes(std::vector<UCHAR>& bytes)
{
    bytes.clear();
    UINT32 len = readUint32();
    if (0 == len)
    {
        return;
    }
    UCHAR* p = read(len);
    if (!p)
    {
        return;
    }
    bytes.insert(bytes.end(), p, p + len);
}

BOOL ByteArray::writeBytes(const std::vector<UCHAR>& value)
{
    return writeBytes(value.data(), value.size());
}

CHAR* ByteArray::readString(UINT32& len)
{
    len = readUint32();
    if (0 == len)
    {
        return nullptr;
    }
    UCHAR* p = read(len);
    if (!p)
    {
        return nullptr;
    }
    CHAR* r = (CHAR*)malloc(len + (size_t)1);
    if (!r)
    {
        throw std::exception(std::logic_error("var 'r' is null"));
    }
    strcpy(r, (CHAR*)p);
    *(r + len) = '\0';
    return r;
}

BOOL ByteArray::writeString(const CHAR* value, UINT32 len)
{
    if (!writeUint32(len))
    {
        return false;
    }
    if (!value)
    {
        return false;
    }
    return copy((const UCHAR*)value, len);
}

void ByteArray::readString(std::string& str)
{
    str.clear();
    UINT32 len = readUint32();
    if (0 == len)
    {
        return;
    }
    UCHAR* p = read(len);
    if (!p)
    {
        return;
    }
    str.insert(str.end(), (CHAR*)p, (CHAR*)p + len);
}

BOOL ByteArray::writeString(const std::string& value)
{
    if (!writeUint32((UINT32)(value.length())))
    {
        return false;
    }
    return copy((const UCHAR*)value.data(), value.length());
}

BOOL ByteArray::copy(const UCHAR* buf, UINT32 n)
{
    if (getSpaceSize() < n)
    {
        return false;
    }
    UCHAR* p = write(n);
    if (!p)
    {
        return false;
    }
    memcpy(p, buf, n);
    return true;
}

UCHAR* ByteArray::read(UINT32 n)
{
    if (m_readIndex + n > m_totalSize)
    {
        return nullptr;
    }
    UCHAR* p = m_buffer + m_readIndex;
    m_readIndex += n;
    return p;
}

UCHAR* ByteArray::write(UINT32 n)
{
    if (m_writeIndex + n > m_totalSize)
    {
        return nullptr;
    }
    UCHAR* p = m_buffer + m_writeIndex;
    m_writeIndex += n;
    return p;
}
} // namespace utility
