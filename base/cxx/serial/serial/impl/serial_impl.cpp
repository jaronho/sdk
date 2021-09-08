#include "serial_impl.h"

#ifndef _WIN32
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sysexits.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#if defined(__linux__)
#include <linux/serial.h>
#endif
#ifdef __MACH__
#include <AvailabilityMacros.h>
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#ifndef TIOCINQ
#ifdef FIONREAD
#define TIOCINQ FIONREAD
#else
#define TIOCINQ 0x541B
#endif
#endif

#if defined(MAC_OS_X_VERSION_10_3) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_3)
#include <IOKit/serial/ioss.h>
#endif

#ifdef INVALID_HANDLE_VALUE
#undef INVALID_HANDLE_VALUE
#endif
#define INVALID_HANDLE_VALUE -1
#endif

namespace serial
{
#ifdef _WIN32
inline std::wstring getPrefixPortIfNeeded(const std::wstring& input)
{
    static const std::wstring winComPortPrefix = L"\\\\.\\";
    if (0 != input.compare(0, winComPortPrefix.size(), winComPortPrefix))
    {
        return winComPortPrefix + input;
    }
    return input;
}
#else
class MillisecondTimer
{
public:
    MillisecondTimer(unsigned int millis)
    {
        int64_t nsec = m_expiry.tv_nsec + (millis * 1e6);
        if (nsec >= 1e9)
        {
            int64_t secDiff = nsec / static_cast<int>(1e9);
            m_expiry.tv_nsec = nsec % static_cast<int>(1e9);
            m_expiry.tv_sec += secDiff;
        }
        else
        {
            m_expiry.tv_nsec = nsec;
        }
    }

    int64_t remaining()
    {
        timespec now(timespecNow());
        int64_t millis = (m_expiry.tv_sec - now.tv_sec) * 1e3;
        millis += (m_expiry.tv_nsec - now.tv_nsec) / 1e6;
        return millis;
    }

    static timespec timespecNow()
    {
        timespec time;
#ifdef __MACH__ /* OS X does not have clock_gettime, use clock_get_time */
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        time.tv_sec = mts.tv_sec;
        time.tv_nsec = mts.tv_nsec;
#else
        clock_gettime(CLOCK_MONOTONIC, &time);
#endif
        return time;
    }

private:
    timespec m_expiry;
};

timespec timespecFromMS(const unsigned int millis)
{
    timespec time;
    time.tv_sec = millis / 1e3;
    time.tv_nsec = (millis - (time.tv_sec * 1e3)) * 1e6;
    return time;
}
#endif

SerialImpl::SerialImpl()
    : m_fd(INVALID_HANDLE_VALUE)
    , m_baudrate(9600)
    , m_databits(Databits::EIGHT)
    , m_parity(ParityType::NONE)
    , m_stopbits(Stopbits::ONE)
    , m_flowcontrol(FlowcontrolType::NONE)
    , m_timeout(Timeout::simpleTimeout(Timeout::maxMS()))
{
}

SerialImpl::~SerialImpl()
{
    close();
}

int SerialImpl::open()
{
    if (m_port.empty())
    {
        return 1;
    }
    if (INVALID_HANDLE_VALUE != m_fd)
    {
        return 0;
    }
#ifdef _WIN32
    /* 参考: https://github.com/wjwwood/serial/issues/84 */
    std::wstring portWithPrefix = getPrefixPortIfNeeded(m_port);
    LPCWSTR lp_port = portWithPrefix.c_str();
    m_fd = CreateFileW(lp_port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        DWORD lastError = GetLastError();
        if (ERROR_FILE_NOT_FOUND == lastError)
        {
            return 2;
        }
        return 3;
    }
#else
    m_fd = ::open(m_port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        switch (errno)
        {
        case EINTR: /* 这里递归调用, 因为这是一个可恢复错误 */
            return open();
        case ENFILE:
        case EMFILE:
            return 2; /* 太多文件句柄打开 */
        default:
            return 3;
        }
    }
#endif
    if (0 != reconfig())
    {
#ifdef _WIN32
        CloseHandle(m_fd);
#else
        ::close(m_fd);
#endif
        m_fd = INVALID_HANDLE_VALUE;
        return 4;
    }
    return 0;
}

bool SerialImpl::close()
{
    if (INVALID_HANDLE_VALUE != m_fd)
    {
#ifdef _WIN32
        if (!CloseHandle(m_fd))
#else
        if (0 != ::close(m_fd))
#endif
        {
            return false;
        }
        m_fd = INVALID_HANDLE_VALUE;
    }
    return true;
}

bool SerialImpl::isOpened() const
{
    return (INVALID_HANDLE_VALUE != m_fd);
}

std::string SerialImpl::getPort() const
{
#ifdef _WIN32
    return std::string(m_port.begin(), m_port.end());
#else
    return m_port;
#endif
}

void SerialImpl::setPort(const std::string& port)
{
#ifdef _WIN32
    m_port = std::wstring(port.begin(), port.end());
#else
    m_port = port;
#endif
    if (INVALID_HANDLE_VALUE != m_fd)
    {
        if (close())
        {
            open();
        }
    }
}

unsigned long SerialImpl::getBaudrate() const
{
    return m_baudrate;
}

void SerialImpl::setBaudrate(unsigned long baudrate)
{
    m_baudrate = baudrate;
    reconfig();
}

Databits SerialImpl::getDatabits() const
{
    return m_databits;
}

void SerialImpl::setDatabits(const Databits& databits)
{
    m_databits = databits;
    reconfig();
}

ParityType SerialImpl::getParity() const
{
    return m_parity;
}

void SerialImpl::setParity(const ParityType& parity)
{
    m_parity = parity;
    reconfig();
}

Stopbits SerialImpl::getStopbits() const
{
    return m_stopbits;
}

void SerialImpl::setStopbits(const Stopbits& stopbits)
{
    m_stopbits = stopbits;
    reconfig();
}

FlowcontrolType SerialImpl::getFlowcontrol() const
{
    return m_flowcontrol;
}

void SerialImpl::setFlowcontrol(const FlowcontrolType& flowcontrol)
{
    m_flowcontrol = flowcontrol;
    reconfig();
}

Timeout SerialImpl::getTimeout() const
{
    return m_timeout;
}

void SerialImpl::setTimeout(const Timeout& timeout)
{
    m_timeout = timeout;
    reconfig();
}

size_t SerialImpl::availableForRead()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return 0;
    }
