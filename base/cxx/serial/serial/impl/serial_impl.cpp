#include "serial_impl.h"

#ifdef _WIN32
#include <Windows.h>
#else
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
static std::string getPrefixPortIfNeeded(const std::string& input)
{
    static const std::string winComPortPrefix = "\\\\.\\";
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
        timespec now = timespecNow();
        unsigned long long nsec = now.tv_nsec + (millis * 1e6);
        if (nsec >= 1e9)
        {
            m_expiry.tv_sec = now.tv_sec + nsec / static_cast<unsigned int>(1e9);
            m_expiry.tv_nsec = nsec % static_cast<unsigned int>(1e9);
        }
        else
        {
            m_expiry.tv_sec = now.tv_sec;
            m_expiry.tv_nsec = nsec;
        }
    }

    long remaining()
    {
        timespec now = timespecNow();
        long long nowNsec = now.tv_sec * 1e9 + now.tv_nsec;
        long long expiryNsec = m_expiry.tv_sec * 1e9 + m_expiry.tv_nsec;
        return (expiryNsec - nowNsec) / 1e6;
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

timespec timespecFromMS(unsigned int millis)
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
    , m_databits(Databits::eight)
    , m_parity(ParityType::none)
    , m_stopbits(Stopbits::one)
    , m_flowcontrol(FlowcontrolType::none)
    , m_timeout(Timeout::simpleTimeout(0))
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
    std::string portWithPrefix = getPrefixPortIfNeeded(m_port);
    LPCSTR portName = portWithPrefix.c_str();
    m_fd = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return 2;
    }
#else
    bool portAlreadyOpened = false;
    FILE* stream = popen(("fuser " + m_port).c_str(), "r"); /* 判断串口文件是否已被其他进程打开 */
    if (stream)
    {
        char buf[1] = {0};
        if (fread(buf, 1, 1, stream) > 0)
        {
            portAlreadyOpened = true;
        }
        pclose(stream);
    }
    if (portAlreadyOpened)
    {
        return 4;
    }
    m_fd = ::open(m_port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK); /* 非阻塞 */
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        if (EINTR == errno) /* 这里递归调用, 因为这是一个可恢复错误 */
        {
            return open();
        }
        return 2;
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
        return 3;
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

bool SerialImpl::isOpened()
{
    return (availableForRead() >= 0);
}

std::string SerialImpl::getPort() const
{
    return m_port;
}

void SerialImpl::setPort(const std::string& port)
{
    if (0 != port.compare(m_port))
    {
        m_port = port;
        if (INVALID_HANDLE_VALUE != m_fd)
        {
            if (close())
            {
                open();
            }
        }
    }
}

unsigned long SerialImpl::getBaudrate() const
{
    return m_baudrate;
}

void SerialImpl::setBaudrate(unsigned long baudrate)
{
    if (baudrate != m_baudrate)
    {
        m_baudrate = baudrate;
        reconfig();
    }
}

Databits SerialImpl::getDatabits() const
{
    return m_databits;
}

void SerialImpl::setDatabits(const Databits& databits)
{
    if (databits != m_databits)
    {
        m_databits = databits;
        reconfig();
    }
}

ParityType SerialImpl::getParity() const
{
    return m_parity;
}

void SerialImpl::setParity(const ParityType& parity)
{
    if (parity != m_parity)
    {
        m_parity = parity;
        reconfig();
    }
}

Stopbits SerialImpl::getStopbits() const
{
    return m_stopbits;
}

void SerialImpl::setStopbits(const Stopbits& stopbits)
{
    if (stopbits != m_stopbits)
    {
        m_stopbits = stopbits;
        reconfig();
    }
}

FlowcontrolType SerialImpl::getFlowcontrol() const
{
    return m_flowcontrol;
}

void SerialImpl::setFlowcontrol(const FlowcontrolType& flowcontrol)
{
    if (flowcontrol != m_flowcontrol)
    {
        m_flowcontrol = flowcontrol;
        reconfig();
    }
}

Timeout SerialImpl::getTimeout() const
{
    return m_timeout;
}

void SerialImpl::setTimeout(const Timeout& timeout)
{
    if (timeout != m_timeout)
    {
        m_timeout = timeout;
        reconfig();
    }
}

