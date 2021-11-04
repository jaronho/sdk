#include "bytearray.h"

#include <stdexcept>
#include <string.h>

namespace utilitiy
{
bool ByteArray::isBigEndium(void)
{
    /**
     *  int(整型, 4个字节), 值为1时的大端存储方式: 00 00 00 01, 强制转为char时, 如果主机是大端, 则值为: 00
     *  int(整型, 4个字节), 值为1时的小端存储方式: 01 00 00 00, 强制转为char时, 如果主机是小端, 则值为: 01
     */
    int a = 1;
    if (0 == *(char*)&a) /* 大端 */
    {
        return true;
    }
    return false; /* 小端 */
}

short ByteArray::swab16(short n)
{
    return (((n & 0x00FF) << 8) | ((n & 0xFF00) >> 8));
}

short ByteArray::swab16(unsigned char p[2])
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    return ((p[0] << 8) | p[1]);
}

int ByteArray::swab32(int n)
{
    return (((n & 0x000000FF) << 24) | ((n & 0x0000FF00) << 8) | ((n & 0x00FF0000) >> 8) | ((n & 0xFF000000) >> 24));
}

int ByteArray::swab32(unsigned char p[4])
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

long long ByteArray::swab64(long long n)
{
    int low = n & 0xFFFFFFFF;
    int high = (n >> 32) & 0xFFFFFFFF;
    low = ((low & 0x000000FF) << 24) | ((low & 0x0000FF00) << 8) | ((low & 0x00FF0000) >> 8) | ((low & 0xFF000000) >> 24);
    high = ((high & 0x000000FF) << 24) | ((high & 0x0000FF00) << 8) | ((high & 0x00FF0000) >> 8) | ((high & 0xFF000000) >> 24);
    return ((long long)low << 32) | high;
}

long long ByteArray::swab64(unsigned char p[8])
{
    if (!p)
    {
        throw std::exception(std::logic_error("arg 'p' is null"));
    }
    int low = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    int high = p[4] | (p[5] << 8) | (p[6] << 16) | (p[7] << 24);
    return ((long long)high << 32) | low;
}

int ByteArray::bcount(const std::string& value)
{
    return sizeof(int) + value.size();
}

int ByteArray::bcount(const unsigned char* value, int len)
{
    if (len >= 0)
    {
        return sizeof(int) + len;
    }
    return sizeof(int);
}

int ByteArray::bcount(const std::vector<unsigned char>& value)
{
    return sizeof(int) + value.size();
}

ByteArray::ByteArray(int totalSize)
{
    if (!allocate(totalSize))
    {
        m_buffer = nullptr;
        m_totalSize = 0;
    }
    m_readIndex = 0;
    m_writeIndex = 0;
}

ByteArray::~ByteArray(void)
{
    if (m_buffer)
    {
        free(m_buffer);
        m_buffer = nullptr;
    }
}

bool ByteArray::allocate(int totalSize)
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