#ifdef _WIN32
    COMSTAT cs;
    if (!ClearCommError(m_fd, NULL, &cs))
    {
        return 0;
    }
    return static_cast<size_t>(cs.cbInQue);
#else
    int count = 0;
    if (-1 == ioctl(m_fd, TIOCINQ, &count))
    {
        return 0;
    }
    return static_cast<size_t>(count);
#endif
}

size_t SerialImpl::read(char* buffer, size_t size)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return 0;
    }
    if (!buffer || 0 == size)
    {
        return 0;
    }
    memset(buffer, 0, size);
#ifdef _WIN32
    DWORD len;
    if (!ReadFile(m_fd, buffer, static_cast<DWORD>(size), &len, NULL))
    {
        return 0;
    }
    return (size_t)(len);
#else
    size_t len = 0;
    /* 计算总超时时间(单位:毫秒), 常数 + (乘数 * 请求字节数) */
    long totalTimeout = m_timeout.readTimeoutConstant + m_timeout.readTimeoutMultiplier * static_cast<long>(size);
    MillisecondTimer totalTimeoutTimer(totalTimeout);
    /* 预先读取数据 */
    {
        ssize_t cnt = ::read(m_fd, buffer, size);
        if (cnt > 0)
        {
            len = cnt;
        }
    }
    while (len < size)
    {
        int64_t remainTimeout = totalTimeoutTimer.remaining();
        if (remainTimeout <= 0) /* 超时 */
        {
            break;
        }
        /* 下一个select的超时是剩余的总超时和字节间超时的最小值 */
        unsigned int timeout = std::min(static_cast<unsigned int>(remainTimeout), m_timeout.interByteTimeout);
        /* 等待可读数据 */
        if (waitReadable(timeout))
        {
            /* 如果它是一个固定长度的多字节读取, 在这里插入一个等待, 这样就可以尝试在单个IO调用中获取全部内容.
               如果指定的interByteTimeout不是最大的, 则跳过此等待. */
            if (size > 1 && m_timeout.interByteTimeout == Timeout::maxMS())
            {
                size_t cnt = availableForRead();
                if (cnt + len < size)
                {
                    waitByteTimes(size - (cnt + len));
                }
            }
            ssize_t cnt = ::read(m_fd, buffer + len, size - len);
            if (cnt < 1)
            {
                assert(0); /* 除非逻辑错误, 否则不可能进入该条件 */
            }
            len += static_cast<size_t>(cnt); /* 更新已读字节数 */
            if (len == size) /* 请求的字节数全部读完 */
            {
                break;
            }
            else if (len < size) /* 请求的字节数未读完, 继续 */
            {
                continue;
            }
            assert(0); /* 除非逻辑错误, 否则不可能进入该条件 */
        }
    }
    return len;