ssize_t SerialImpl::availableForRead()
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return -1;
    }
#ifdef _WIN32
    COMSTAT cs;
    if (!ClearCommError(m_fd, NULL, &cs))
    {
        m_fd = INVALID_HANDLE_VALUE;
        return -1;
    }
    return cs.cbInQue;
#else
    int count = 0;
    if (ioctl(m_fd, FIONREAD, &count) < 0)
    {
        m_fd = INVALID_HANDLE_VALUE;
        return -1;
    }
    return count;
#endif
}

ssize_t SerialImpl::read(char* buffer, size_t size)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return -1;
    }
    if (!buffer || 0 == size)
    {
        return 0;
    }
    memset(buffer, 0, size);
    ssize_t len = 0;
#ifdef _WIN32
    DWORD count;
    if (!ReadFile(m_fd, buffer, static_cast<DWORD>(size), &count, NULL))
    {
        m_fd = INVALID_HANDLE_VALUE;
        return -1;
    }
    len = count;
#else
    /* 计算总超时时间(单位:毫秒), 常数 + (乘数 * 请求字节数) */
    unsigned int totalTimeout = m_timeout.readTimeoutConstant + m_timeout.readTimeoutMultiplier * static_cast<unsigned int>(size);
    MillisecondTimer timeoutTimer(totalTimeout);
    while (len < size)
    {
        ssize_t count = ::read(m_fd, buffer + len, size - len);
        if (count < 0)
        {
            m_fd = INVALID_HANDLE_VALUE;
            return -1;
        }
        else if (0 == count)
        {
            /* 超时时间为剩余的总超时和字节间超时的最小值 */
            if (std::min(timeoutTimer.remaining(), static_cast<long>(m_timeout.interByteTimeout)) <= 0)
            {
                break;
            }
            continue;
        }
        len += count; /* 更新已读字节数 */
    }
#endif
    return len;
}

ssize_t SerialImpl::write(const char* data, size_t length)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return -1;
    }
    if (!data || 0 == length)
    {
        return 0;
    }
#ifdef _WIN32
    DWORD len;
    if (!WriteFile(m_fd, data, static_cast<DWORD>(length), &len, NULL))
    {
        m_fd = INVALID_HANDLE_VALUE;
        return -1;
    }
    return len;
#else
    ssize_t len = 0;
    /* 计算总超时时间(单位:毫秒), 常数 + (乘数 * 请求字节数) */
    unsigned int totalTimeout = m_timeout.writeTimeoutConstant + m_timeout.writeTimeoutMultiplier * static_cast<unsigned int>(length);
    MillisecondTimer timeoutTimer(totalTimeout);
    while (len < length)
    {
        ssize_t count = ::write(m_fd, data + len, length - len);
        if (count < 0)
        {
            m_fd = INVALID_HANDLE_VALUE;
            return -1;
        }
        else if (0 == count)
        {
            /* 超时时间为剩余的总超时和字节间超时的最小值 */
            if (std::min(timeoutTimer.remaining(), static_cast<long>(m_timeout.interByteTimeout)) <= 0)
            {
                break;
            }
            continue;
        }
        len += count; /* 更新已写字节数 */
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
    if (ioctl(m_fd, TIOCMGET, &status) < 0)
    {
        return false;
    }
    return 0 != (status & TIOCM_CD);
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
    if (ioctl(m_fd, TIOCMGET, &status) < 0)
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
    if (ioctl(m_fd, TIOCMGET, &status) < 0)
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
    if (ioctl(m_fd, TIOCMGET, &status) < 0)
    {
        return false;
    }
    return 0 != (status & TIOCM_RI);
#endif
}

void SerialImpl::setBreak(bool set)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    if (set)
    {
        EscapeCommFunction(m_fd, SETBREAK);
    }
    else
    {
        EscapeCommFunction(m_fd, CLRBREAK);
    }
#else
    if (set)
    {
        ioctl(m_fd, TIOCSBRK);
    }
    else
    {
        ioctl(m_fd, TIOCCBRK);
    }
#endif
}

