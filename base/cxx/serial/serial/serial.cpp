#include "serial.h"

#include <algorithm>

#if !defined(_WIN32) && !defined(__OpenBSD__) && !defined(__FreeBSD__)
#include <alloca.h>
#endif

#if defined(__MINGW32__)
#define alloca __builtin_alloca
#endif

namespace serial
{
Serial::Serial()
{
    m_impl = new SerialImpl();
}

Serial::~Serial()
{
    delete m_impl;
}

bool Serial::open()
{
    int ret = m_impl->open();
    return (0 == ret);
}

bool Serial::close()
{
    return m_impl->close();
}

bool Serial::isOpened() const
{
    return m_impl->isOpened();
}

std::string Serial::getPort() const
{
    return m_impl->getPort();
}

void Serial::setPort(const std::string& port)
{
    m_impl->setPort(port);
}

unsigned long Serial::getBaudrate() const
{
    return m_impl->getBaudrate();
}

void Serial::setBaudrate(unsigned long baudrate)
{
    m_impl->setBaudrate(baudrate);
}

Databits Serial::getDatabits() const
{
    return m_impl->getDatabits();
}

void Serial::setDatabits(const Databits& databits)
{
    m_impl->setDatabits(databits);
}

ParityType Serial::getParity() const
{
    return m_impl->getParity();
}

void Serial::setParity(const ParityType& parity)
{
    m_impl->setParity(parity);
}

Stopbits Serial::getStopbits() const
{
    return m_impl->getStopbits();
}

void Serial::setStopbits(const Stopbits& stopbits)
{
    m_impl->setStopbits(stopbits);
}

FlowcontrolType Serial::getFlowcontrol() const
{
    return m_impl->getFlowcontrol();
}

void Serial::setFlowcontrol(const FlowcontrolType& flowcontrol)
{
    m_impl->setFlowcontrol(flowcontrol);
}

Timeout Serial::getTimeout() const
{
    return m_impl->getTimeout();
}

void Serial::setTimeout(const Timeout& timeout)
{
    m_impl->setTimeout(timeout);
}

size_t Serial::availableForRead()
{
    return m_impl->availableForRead();
}

size_t Serial::read(char* buffer, size_t size)
{
    if (!buffer || 0 == size)
    {
        return 0;
    }
    return m_impl->read(buffer, size);
}

size_t Serial::read(std::vector<char>& buffer, size_t size)
{
    if (0 == size)
    {
        return 0;
    }
    char* bytes = new char[size];
    size_t len = m_impl->read(bytes, size);
    buffer.insert(buffer.end(), bytes, bytes + len);
    delete[] bytes;
    return len;
}

size_t Serial::read(std::string& buffer, size_t size)
{
    if (0 == size)
    {
        return 0;
    }
    char* bytes = new char[size];
    size_t len = m_impl->read(bytes, size);
    buffer.append(reinterpret_cast<const char*>(bytes), len);
    delete[] bytes;
    return len;
}

size_t Serial::readLine(std::string& buffer, size_t size, const std::string& eol)
{
    size_t eolSize = eol.size();
    if (0 == size || 0 == eolSize)
    {
        return 0;
    }
    char* bytes = static_cast<char*>(alloca(size * sizeof(char)));
    size_t totalReadLen = 0;
    while (true)
    {
        size_t len = m_impl->read(bytes + totalReadLen, 1);
        totalReadLen += len;
        if (0 == len) /* 读取1字节时超时 */
        {
            break;
        }
        if (totalReadLen < eolSize)
        {
            continue;
        }
        if (std::string(reinterpret_cast<const char*>(bytes + totalReadLen - eolSize), eolSize) == eol) /* 找到行尾标识 */
        {
            break;
        }
        if (totalReadLen == size) /* 达到行指定最大长度 */
        {
            break;
        }
    }
    buffer.append(reinterpret_cast<const char*>(bytes), totalReadLen);
    return totalReadLen;
}

std::vector<std::string> Serial::readLines(size_t size, std::string eol)
{
    std::vector<std::string> lines;
    size_t eolSize = eol.size();
    if (0 == size || 0 == eolSize)
    {
        return lines;
    }
    char* bytes = static_cast<char*>(alloca(size * sizeof(char)));
    size_t totalReadLen = 0;
    size_t startOfLine = 0;
    while (totalReadLen < size)
    {
        size_t bytes_read = m_impl->read(bytes + totalReadLen, 1);
        totalReadLen += bytes_read;
        if (0 == bytes_read) /* 读取1字节时超时 */
        {
            if (startOfLine != totalReadLen)
            {
                lines.emplace_back(std::string(reinterpret_cast<const char*>(bytes + startOfLine), totalReadLen - startOfLine));
            }
            break;
        }
        if (totalReadLen < eolSize)
        {
            continue;
        }
        if (std::string(reinterpret_cast<const char*>(bytes + totalReadLen - eolSize), eolSize) == eol) /* 找到行尾标识 */
        {
            lines.emplace_back(std::string(reinterpret_cast<const char*>(bytes + startOfLine), totalReadLen - startOfLine));
            startOfLine = totalReadLen;
        }
        if (totalReadLen == size) /* 达到所有行之和指定最大长度 */
        {
            if (startOfLine != totalReadLen)
            {
                lines.emplace_back(std::string(reinterpret_cast<const char*>(bytes + startOfLine), totalReadLen - startOfLine));
            }
            break;
        }
    }
    return lines;
}

size_t Serial::write(const char* data, size_t size)
{
    if (!data || 0 == size)
    {
        return 0;
    }
    return m_impl->write(data, size);
}

size_t Serial::write(const std::vector<char>& data)
{
    if (data.empty())
    {
        return 0;
    }
    return m_impl->write(&data[0], data.size());
}

size_t Serial::write(const std::string& data)
{
    if (data.empty())
    {
        return 0;
    }
    return m_impl->write(reinterpret_cast<const char*>(data.c_str()), data.size());
}

void Serial::flush()
{
    m_impl->flush();
}

void Serial::flushInput()
{
    m_impl->flushInput();
}

void Serial::flushOutput()
{
    m_impl->flushOutput();
}

bool Serial::getCTS()
{
    return m_impl->getCTS();
}

bool Serial::getDSR()
{
    return m_impl->getDSR();
}

bool Serial::getRI()
{
    return m_impl->getRI();
}

bool Serial::getCD()
{
    return m_impl->getCD();
}

void Serial::sendBreak(int duration)
{
    m_impl->sendBreak(duration);
}

void Serial::setBreak(bool level)
{
    m_impl->setBreak(level);
}

void Serial::setRTS(bool level)
{
    m_impl->setRTS(level);
}

void Serial::setDTR(bool level)
{
    m_impl->setDTR(level);
}

bool Serial::waitReadable()
{
    Timeout timeout(m_impl->getTimeout());
    return m_impl->waitReadable(timeout.readTimeoutConstant);
}

void Serial::waitByteTimes(size_t count)
{
    m_impl->waitByteTimes(count);
}

bool Serial::waitForChange()
{
    return m_impl->waitForChange();
}
} // namespace serial