#endif
}

size_t SerialImpl::write(const char* data, size_t length)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return 0;
    }
    if (!data || 0 == length)
    {
        return 0;
    }
#ifdef _WIN32
    DWORD len;
    if (!WriteFile(m_fd, data, static_cast<DWORD>(length), &len, NULL))
    {
        return 0;
    }
    return (size_t)(len);
#else
    fd_set writefds;
    size_t len = 0;
    /* 计算总超时时间(单位:毫秒), 常数 + (乘数 * 请求字节数) */
    long totalTimeout = m_timeout.writeTimeoutConstant + m_timeout.writeTimeoutMultiplier * static_cast<long>(length);
    MillisecondTimer totalTimeoutTimer(totalTimeout);
    bool firstFlag = true;
    while (len < length)
    {
        int64_t remainTimeout = totalTimeoutTimer.remaining();
        /* 如果不是循环的第一次迭代, 则只考虑超时, 否则不允许超时为0 */
        if (!firstFlag && (remainTimeout <= 0)) /* 超时 */
        {
            break;
        }
        firstFlag = false;
        timespec ts(timespecFromMS(remainTimeout));
        FD_ZERO(&writefds);
        FD_SET(m_fd, &writefds);
        int ret = pselect(m_fd + 1, NULL, &writefds, NULL, &ts, NULL);
        if (ret < 0)
        {
            if (EINTR == errno) /* 中断, 继续尝试 */
            {
                continue;
            }
            assert(0); /* 其他错误 */
        }
        if (0 == ret) /* 超时 */
        {
            break;
        }
        else if (ret > 0) /* 端口准备写数据 */
        {
            if (FD_ISSET(m_fd, &writefds)) /* 确保文件描述符在写入列表中 */
            {
                ssize_t cnt = ::write(m_fd, data + len, length - len);
                if (-1 == cnt && EINTR == errno) /* 即使pselect返回了就绪状态, 调用仍然可能被中断, 在这种情况下简单地重试 */
                {
                    continue;
                }
                if (cnt < 1)
                {
                    assert(0); /* 除非逻辑错误, 否则不可能进入该条件 */
                }
                len += static_cast<size_t>(cnt); /* 更新已写字节数 */
                if (len == length) /* 请求的字节数全部写完 */
                {
                    break;
                }
                else if (len < length) /* 请求的字节数未写完, 继续 */
                {
                    continue;
                }
            }
            assert(0); /* 除非逻辑错误, 否则不可能进入该条件 */
        }
    }
    return len;
#endif
}

void SerialImpl::flush()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    FlushFileBuffers(m_fd);
#else
    tcdrain(m_fd);
#endif
}

void SerialImpl::flushInput()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    PurgeComm(m_fd, PURGE_RXCLEAR);
#else
    tcflush(m_fd, TCIFLUSH);
#endif
}

void SerialImpl::flushOutput()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    PurgeComm(m_fd, PURGE_TXCLEAR);
#else
    tcflush(m_fd, TCOFLUSH);
#endif
}

bool SerialImpl::getCTS()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return false;
    }
#ifdef _WIN32
    DWORD dwModemStatus;
    if (!GetCommModemStatus(m_fd, &dwModemStatus))
    {
        return false;
    }
    return 0 != (MS_CTS_ON & dwModemStatus);
#else
    int status;
    if (INVALID_HANDLE_VALUE == ioctl(m_fd, TIOCMGET, &status))
    {
        return false;
    }
    return 0 != (status & TIOCM_CTS);
#endif
}

bool SerialImpl::getDSR()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return false;
    }
#ifdef _WIN32
    DWORD dwModemStatus;
    if (!GetCommModemStatus(m_fd, &dwModemStatus))
    {
        return false;
    }

    return 0 != (MS_DSR_ON & dwModemStatus);
#else
    int status;
    if (-1 == ioctl(m_fd, TIOCMGET, &status))
    {
        return false;
    }
    return 0 != (status & TIOCM_DSR);
#endif
}

bool SerialImpl::getRI()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return false;
    }
#ifdef _WIN32
    DWORD dwModemStatus;
    if (!GetCommModemStatus(m_fd, &dwModemStatus))
    {
        return false;
    }
    return 0 != (MS_RING_ON & dwModemStatus);
#else
    int status;
    if (-1 == ioctl(m_fd, TIOCMGET, &status))
    {
        return false;
    }
    return 0 != (status & TIOCM_RI);