void SerialImpl::setDTR(bool set)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    if (set)
    {
        EscapeCommFunction(m_fd, SETDTR);
    }
    else
    {
        EscapeCommFunction(m_fd, CLRDTR);
    }
#else
    int command = TIOCM_DTR;
    if (set)
    {
        ioctl(m_fd, TIOCMBIS, &command);
    }
    else
    {
        ioctl(m_fd, TIOCMBIC, &command);
    }
#endif
}

void SerialImpl::setRTS(bool set)
{
    if (INVALID_HANDLE_VALUE == m_fd)
    {
        return;
    }
#ifdef _WIN32
    if (set)
    {
        EscapeCommFunction(m_fd, SETRTS);
    }
    else
    {
        EscapeCommFunction(m_fd, CLRRTS);
    }
#else
    int command = TIOCM_RTS;
    if (set)
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
    return true;
#else
    /* 设置一次select调用用于阻塞 */
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_fd, &readfds);
    timespec ts = timespecFromMS(timeout);
    int ret = pselect(m_fd + 1, &readfds, NULL, NULL, &ts, NULL);
    if (ret < 0)
    {
        if (EINTR == errno) /* 中断 */
        {
            return false;
        }
        return false;
    }
    else if (0 == ret) /* 超时 */
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
    if (!SetCommMask(m_fd, EV_RLSD | EV_CTS | EV_DSR | EV_RING))
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
    while (1)
    {
        int status;
        if (ioctl(m_fd, TIOCMGET, &status) < 0)
        {
            break;
        }
        if (0 != (status & TIOCM_CD) || 0 != (status & TIOCM_CTS) || 0 != (status & TIOCM_DSR) || 0 != (status & TIOCM_RI))
        {
            return true;
        }
        usleep(1000);
    }
    return false;
#else
    int command = (TIOCM_CD | TIOCM_CTS | TIOCM_DSR | TIOCM_RI);
    if (ioctl(m_fd, TIOCMIWAIT, &command) < 0)
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
    case Databits::five:
        dcbSerialParams.ByteSize = 5;
        break;
    case Databits::six:
        dcbSerialParams.ByteSize = 6;
        break;
    case Databits::seven:
        dcbSerialParams.ByteSize = 7;
        break;
    case Databits::eight:
        dcbSerialParams.ByteSize = 8;
        break;
    default:
        return 3; /* 数据位无效 */
    }
    /* 校验位 */
    switch (m_parity)
    {
    case ParityType::none:
        dcbSerialParams.Parity = NOPARITY;
        break;
    case ParityType::even:
        dcbSerialParams.Parity = EVENPARITY;
        break;
    case ParityType::odd:
        dcbSerialParams.Parity = ODDPARITY;
        break;
    case ParityType::mark:
        dcbSerialParams.Parity = MARKPARITY;
        break;
    case ParityType::space:
        dcbSerialParams.Parity = SPACEPARITY;
        break;
    default:
        return 4; /* 校验位无效 */
    }
    /* 停止位 */
    switch (m_stopbits)
    {
    case Stopbits::one:
        dcbSerialParams.StopBits = ONESTOPBIT;
        break;
    case Stopbits::one_and_half:
        dcbSerialParams.StopBits = ONE5STOPBITS;
        break;
    case Stopbits::two:
        dcbSerialParams.StopBits = TWOSTOPBITS;
        break;
    default:
        return 5; /* 停止位无效 */
    }
    /* 流控 */
    switch (m_flowcontrol)
    {
    case FlowcontrolType::none:
        dcbSerialParams.fOutxCtsFlow = false;
        dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
        dcbSerialParams.fOutX = false;
        dcbSerialParams.fInX = false;
        break;
    case FlowcontrolType::software:
        dcbSerialParams.fOutxCtsFlow = false;
        dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
        dcbSerialParams.fOutX = true;
        dcbSerialParams.fInX = true;
        break;
    case FlowcontrolType::hardware:
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
    case Databits::five:
        return 5;
    case Databits::six:
        return 6;
    case Databits::seven:
        return 7;
    case Databits::eight:
        return 8;
    default:
        return 0;
    }
}

