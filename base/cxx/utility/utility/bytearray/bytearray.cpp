#include "bytearray.h"

#include <stdexcept>
#include <string.h>

namespace utility
{
bool ByteArray::isBigEndium()
{
    /**
     *  int(整型, 4个字节), 值为1时的大端存储方式: 00 00 00 01, 强制转为char时, 如果主机是大端, 则值为: 00
     *  int(整型, 4个字节), 值为1时的小端存储方式: 01 00 00 00, 强制转为char时, 如果主机是小端, 则值为: 01
     */
    int32_t a = 1;
    if (0 == *(char*)&a) /* 大端 */
    {
        return true;
    }
    return false; /* 小端 */
}

int16_t ByteArray::read16(const unsigned char p[2], bool bigEndium)
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    int16_t n = 0;
    if (bigEndium)
    {
        n += (int16_t)p[0] << 8;
        n += (int16_t)p[1];
    }
    else
    {
        n += (int16_t)p[0];
        n += (int16_t)p[1] << 8;
    }
    return n;
}

void ByteArray::write16(unsigned char p[2], int16_t n, bool bigEndium)
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    if (bigEndium)
    {
        p[0] = (n >> 8) & 0xFF;
        p[1] = n & 0xFF;
    }
    else
    {
        p[0] = n & 0xFF;
        p[1] = (n >> 8) & 0xFF;
    }
}

void ByteArray::write16(std::vector<unsigned char>& v, int16_t n, bool bigEndium)
{
    if (bigEndium)
    {
        v.emplace_back((n >> 8) & 0xFF);
        v.emplace_back(n & 0xFF);
    }
    else
    {
        v.emplace_back(n & 0xFF);
        v.emplace_back((n >> 8) & 0xFF);
    }
}

int16_t ByteArray::swap16(int16_t n)
{
    return (((n & 0x00FF) << 8) | ((n & 0xFF00) >> 8));
}

int16_t ByteArray::swap16(const unsigned char p[2])
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    return (((int16_t)p[0] << 8) | (int16_t)p[1]);
}

int32_t ByteArray::read32(const unsigned char p[4], bool bigEndium)
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    int32_t n = 0;
    if (bigEndium)
    {
        n += (int32_t)p[0] << 24;
        n += (int32_t)p[1] << 16;
        n += (int32_t)p[2] << 8;
        n += (int32_t)p[3];
    }
    else
    {
        n += (int32_t)p[0];
        n += (int32_t)p[1] << 8;
        n += (int32_t)p[2] << 16;
        n += (int32_t)p[3] << 24;
    }
    return n;
}

void ByteArray::write32(unsigned char p[4], int32_t n, bool bigEndium)
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    if (bigEndium)
    {
        p[0] = (n >> 24) & 0xFF;
        p[1] = (n >> 16) & 0xFF;
        p[2] = (n >> 8) & 0xFF;
        p[3] = n & 0xFF;
    }
    else
    {
        p[0] = n & 0xFF;
        p[1] = (n >> 8) & 0xFF;
        p[2] = (n >> 16) & 0xFF;
        p[3] = (n >> 24) & 0xFF;
    }
}

void ByteArray::write32(std::vector<unsigned char>& v, int32_t n, bool bigEndium)
{
    if (bigEndium)
    {
        v.emplace_back((n >> 24) & 0xFF);
        v.emplace_back((n >> 16) & 0xFF);
        v.emplace_back((n >> 8) & 0xFF);
        v.emplace_back(n & 0xFF);
    }
    else
    {
        v.emplace_back(n & 0xFF);
        v.emplace_back((n >> 8) & 0xFF);
        v.emplace_back((n >> 16) & 0xFF);
        v.emplace_back((n >> 24) & 0xFF);
    }
}

int32_t ByteArray::swap32(int32_t n)
{
    return (((n & 0x000000FF) << 24) | ((n & 0x0000FF00) << 8) | ((n & 0x00FF0000) >> 8) | ((n & 0xFF000000) >> 24));
}

int32_t ByteArray::swap32(const unsigned char p[4])
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    return (((int32_t)p[0] << 24) | ((int32_t)p[1] << 16) | ((int32_t)p[2] << 8) | (int32_t)p[3]);
}