#endif
}

bool SerialImpl::getCD()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return false;
    }
#ifdef _WIN32
    DWORD dwModemStatus;
    if (!GetCommModemStatus(m_fd, &dwModemStatus))
    {
        return false;
    }
    return 0 != (MS_RLSD_ON & dwModemStatus);
#else
    int status;
    if (-1 == ioctl(m_fd, TIOCMGET, &status))
    {
        return false;
    }
    return 0 != (status & TIOCM_CD);
#endif
}

void SerialImpl::sendBreak(int duration)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    /* Windows平台不支持 */
#else
    tcsendbreak(m_fd, static_cast<int>(duration / 4));
#endif
}

void SerialImpl::setBreak(bool level)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    if (level)
    {
        EscapeCommFunction(m_fd, SETBREAK);
    }
    else
    {
        EscapeCommFunction(m_fd, CLRBREAK);
    }
#else
    if (level)
    {
        ioctl(m_fd, TIOCSBRK);
    }
    else
    {
        ioctl(m_fd, TIOCCBRK);
    }
#endif
}

void SerialImpl::setRTS(bool level)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    if (level)
    {
        EscapeCommFunction(m_fd, SETRTS);
    }
    else
    {
        EscapeCommFunction(m_fd, CLRRTS);
    }
#else
    int command = TIOCM_RTS;
    if (level)
    {
        ioctl(m_fd, TIOCMBIS, &command);
    }
    else
    {
        ioctl(m_fd, TIOCMBIC, &command);
    }
#endif
}

void SerialImpl::setDTR(bool level)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    if (level)
    {
        EscapeCommFunction(m_fd, SETDTR);
    }
    else
    {
        EscapeCommFunction(m_fd, CLRDTR);
    }
#else
    int command = TIOCM_DTR;
    if (level)
    {
        ioctl(m_fd, TIOCMBIS, &command);
    }
    else
    {
        ioctl(m_fd, TIOCMBIC, &command);
    }
#endif
}

bool SerialImpl::waitReadable(unsigned int timeout)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return false;
    }
#ifdef _WIN32
    /* TODO: 未实现 */
    return false;
#else
    /* 设置一次select调用用于阻塞 */
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_fd, &readfds);
    timespec ts(timespecFromMS(timeout));
    int ret = pselect(m_fd + 1, &readfds, NULL, NULL, &ts, NULL);
    if (ret < 0)
    {
        if (EINTR == errno) /* 中断 */
        {
            return false;
        }
        return false;
    }
    if (0 == ret) /* 超时 */
    {
        return false;
    }
    if (!FD_ISSET(m_fd, &readfds)) /*  */
    {
        assert(0); /* 除非逻辑错误, 否则不可能进入该条件 */
    }
    return true; /* 有可读数据 */
#endif
}

void SerialImpl::waitByteTimes(size_t count)
{
#ifdef _WIN32
    /* TODO: 未实现 */
#else
    timespec wait_time = {0, static_cast<long>(m_byteTimeNS * count)};
    pselect(0, NULL, NULL, NULL, &wait_time, NULL);
#endif
}

bool SerialImpl::waitForChange()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return false;
    }
#ifdef _WIN32
    if (!SetCommMask(m_fd, EV_CTS | EV_DSR | EV_RING | EV_RLSD))
    {
        return false;
    }
    DWORD dwCommEvent;
    memset(&dwCommEvent, 0, sizeof(DWORD));
    if (!WaitCommEvent(m_fd, &dwCommEvent, NULL))
    {
        return false;
    }
    return true;
#else
#ifndef TIOCMIWAIT
    while (true)
    {
        int status;
        if (-1 == ioctl(m_fd, TIOCMGET, &status))
        {
            break;
        }
        if (0 != (status & TIOCM_CTS) || 0 != (status & TIOCM_DSR) || 0 != (status & TIOCM_RI) || 0 != (status & TIOCM_CD))
        {
            return true;
        }
        usleep(1000);
    }
    return false;
#else
    int command = (TIOCM_CD | TIOCM_DSR | TIOCM_RI | TIOCM_CTS);
    if (-1 == ioctl(m_fd, TIOCMIWAIT, &command))
    {
        return false;
    }
    return true;
#endif
#endif
}