int getParityNum(const ParityType& parity)
{
    switch (parity)
    {
    case ParityType::none:
        return 0;
    case ParityType::even:
        return 2;
    case ParityType::odd:
        return 1;
    case ParityType::mark:
        return 3;
    case ParityType::space:
        return 4;
    default:
        return 0;
    }
}

int getStopbitsNum(const Stopbits& stopbits)
{
    switch (stopbits)
    {
    case Stopbits::one:
        return 1;
    case Stopbits::one_and_half:
        return 3;
    case Stopbits::two:
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
    bzero(&options, sizeof(options));
    /* 设置原始模式, 无ECHO, 二进制 */
    options.c_cflag |= (tcflag_t)(CLOCAL | CREAD);
    options.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG | IEXTEN);
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
    case Databits::five:
        options.c_cflag |= CS5;
        break;
    case Databits::six:
        options.c_cflag |= CS6;
        break;
    case Databits::seven:
        options.c_cflag |= CS7;
        break;
    case Databits::eight:
        options.c_cflag |= CS8;
        break;
    default:
        return 3; /* 数据位无效 */
    }
    /* 校验位 */
    options.c_iflag &= (tcflag_t) ~(INPCK | ISTRIP);
    switch (m_parity)
    {
    case ParityType::none:
        options.c_cflag &= (tcflag_t) ~(PARENB | PARODD);
        break;
    case ParityType::even:
        options.c_cflag &= (tcflag_t) ~(PARODD);
        options.c_cflag |= (PARENB);
        break;
    case ParityType::odd:
        options.c_cflag |= (PARENB | PARODD);
        break;
#ifdef CMSPAR
    case ParityType::mark:
        options.c_cflag |= (PARENB | CMSPAR | PARODD);
        break;
    case ParityType::space:
        options.c_cflag |= (PARENB | CMSPAR);
        options.c_cflag &= (tcflag_t) ~(PARODD);
        break;
#else
    case ParityType::mark:
    case ParityType::space:
        return 4; /* OSX没有定义CMSPAR, 所以不支持MARK和SPACE */
#endif
    default:
        return 4; /* 校验位无效 */
    }
    /* 停止位 */
    switch (m_stopbits)
    {
    case Stopbits::one:
        options.c_cflag &= (tcflag_t) ~(CSTOPB);
        break;
    case Stopbits::one_and_half: /* POSIX不支持1.5位停止位 */
        options.c_cflag |= (CSTOPB);
        break;
    case Stopbits::two:
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
    case FlowcontrolType::none:
        xonxoff = false;
        rtscts = false;
        break;
    case FlowcontrolType::software:
        xonxoff = true;
        rtscts = false;
        break;
    case FlowcontrolType::hardware:
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
    /* 参考: http://www.unixwiz.net/techtips/termios-vmin-vtime.html, 基本上将read调用设置为轮询读取 */
    options.c_cc[VMIN] = 0; /* 非规范模式读取时的超时时间(单位:百毫秒) */
    options.c_cc[VTIME] = 0; /* 非规范模式读取时的最小字符数 */
    /* 使设置生效 */
    tcflush(m_fd, TCIFLUSH);
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
        if (ioctl(m_fd, IOSSIOSPEED, &newBaudrate, 1) < 0) /* PySerial uses IOSSIOSPEED=0x80045402 */
        {
            return 8; /* 自定义波特率设置失败 */
        }
#elif defined(__linux__) && defined(TIOCSSERIAL) /* Linux支持 */
        struct serial_struct ser;
        if (ioctl(m_fd, TIOCGSERIAL, &ser) < 0)
        {
            return 8; /* 自定义波特率设置失败 */
        }
        /* 设置自定义除数 */
        ser.custom_divisor = ser.baud_base / static_cast<int>(m_baudrate);
        /* 更新标志位 */
        ser.flags &= ~ASYNC_SPD_MASK;
        ser.flags |= ASYNC_SPD_CUST;
        if (ioctl(m_fd, TIOCSSERIAL, &ser) < 0)
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
    if (Stopbits::one_and_half == m_stopbits)
    {
        m_byteTimeNS += ((1.5 - stopbitsNum) * bitTimeNS);
    }
    return 0;
}
#endif
} // namespace serial