void ByteArray::print()
{
    printf("==================== byte array: max=%d, length=%d, space=%d\n", m_totalSize, getCurrentSize(), getSpaceSize());
    for (int i = 0, len = getCurrentSize(); i < len; ++i)
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

void ByteArray::reset(void)
{
    if (m_buffer)
    {
        memset(m_buffer, 0, m_totalSize);
    }
    m_readIndex = 0;
    m_writeIndex = 0;
}

int ByteArray::getTotalSize(void)
{
    return m_totalSize;
}

int ByteArray::getCurrentSize(void)
{
    return abs(m_writeIndex - m_readIndex);
}

int ByteArray::getSpaceSize()
{
    return m_totalSize - m_writeIndex;
}

const unsigned char* ByteArray::getBuffer(void)
{
    return m_buffer;
}

bool ByteArray::setBuffer(const unsigned char* buffer, int len)
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

bool ByteArray::readBool(void)
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

char ByteArray::readChar(void)
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

unsigned char ByteArray::readUchar(void)
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

short ByteArray::readInt16(void)
{
    unsigned char* p = read(sizeof(short));
    if (!p)
    {
        return 0;
    }
    short r = *((short*)p);
    return r;
}

bool ByteArray::writeInt16(short value)
{
    unsigned char* p = write(sizeof(short));
    if (!p)
    {
        return false;
    }
    short* w = (short*)p;
    *w = value;
    return true;
}

unsigned short ByteArray::readUint16(void)
{
    unsigned char* p = read(sizeof(unsigned short));
    if (!p)
    {
        return 0;
    }
    unsigned short r = *((unsigned short*)p);
    return r;
}

bool ByteArray::writeUint16(unsigned short value)
{
    unsigned char* p = write(sizeof(unsigned short));
    if (!p)
    {
        return false;
    }
    unsigned short* w = (unsigned short*)p;
    *w = value;
    return true;
}

int ByteArray::readInt(void)
{
    unsigned char* p = read(sizeof(int));
    if (!p)
    {
        return 0;
    }
    int r = *((int*)p);
    return r;
}

bool ByteArray::writeInt(int value)
{
    unsigned char* p = write(sizeof(int));
    if (!p)
    {
        return false;
    }
    int* w = (int*)p;
    *w = value;
    return true;
}

unsigned int ByteArray::readUint(void)
{
    unsigned char* p = read(sizeof(unsigned int));
    if (!p)
    {
        return 0;
    }
    unsigned int r = *((unsigned int*)p);
    return r;
}

bool ByteArray::writeUint(unsigned int value)
{
    unsigned char* p = write(sizeof(unsigned int));
    if (!p)
    {
        return false;
    }
    unsigned int* w = (unsigned int*)p;
    *w = value;
    return true;
}

long ByteArray::readLong(void)
{
    unsigned char* p = read(sizeof(long));
    if (!p)
    {
        return 0;
    }
    long r = *((long*)p);
    return r;
}

bool ByteArray::writeLong(long value)
{
    unsigned char* p = write(sizeof(long));
    if (!p)
    {
        return false;
    }
    long* w = (long*)p;
    *w = value;
    return true;
}

unsigned long ByteArray::readUlong(void)
{
    unsigned char* p = read(sizeof(unsigned long));
    if (!p)
    {
        return 0;
    }
    unsigned long r = *((unsigned long*)p);
    return r;
}

bool ByteArray::writeUlong(unsigned long value)
{
    unsigned char* p = write(sizeof(unsigned long));
    if (!p)
    {
        return false;
    }
    unsigned long* w = (unsigned long*)p;
    *w = value;
    return true;
}

long long ByteArray::readInt64(void)
{
    unsigned char* p = read(sizeof(long long));
    if (!p)
    {
        return 0;
    }
    long long r = *((long long*)p);
    return r;
}

bool ByteArray::writeInt64(long long value)
{
    unsigned char* p = write(sizeof(long long));
    if (!p)
    {
        return false;
    }
    long long* w = (long long*)p;
    *w = value;
    return true;
}

unsigned long long ByteArray::readUint64(void)
{
    unsigned char* p = read(sizeof(unsigned long long));
    if (!p)
    {
        return 0;
    }
    unsigned long long r = *((unsigned long long*)p);
    return r;
}

bool ByteArray::writeUint64(unsigned long long value)
{
    unsigned char* p = write(sizeof(unsigned long long));
    if (!p)
    {
        return false;
    }
    unsigned long long* w = (unsigned long long*)p;
    *w = value;
    return true;
}

float ByteArray::readFloat(void)
{
    unsigned char* p = read(sizeof(float));
    if (!p)
    {
        return 0.f;
    }
    float r = *((float*)p);
    return r;
}

bool ByteArray::writeFloat(float value)
{
    unsigned char* p = write(sizeof(float));
    if (!p)
    {
        return false;
    }
    float* w = (float*)p;
    *w = value;
    return true;
}

double ByteArray::readDouble(void)
{
    unsigned char* p = read(sizeof(double));
    if (!p)
    {
        return 0.0;
    }
    double r = *((double*)p);
    return r;
}

bool ByteArray::writeDouble(double value)
{
    unsigned char* p = write(sizeof(double));
    if (!p)
    {
        return false;
    }
    double* w = (double*)p;
    *w = value;
    return true;
}

unsigned char* ByteArray::readBytes(unsigned int& len)
{
    len = readUint();
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

bool ByteArray::writeBytes(const unsigned char* value, unsigned int len)
{
    if (!value)
    {
        return false;
    }
    if (!writeUint(len))
    {
        return false;
    }
    return copy(value, len);
}

void ByteArray::readBytes(std::vector<unsigned char>& bytes)
{
    bytes.clear();
    unsigned int len = readUint();
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

char* ByteArray::readString(unsigned int& len)
{
    len = readUint();
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

bool ByteArray::writeString(const char* value, unsigned int len)
{
    if (!value)
    {
        return false;
    }
    if (!writeUint(len))
    {
        return false;
    }
    return copy((const unsigned char*)value, len);
}

void ByteArray::readString(std::string& str)
{
    str.clear();
    unsigned int len = readUint();
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
    if (!writeUint((unsigned int)(value.length())))
    {
        return false;
    }
    return copy((const unsigned char*)value.data(), value.length());
}

bool ByteArray::copy(const unsigned char* buf, int n)
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

unsigned char* ByteArray::read(int n)
{
    if (m_readIndex + n > m_totalSize)
    {
        return nullptr;
    }
    unsigned char* p = m_buffer + m_readIndex;
    m_readIndex += n;
    return p;
}

unsigned char* ByteArray::write(int n)
{
    if (m_writeIndex + n > m_totalSize)
    {
        return nullptr;
    }
    unsigned char* p = m_buffer + m_writeIndex;
    m_writeIndex += n;
    return p;
}

} // namespace utilitiy