#ifdef _WIN32
int SerialImpl::reconfig()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return 1; /* 串口未打开 */
    }
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(m_fd, &dcbSerialParams))
    {
        return 2; /* 获取串口状态错误 */
    }
    /* 波特率 */
    switch (m_baudrate)
    {
#ifdef CBR_0
    case 0:
        dcbSerialParams.BaudRate = CBR_0;
        break;
#endif
#ifdef CBR_50
    case 50:
        dcbSerialParams.BaudRate = CBR_50;
        break;
#endif
#ifdef CBR_75
    case 75:
        dcbSerialParams.BaudRate = CBR_75;
        break;
#endif
#ifdef CBR_110
    case 110:
        dcbSerialParams.BaudRate = CBR_110;
        break;
#endif
#ifdef CBR_134
    case 134:
        dcbSerialParams.BaudRate = CBR_134;
        break;
#endif
#ifdef CBR_150
    case 150:
        dcbSerialParams.BaudRate = CBR_150;
        break;
#endif
#ifdef CBR_200
    case 200:
        dcbSerialParams.BaudRate = CBR_200;
        break;
#endif
#ifdef CBR_300
    case 300:
        dcbSerialParams.BaudRate = CBR_300;
        break;
#endif
#ifdef CBR_600
    case 600:
        dcbSerialParams.BaudRate = CBR_600;
        break;
#endif
#ifdef CBR_1200
    case 1200:
        dcbSerialParams.BaudRate = CBR_1200;
        break;
#endif
#ifdef CBR_1800
    case 1800:
        dcbSerialParams.BaudRate = CBR_1800;
        break;
#endif
#ifdef CBR_2400
    case 2400:
        dcbSerialParams.BaudRate = CBR_2400;
        break;
#endif
#ifdef CBR_4800
    case 4800:
        dcbSerialParams.BaudRate = CBR_4800;
        break;
#endif
#ifdef CBR_7200
    case 7200:
        dcbSerialParams.BaudRate = CBR_7200;
        break;
#endif
#ifdef CBR_9600
    case 9600:
        dcbSerialParams.BaudRate = CBR_9600;
        break;
#endif
#ifdef CBR_14400
    case 14400:
        dcbSerialParams.BaudRate = CBR_14400;
        break;
#endif
#ifdef CBR_19200
    case 19200:
        dcbSerialParams.BaudRate = CBR_19200;
        break;
#endif
#ifdef CBR_28800
    case 28800:
        dcbSerialParams.BaudRate = CBR_28800;
        break;
#endif
#ifdef CBR_57600
    case 57600:
        dcbSerialParams.BaudRate = CBR_57600;
        break;
#endif
#ifdef CBR_76800
    case 76800:
        dcbSerialParams.BaudRate = CBR_76800;
        break;
#endif
#ifdef CBR_38400
    case 38400:
        dcbSerialParams.BaudRate = CBR_38400;
        break;
#endif
#ifdef CBR_115200
    case 115200:
        dcbSerialParams.BaudRate = CBR_115200;
        break;
#endif
#ifdef CBR_128000
    case 128000:
        dcbSerialParams.BaudRate = CBR_128000;
        break;
#endif
#ifdef CBR_153600
    case 153600:
        dcbSerialParams.BaudRate = CBR_153600;
        break;
#endif
#ifdef CBR_230400
    case 230400:
        dcbSerialParams.BaudRate = CBR_230400;
        break;
#endif
#ifdef CBR_256000
    case 256000:
        dcbSerialParams.BaudRate = CBR_256000;
        break;
#endif
#ifdef CBR_460800
    case 460800:
        dcbSerialParams.BaudRate = CBR_460800;
        break;
#endif
#ifdef CBR_921600
    case 921600:
        dcbSerialParams.BaudRate = CBR_921600;
        break;
#endif
    default:
        dcbSerialParams.BaudRate = m_baudrate;
    }
    /* 数据位 */
    switch (m_databits)
    {
    case Databits::FIVE:
        dcbSerialParams.ByteSize = 5;
        break;
    case Databits::SIX:
        dcbSerialParams.ByteSize = 6;
        break;
    case Databits::SEVEN:
        dcbSerialParams.ByteSize = 7;
        break;
    case Databits::EIGHT:
        dcbSerialParams.ByteSize = 8;
        break;
    default:
        return 3; /* 数据位无效 */
    }
    /* 校验位 */
    switch (m_parity)
    {
    case ParityType::NONE:
        dcbSerialParams.Parity = NOPARITY;
        break;
    case ParityType::EVEN:
        dcbSerialParams.Parity = EVENPARITY;
        break;
    case ParityType::ODD:
        dcbSerialParams.Parity = ODDPARITY;
        break;
    case ParityType::MARK:
        dcbSerialParams.Parity = MARKPARITY;
        break;
    case ParityType::SPACE:
        dcbSerialParams.Parity = SPACEPARITY;
        break;
    default:
        return 4; /* 校验位无效 */
    }
    /* 停止位 */
    switch (m_stopbits)
    {
    case Stopbits::ONE:
        dcbSerialParams.StopBits = ONESTOPBIT;
        break;
    case Stopbits::ONE_AND_HALF:
        dcbSerialParams.StopBits = ONE5STOPBITS;
        break;
    case Stopbits::TWO:
        dcbSerialParams.StopBits = TWOSTOPBITS;
        break;
    default:
        return 5; /* 停止位无效 */
    }
    /* 流控 */
    switch (m_flowcontrol)
    {
    case FlowcontrolType::NONE:
        dcbSerialParams.fOutxCtsFlow = false;
        dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
        dcbSerialParams.fOutX = false;
        dcbSerialParams.fInX = false;
        break;
    case FlowcontrolType::SOFTWARE:
        dcbSerialParams.fOutxCtsFlow = false;
        dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
        dcbSerialParams.fOutX = true;
        dcbSerialParams.fInX = true;
        break;
    case FlowcontrolType::HARDWARE:
        dcbSerialParams.fOutxCtsFlow = true;
        dcbSerialParams.fRtsControl = RTS_CONTROL_HANDSHAKE;
        dcbSerialParams.fOutX = false;
        dcbSerialParams.fInX = false;
        break;
    default:
        return 6; /* 流控无效 */
    }
    /* 使设置生效 */
    if (!SetCommState(m_fd, &dcbSerialParams))
    {
        return 7; /* 串口设置失败 */
    }
    /* 超时 */
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = m_timeout.interByteTimeout;
    timeouts.ReadTotalTimeoutConstant = m_timeout.readTimeoutConstant;
    timeouts.ReadTotalTimeoutMultiplier = m_timeout.readTimeoutMultiplier;
    timeouts.WriteTotalTimeoutConstant = m_timeout.writeTimeoutConstant;
    timeouts.WriteTotalTimeoutMultiplier = m_timeout.writeTimeoutMultiplier;
    if (!SetCommTimeouts(m_fd, &timeouts))
    {
        return 8; /* 超时设置失败 */
    }
    return 0;
}
#else
int getDatabitsNum(const Databits& databits)
{
    switch (databits)
    {
    case Databits::FIVE:
        return 5;
    case Databits::SIX:
        return 6;
    case Databits::SEVEN:
        return 7;
    case Databits::EIGHT:
        return 8;
    default:
        return 0;
    }
}

