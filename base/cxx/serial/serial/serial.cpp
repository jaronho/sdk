#include "serial.h"

#include <algorithm>
#include <chrono>
#include <thread>

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
    m_impl = std::make_shared<SerialImpl>();
}

int Serial::open()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->open();
}

bool Serial::close()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->close();
}

bool Serial::isOpened()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->isOpened();
}

std::string Serial::getPort()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getPort();
}

void Serial::setPort(const std::string& port)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setPort(port);
}

unsigned long Serial::getBaudrate()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getBaudrate();
}

void Serial::setBaudrate(unsigned long baudrate)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setBaudrate(baudrate);
}

Databits Serial::getDatabits()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getDatabits();
}

void Serial::setDatabits(const Databits& databits)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setDatabits(databits);
}

ParityType Serial::getParity()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getParity();
}

void Serial::setParity(const ParityType& parity)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setParity(parity);
}

Stopbits Serial::getStopbits()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getStopbits();
}

void Serial::setStopbits(const Stopbits& stopbits)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setStopbits(stopbits);
}

FlowcontrolType Serial::getFlowcontrol()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getFlowcontrol();
}

void Serial::setFlowcontrol(const FlowcontrolType& flowcontrol)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setFlowcontrol(flowcontrol);
}

Timeout Serial::getTimeout()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getTimeout();
}

void Serial::setTimeout(const Timeout& timeout)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setTimeout(timeout);
}

ssize_t Serial::availableForRead()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->availableForRead();
}

ssize_t Serial::read(char* buffer, size_t size)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->read(buffer, size);
}

std::string Serial::read(size_t size)
{
    std::string bytes;
    if (0 == size)
    {
        return bytes;
    }
    std::lock_guard<std::mutex> locker(m_mutex);
    char* buf = (char*)malloc(size);
    if (buf)
    {
        auto len = m_impl->read(buf, size);
        if (len > 0)
        {
            bytes.append(buf, len);
        }
        free(buf);
    }
    return bytes;
}

std::string Serial::readAll(ssize_t* availableCount)
{
    static const size_t BUF_SIZE = 1025;
    char buf[BUF_SIZE] = {0};
    std::string bytes;
    std::lock_guard<std::mutex> locker(m_mutex);
    auto canReadCount = m_impl->availableForRead();
    if (availableCount)
    {
        *availableCount = canReadCount;
    }
    while (canReadCount > 0)
    {
        size_t willReadCount = BUF_SIZE - 1;
        if (canReadCount < willReadCount)
        {
            willReadCount = canReadCount;
        }
        auto len = m_impl->read(buf, willReadCount);
        if (willReadCount != len)
        {
            break;
        }
        bytes.append(buf, len);
        canReadCount -= len;
    }
    return bytes;
}

std::string Serial::readUntil(const std::string& flag, size_t msec)
{
    if (flag.empty() && msec <= 0)
    {
        return std::string();
    }
    char buf[2] = {0};
    auto stp = std::chrono::steady_clock::now();
    std::string bytes;
    std::lock_guard<std::mutex> locker(m_mutex);
    while (1)
    {
        auto len = m_impl->read(buf, 1);
        if (1 == len)
        {
            bytes.push_back(buf[0]);
            if (!flag.empty() && bytes.size() >= flag.size() && std::equal(flag.rbegin(), flag.rend(), bytes.rbegin())) /* 读完 */
            {
                break;
            }
        }
        if (msec > 0 && (std::chrono::steady_clock::now() - stp >= std::chrono::milliseconds(msec))) /* 超时 */
        {
            break;
        }
        if (1 != len)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    return bytes;
}

ssize_t Serial::write(const char* data, size_t size)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->write(data, size);
}

ssize_t Serial::write(const std::string& data)
{
    if (data.empty())
    {
        return 0;
    }
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->write(data.c_str(), data.size());
}

ssize_t Serial::write(const std::vector<char>& data)
{
    if (data.empty())
    {
        return 0;
    }
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->write(&data[0], data.size());
}

void Serial::flush()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->flush();
}

void Serial::flushInput()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->flushInput();
}

void Serial::flushOutput()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->flushOutput();
}

bool Serial::getCD()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getCD();
}

bool Serial::getCTS()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getCTS();
}

bool Serial::getDSR()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getDSR();
}

bool Serial::getRI()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_impl->getRI();
}

void Serial::setBreak(bool set)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setBreak(set);
}

void Serial::setDTR(bool set)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setDTR(set);
}

void Serial::setRTS(bool set)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_impl->setRTS(set);
}

bool Serial::waitReadable(unsigned int timeout)
{
    return m_impl->waitReadable(timeout);
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