int64_t ByteArray::read64(const unsigned char p[8], bool bigEndium)
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    int64_t n = 0;
    if (bigEndium)
    {
        n += (int64_t)p[0] << 56;
        n += (int64_t)p[1] << 48;
        n += (int64_t)p[2] << 40;
        n += (int64_t)p[3] << 32;
        n += (int64_t)p[4] << 24;
        n += (int64_t)p[5] << 16;
        n += (int64_t)p[6] << 8;
        n += (int64_t)p[7];
    }
    else
    {
        n += (int64_t)p[0];
        n += (int64_t)p[1] << 8;
        n += (int64_t)p[2] << 16;
        n += (int64_t)p[3] << 24;
        n += (int64_t)p[4] << 32;
        n += (int64_t)p[5] << 40;
        n += (int64_t)p[6] << 48;
        n += (int64_t)p[7] << 56;
    }
    return n;
}

void ByteArray::write64(unsigned char p[8], int64_t n, bool bigEndium)
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    if (bigEndium)
    {
        p[0] = (n >> 56) & 0xFF;
        p[1] = (n >> 48) & 0xFF;
        p[2] = (n >> 40) & 0xFF;
        p[3] = (n >> 32) & 0xFF;
        p[4] = (n >> 24) & 0xFF;
        p[5] = (n >> 16) & 0xFF;
        p[6] = (n >> 8) & 0xFF;
        p[7] = n & 0xFF;
    }
    else
    {
        p[0] = n & 0xFF;
        p[1] = (n >> 8) & 0xFF;
        p[2] = (n >> 16) & 0xFF;
        p[3] = (n >> 24) & 0xFF;
        p[4] = (n >> 32) & 0xFF;
        p[5] = (n >> 40) & 0xFF;
        p[6] = (n >> 48) & 0xFF;
        p[7] = (n >> 56) & 0xFF;
    }
}

void ByteArray::write64(std::vector<unsigned char>& v, int64_t n, bool bigEndium)
{
    if (bigEndium)
    {
        v.emplace_back((n >> 56) & 0xFF);
        v.emplace_back((n >> 48) & 0xFF);
        v.emplace_back((n >> 40) & 0xFF);
        v.emplace_back((n >> 32) & 0xFF);
        v.emplace_back((n >> 24) & 0xFF);
        v.emplace_back((n >> 16) & 0xFF);
        v.emplace_back((n >> 8) & 0xFF);
        v.emplace_back(n & 0xFF);
    }
    else
    {
        v.emplace_back(n & 0xFF);
        v.emplace_back((n >> 8) & 0xFF);
        v.emplace_back((n >> 16) & 0xFF);
        v.emplace_back((n >> 24) & 0xFF);
        v.emplace_back((n >> 32) & 0xFF);
        v.emplace_back((n >> 40) & 0xFF);
        v.emplace_back((n >> 48) & 0xFF);
        v.emplace_back((n >> 56) & 0xFF);
    }
}

int64_t ByteArray::swap64(int64_t n)
{
    int32_t low = n & 0xFFFFFFFF;
    int32_t high = (n >> 32) & 0xFFFFFFFF;
    low = ((low & 0x000000FF) << 24) | ((low & 0x0000FF00) << 8) | ((low & 0x00FF0000) >> 8) | ((low & 0xFF000000) >> 24);
    high = ((high & 0x000000FF) << 24) | ((high & 0x0000FF00) << 8) | ((high & 0x00FF0000) >> 8) | ((high & 0xFF000000) >> 24);
    return ((int64_t)low << 32) | high;
}

int64_t ByteArray::swap64(const unsigned char p[8])
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    int32_t low = (int32_t)p[0] | ((int32_t)p[1] << 8) | ((int32_t)p[2] << 16) | ((int32_t)p[3] << 24);
    int32_t high = (int32_t)p[4] | ((int32_t)p[5] << 8) | ((int32_t)p[6] << 16) | ((int32_t)p[7] << 24);
    return ((int64_t)high << 32) | low;
}

uint32_t ByteArray::bcount(const std::string& value)
{
    return sizeof(uint32_t) + value.size();
}

uint32_t ByteArray::bcount(const unsigned char* value, uint32_t len)
{
    if (len >= 0)
    {
        return sizeof(uint32_t) + len;
    }
    return sizeof(uint32_t);
}

uint32_t ByteArray::bcount(const std::vector<unsigned char>& value)
{
    return sizeof(uint32_t) + value.size();
}

ByteArray::ByteArray(uint32_t totalSize)
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