int getParityNum(const ParityType& parity)
{
    switch (parity)
    {
    case ParityType::NONE:
        return 0;
    case ParityType::EVEN:
        return 2;
    case ParityType::ODD:
        return 1;
    case ParityType::MARK:
        return 3;
    case ParityType::SPACE:
        return 4;
    default:
        return 0;
    }
}

int getStopbitsNum(const Stopbits& stopbits)
{
    switch (stopbits)
    {
    case Stopbits::ONE:
        return 1;
    case Stopbits::ONE_AND_HALF:
        return 3;
    case Stopbits::TWO:
        return 2;
    default:
        return 0;
    }
}

int SerialImpl::reconfig()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return 1; /* 串口未打开 */
    }
    struct termios options;
    if (-1 == tcgetattr(m_fd, &options))
    {
        return 2; /* 获取串口状态错误 */
    }
    /* 设置原始模式, 无ECHO, 二进制 */
    options.c_cflag |= (tcflag_t)(CLOCAL | CREAD);
    options.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG | IEXTEN); //|ECHOPRT
    options.c_oflag &= (tcflag_t) ~(OPOST);
    options.c_iflag &= (tcflag_t) ~(INLCR | IGNCR | ICRNL | IGNBRK);
#ifdef IUCLC
    options.c_iflag &= (tcflag_t)~IUCLC;
#endif
#ifdef PARMRK
    options.c_iflag &= (tcflag_t)~PARMRK;
#endif
    /* 波特率 */
    bool isCustomBaudrate = false;
    speed_t baud;
    switch (m_baudrate)
    {
#ifdef B0
    case 0:
        baud = B0;
        break;
#endif
#ifdef B50
    case 50:
        baud = B50;
        break;
#endif
#ifdef B75
    case 75:
        baud = B75;
        break;
#endif
#ifdef B110
    case 110:
        baud = B110;
        break;
#endif
#ifdef B134
    case 134:
        baud = B134;
        break;
#endif
#ifdef B150
    case 150:
        baud = B150;
        break;
#endif
#ifdef B200
    case 200:
        baud = B200;
        break;
#endif
#ifdef B300
    case 300:
        baud = B300;
        break;
#endif
#ifdef B600
    case 600:
        baud = B600;
        break;
#endif
#ifdef B1200
    case 1200:
        baud = B1200;
        break;
#endif
#ifdef B1800
    case 1800:
        baud = B1800;
        break;
#endif
#ifdef B2400
    case 2400:
        baud = B2400;
        break;
#endif
#ifdef B4800
    case 4800:
        baud = B4800;
        break;
#endif
#ifdef B7200
    case 7200:
        baud = B7200;
        break;
#endif
#ifdef B9600
    case 9600:
        baud = B9600;
        break;
#endif
#ifdef B14400
    case 14400:
        baud = B14400;
        break;
#endif
#ifdef B19200
    case 19200:
        baud = B19200;
        break;
#endif
#ifdef B28800
    case 28800:
        baud = B28800;
        break;
#endif
#ifdef B57600
    case 57600:
        baud = B57600;
        break;
#endif
#ifdef B76800
    case 76800:
        baud = B76800;
        break;
#endif
#ifdef B38400
    case 38400:
        baud = B38400;
        break;
#endif
#ifdef B115200
    case 115200:
        baud = B115200;
        break;
#endif
#ifdef B128000
    case 128000:
        baud = B128000;
        break;
#endif
#ifdef B153600
    case 153600:
        baud = B153600;
        break;
#endif
#ifdef B230400
    case 230400:
        baud = B230400;
        break;
#endif
#ifdef B256000
    case 256000:
        baud = B256000;
        break;
#endif
#ifdef B460800
    case 460800:
        baud = B460800;
        break;
#endif
#ifdef B500000
    case 500000:
        baud = B500000;
        break;
#endif
#ifdef B576000
    case 576000:
        baud = B576000;
        break;
#endif
#ifdef B921600
    case 921600:
        baud = B921600;
        break;
#endif
#ifdef B1000000
    case 1000000:
        baud = B1000000;
        break;
#endif
#ifdef B1152000
    case 1152000:
        baud = B1152000;
        break;
#endif
#ifdef B1500000
    case 1500000:
        baud = B1500000;
        break;
#endif
#ifdef B2000000
    case 2000000:
        baud = B2000000;
        break;
#endif
#ifdef B2500000
    case 2500000:
        baud = B2500000;
        break;
#endif
#ifdef B3000000
    case 3000000:
        baud = B3000000;
        break;
#endif
#ifdef B3500000
    case 3500000:
        baud = B3500000;
        break;
#endif
#ifdef B4000000
    case 4000000:
        baud = B4000000;
        break;
#endif
    default:
        isCustomBaudrate = true;
    }
    if (!isCustomBaudrate)
    {
#ifdef _BSD_SOURCE
        cfsetspeed(&options, baud);
#else
        cfsetispeed(&options, baud);
        cfsetospeed(&options, baud);
#endif
    }
    /* 数据位 */
    options.c_cflag &= (tcflag_t)~CSIZE;
    switch (m_databits)
    {
    case Databits::FIVE:
        options.c_cflag |= CS5;
        break;
    case Databits::SIX:
        options.c_cflag |= CS6;
        break;
    case Databits::SEVEN:
        options.c_cflag |= CS7;
        break;
    case Databits::EIGHT:
        options.c_cflag |= CS8;
        break;
    default:
        return 3; /* 数据位无效 */
    }
    /* 校验位 */
    options.c_iflag &= (tcflag_t) ~(INPCK | ISTRIP);
    switch (m_parity)
    {
    case ParityType::NONE:
        options.c_cflag &= (tcflag_t) ~(PARENB | PARODD);
        break;
    case ParityType::EVEN:
        options.c_cflag &= (tcflag_t) ~(PARODD);
        options.c_cflag |= (PARENB);
        break;
    case ParityType::ODD:
        options.c_cflag |= (PARENB | PARODD);
        break;
#ifdef CMSPAR
    case ParityType::MARK:
        options.c_cflag |= (PARENB | CMSPAR | PARODD);
        break;
    case ParityType::SPACE:
        options.c_cflag |= (PARENB | CMSPAR);
        options.c_cflag &= (tcflag_t) ~(PARODD);
        break;
#else
    case ParityType::MARK:
    case ParityType::SPACE:
        return 4; /* OSX没有定义CMSPAR, 所以不支持MARK和SPACE */
#endif
    default:
        return 4; /* 校验位无效 */
    }
    /* 停止位 */
    switch (m_stopbits)
    {
    case Stopbits::ONE:
        options.c_cflag &= (tcflag_t) ~(CSTOPB);
        break;
    case Stopbits::ONE_AND_HALF: /* POSIX不支持1.5位停止位 */
        options.c_cflag |= (CSTOPB);
        break;
    case Stopbits::TWO:
        options.c_cflag |= (CSTOPB);
        break;
    default:
        return 5; /* 停止位无效 */
    }
    /* 流控 */
    bool xonxoff = false;
    bool rtscts = false;
    switch (m_flowcontrol)
    {
    case FlowcontrolType::NONE:
        xonxoff = false;
        rtscts = false;
        break;
    case FlowcontrolType::SOFTWARE:
        xonxoff = true;
        rtscts = false;
        break;
    case FlowcontrolType::HARDWARE:
        xonxoff = false;
        rtscts = true;
        break;
    default:
        return 6; /* 流控无效 */
    }