bool ByteArray::allocate(uint32_t totalSize)
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
            m_buffer = (unsigned char*)p;
        }
    }
    else
    {
        m_buffer = (unsigned char*)malloc(totalSize);
    }
    if (!m_buffer)
    {
        throw std::exception(std::logic_error("var 'm_buffer' is null"));
    }
    memset(m_buffer, 0, totalSize);
    m_totalSize = totalSize;
    return true;
}

void ByteArray::print() const
{
    printf("==================== byte array: max=%d, length=%d, space=%d\n", m_totalSize, getCurrentSize(), getSpaceSize());
    for (uint32_t i = 0, len = getCurrentSize(); i < len; ++i)
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

uint32_t ByteArray::getTotalSize() const
{
    return m_totalSize;
}

uint32_t ByteArray::getCurrentSize() const
{
    return abs(int(m_writeIndex - m_readIndex));
}

uint32_t ByteArray::getSpaceSize() const
{
    return m_totalSize - m_writeIndex;
}

const unsigned char* ByteArray::getBuffer() const
{
    return m_buffer;
}

bool ByteArray::setBuffer(const unsigned char* buffer, uint32_t len)
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

bool ByteArray::readBool()
{
    unsigned char* p = read(sizeof(bool));
    if (!p)
    {
        return false;
    }
    bool r = *((bool*)p);
    return r;
}

bool ByteArray::writeBool(bool value)
{
    unsigned char* p = write(sizeof(bool));
    if (!p)
    {
        return false;
    }
    bool* w = (bool*)p;
    *w = value;
    return true;
}

char ByteArray::readChar()
{
    unsigned char* p = read(sizeof(char));
    if (!p)
    {
        return false;
    }
    char r = *((char*)p);
    return r;
}

bool ByteArray::writeChar(char value)
{
    unsigned char* p = write(sizeof(char));
    if (!p)
    {
        return false;
    }
    char* w = (char*)p;
    *w = value;
    return true;
}

unsigned char ByteArray::readUchar()
{
    unsigned char* p = read(sizeof(unsigned char));
    if (!p)
    {
        return false;
    }
    unsigned char r = *((unsigned char*)p);
    return r;
}

bool ByteArray::writeUchar(unsigned char value)
{
    unsigned char* p = write(sizeof(unsigned char));
    if (!p)
    {
        return false;
    }
    unsigned char* w = (unsigned char*)p;
    *w = value;
    return true;
}

int16_t ByteArray::readInt16()
{
    unsigned char* p = read(sizeof(int16_t));
    if (!p)
    {
        return 0;
    }
    int16_t r = *((int16_t*)p);
    return r;
}

bool ByteArray::writeInt16(int16_t value)
{
    unsigned char* p = write(sizeof(int16_t));
    if (!p)
    {
        return false;
    }
    int16_t* w = (int16_t*)p;
    *w = value;
    return true;
}

uint16_t ByteArray::readUint16()
{
    unsigned char* p = read(sizeof(uint16_t));
    if (!p)
    {
        return 0;
    }
    uint16_t r = *((uint16_t*)p);
    return r;
}

bool ByteArray::writeUint16(uint16_t value)
{
    unsigned char* p = write(sizeof(uint16_t));
    if (!p)
    {
        return false;
    }
    uint16_t* w = (uint16_t*)p;
    *w = value;
    return true;
}

int32_t ByteArray::readInt32()
{
    unsigned char* p = read(sizeof(int32_t));
    if (!p)
    {
        return 0;
    }
    int32_t r = *((int32_t*)p);
    return r;
}

bool ByteArray::writeInt32(int32_t value)
{
    unsigned char* p = write(sizeof(int32_t));
    if (!p)
    {
        return false;
    }
    int32_t* w = (int32_t*)p;
    *w = value;
    return true;
}

uint32_t ByteArray::readUint32()
{
    unsigned char* p = read(sizeof(uint32_t));
    if (!p)
    {
        return 0;
    }
    uint32_t r = *((uint32_t*)p);
    return r;
}

bool ByteArray::writeUint32(uint32_t value)
{
    unsigned char* p = write(sizeof(uint32_t));
    if (!p)
    {
        return false;
    }
    uint32_t* w = (uint32_t*)p;
    *w = value;
    return true;
}

int64_t ByteArray::readInt64()
{
    unsigned char* p = read(sizeof(int64_t));
    if (!p)
    {
        return 0;
    }
    int64_t r = *((int64_t*)p);
    return r;
}

bool ByteArray::writeInt64(int64_t value)
{
    unsigned char* p = write(sizeof(int64_t));
    if (!p)
    {
        return false;
    }
    int64_t* w = (int64_t*)p;
    *w = value;
    return true;
}

uint64_t ByteArray::readUint64()
{
    unsigned char* p = read(sizeof(uint64_t));
    if (!p)
    {
        return 0;
    }
    uint64_t r = *((uint64_t*)p);
    return r;
}

bool ByteArray::writeUint64(uint64_t value)
{
    unsigned char* p = write(sizeof(uint64_t));
    if (!p)
    {
        return false;
    }
    uint64_t* w = (uint64_t*)p;
    *w = value;
    return true;
}

float_t ByteArray::readFloat()
{
    unsigned char* p = read(sizeof(float_t));
    if (!p)
    {
        return 0.f;
    }
    float_t r = *((float_t*)p);
    return r;
}

bool ByteArray::writeFloat(float_t value)
{
    unsigned char* p = write(sizeof(float_t));
    if (!p)
    {
        return false;
    }
    float_t* w = (float_t*)p;
    *w = value;
    return true;
}

double_t ByteArray::readDouble()
{
    unsigned char* p = read(sizeof(double_t));
    if (!p)
    {
        return 0.0;
    }
    double_t r = *((double_t*)p);
    return r;
}

bool ByteArray::writeDouble(double_t value)
{
    unsigned char* p = write(sizeof(double_t));
    if (!p)
    {
        return false;
    }
    double_t* w = (double_t*)p;
    *w = value;
    return true;
}

unsigned char* ByteArray::readBytes(uint32_t& len)
{
    len = readUint32();
    if (0 == len)
    {
        return nullptr;
    }
    unsigned char* p = read(len);
    if (!p)
    {
        return nullptr;
    }
    unsigned char* r = (unsigned char*)malloc(len);
    if (!r)
    {
        throw std::exception(std::logic_error("var 'r' is null"));
    }
    memcpy(r, p, len);
    return r;
}

bool ByteArray::writeBytes(const unsigned char* value, uint32_t len)
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

void ByteArray::readBytes(std::vector<unsigned char>& bytes)
{
    bytes.clear();
    uint32_t len = readUint32();
    if (0 == len)
    {
        return;
    }
    unsigned char* p = read(len);
    if (!p)
    {
        return;
    }
    bytes.insert(bytes.end(), p, p + len);
}

bool ByteArray::writeBytes(const std::vector<unsigned char>& value)
{
    return writeBytes(value.data(), value.size());
}

char* ByteArray::readString(uint32_t& len)
{
    len = readUint32();
    if (0 == len)
    {
        return nullptr;
    }
    unsigned char* p = read(len);
    if (!p)
    {
        return nullptr;
    }
    char* r = (char*)malloc(len + (size_t)1);
    if (!r)
    {
        throw std::exception(std::logic_error("var 'r' is null"));
    }
    strcpy(r, (char*)p);
    *(r + len) = '\0';
    return r;
}

bool ByteArray::writeString(const char* value, uint32_t len)
{
    if (!writeUint32(len))
    {
        return false;
    }
    if (!value)
    {
        return false;
    }
    return copy((const unsigned char*)value, len);
}

void ByteArray::readString(std::string& str)
{
    str.clear();
    uint32_t len = readUint32();
    if (0 == len)
    {
        return;
    }
    unsigned char* p = read(len);
    if (!p)
    {
        return;
    }
    str.insert(str.end(), (char*)p, (char*)p + len);
}

bool ByteArray::writeString(const std::string& value)
{
    if (!writeUint32((uint32_t)(value.length())))
    {
        return false;
    }
    return copy((const unsigned char*)value.data(), value.length());
}

bool ByteArray::copy(const unsigned char* buf, uint32_t n)
{
    if (getSpaceSize() < n)
    {
        return false;
    }
    unsigned char* p = write(n);
    if (!p)
    {
        return false;
    }
    memcpy(p, buf, n);
    return true;
}

unsigned char* ByteArray::read(uint32_t n)
{
    if (m_readIndex + n > m_totalSize)
    {
        return nullptr;
    }
    unsigned char* p = m_buffer + m_readIndex;
    m_readIndex += n;
    return p;
}

unsigned char* ByteArray::write(uint32_t n)
{
    if (m_writeIndex + n > m_totalSize)
    {
        return nullptr;
    }
    unsigned char* p = m_buffer + m_writeIndex;
    m_writeIndex += n;
    return p;
}
} // namespace utility