#ifdef IXANY
    if (xonxoff)
    {
        options.c_iflag |= (IXON | IXOFF /*| IXANY*/);
    }
    else
    {
        options.c_iflag &= (tcflag_t) ~(IXON | IXOFF | IXANY);
    }
#else
    if (xonxoff)
    {
        options.c_iflag |= (IXON | IXOFF);
    }
    else
    {
        options.c_iflag &= (tcflag_t) ~(IXON | IXOFF);
    }
#endif
#ifdef CRTSCTS
    if (rtscts)
    {
        options.c_cflag |= (CRTSCTS);
    }
    else
    {
        options.c_cflag &= (unsigned long)~(CRTSCTS);
    }
#elif defined CNEW_RTSCTS
    if (rtscts)
    {
        options.c_cflag |= (CNEW_RTSCTS);
    }
    else
    {
        options.c_cflag &= (unsigned long)~(CNEW_RTSCTS);
    }
#else
#error "OS Support seems wrong."
#endif
    /* 参考: http://www.unixwiz.net/techtips/termios-vmin-vtime.html
       这基本上将read调用设置为轮询读取, 但我们使用select来确保在每次调用之前有可用的数据可以读取, 因此我们永远不应该进行不必要的轮询 */
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;
    /* 使设置生效 */
    if (-1 == tcsetattr(m_fd, TCSANOW, &options))
    {
        return 7; /* 串口设置失败 */
    }
    if (isCustomBaudrate) /* 应用自定义波特率 */
    {
#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4) /* OSX支持 */
        /* Starting with Tiger, the IOSSIOSPEED ioctl can be used to set arbitrary baud rates other than those specified by POSIX.
           The driver for the underlying serial hardware ultimately determines which baud rates can be used. This ioctl sets both 
           the input and output speed. */
        speed_t newBaudrate = static_cast<speed_t>(m_baudrate);
        if (-1 == ioctl(m_fd, IOSSIOSPEED, &newBaudrate, 1)) /* PySerial uses IOSSIOSPEED=0x80045402 */
        {
            return 8; /* 自定义波特率设置失败 */
        }
#elif defined(__linux__) && defined(TIOCSSERIAL) /* Linux支持 */
        struct serial_struct ser;
        if (-1 == ioctl(m_fd, TIOCGSERIAL, &ser))
        {
            return 8; /* 自定义波特率设置失败 */
        }
        /* 设置自定义除数 */
        ser.custom_divisor = ser.baud_base / static_cast<int>(m_baudrate);
        /* 更新标志位 */
        ser.flags &= ~ASYNC_SPD_MASK;
        ser.flags |= ASYNC_SPD_CUST;
        if (-1 == ioctl(m_fd, TIOCSSERIAL, &ser))
        {
            return 8; /* 自定义波特率设置失败 */
        }
#else
        throw std::invalid_argument("OS does not currently support custom bauds");
#endif
    }
    /* 使用新设置更新读/写单个字节时间 */
    unsigned int bitTimeNS = 1e9 / m_baudrate;
    int databitsNum = getDatabitsNum(m_databits);
    int parityNum = getParityNum(m_parity);
    int stopbitsNum = getStopbitsNum(m_stopbits);
    m_byteTimeNS = bitTimeNS * (1 + databitsNum + parityNum + stopbitsNum);
    /* 对1.5位停止位进行补偿, 使用整数值3而不是1.5 */
    if (Stopbits::ONE_AND_HALF == m_stopbits)
    {
        m_byteTimeNS += ((1.5 - stopbitsNum) * bitTimeNS);
    }
    return 0;
}
#endif
} // namespace serial
