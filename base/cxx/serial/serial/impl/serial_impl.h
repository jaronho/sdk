#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif
#include "serial_define.h"

namespace serial
{
class SerialImpl final
{
public:
    SerialImpl();

    virtual ~SerialImpl();

    /**
     * @brief 打开串口
     * @return 0-成功, 1-端口为空, 2-端口不存在, 3-打开错误, 4-配置失败
     */
    int open();

    /**
     * @brief 关闭串口
     * @return true-成功, false-失败
     */
    bool close();

    /**
     * @brief 是否已打开
     * @return true-是, false-否
     */
    bool isOpened() const;

    /**
     * @brief 获取端口
     * @return 端口
     */
    std::string getPort() const;

    /**
     * @brief 设置端口
     * @param port 端口, 例如: Windows为'COM1', Linux为'/dev/ttyS0'或'/dev/ttyUSB0'
     */
    void setPort(const std::string& port);

    /**
     * @brief 获取波特率
     * @return 波特率
     */
    unsigned long getBaudrate() const;

    /**
     * @brief 设置波特率
     * @param baudrate 波特率, 值可为: 
     *                  110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, 115200, 
     *                  128000, 153600, 230400, 256000, 460800, 500000, 921600
     */
    void setBaudrate(unsigned long baudrate);

    /**
     * @brief 获取数据位
     * @return 数据位
     */
    Databits getDatabits() const;

    /**
     * @brief 设置数据位
     * @param databits 数据位
     */
    void setDatabits(const Databits& databits);

    /**
     * @brief 获取校验位
     * @return 校验位
     */
    ParityType getParity() const;

    /**
     * @brief 设置校验位
     * @param parity 校验位
     */
    void setParity(const ParityType& parity);

    /**
     * @brief 获取停止位
     * @return 停止位
     */
    Stopbits getStopbits() const;

    /**
     * @brief 设置停止位
     * @param stopbits 停止位
     */
    void setStopbits(const Stopbits& stopbits);

    /**
     * @brief 获取流控
     * @return 流控
     */
    FlowcontrolType getFlowcontrol() const;

    /**
     * @brief 设置流控
     * @param flowcontrol 流控
     */
    void setFlowcontrol(const FlowcontrolType& flowcontrol);

    /**
     * @brief 获取超时
     * @return 超时
     */
    Timeout getTimeout() const;

    /**
     * @brief 设置读/写超时, 有两个超时条件:
     *        (1)字节间超时, 定义了串口上接收两个字节之间的最大时间(单位:毫秒), 设置为0表示禁止超时机制.
     *        (2)总时间超时
     *          2-1.定义了读/写操作的超时条件常数和乘数, 如果从读/写调用开始的总时间超过了指定的时间(单位:毫秒), 则触发此超时.
     *          2-2.超时时间=常数+请求字节数*乘数, 例如: 
     *              希望read调用在1秒后超时, 而不管请求的字节数是多少, 那么可以将readTimeoutConstant设置为1000, 将readTimeoutMultiplier设置为0.
     *              这个超时条件可以与字节间超时条件一起使用, 不会有任何问题, 当满足两个超时条件中的一个时, 就会触发超时.
     *              这允许用户最大限度地控制响应性和效率之间的权衡.
     * @param timeout 超时
     */
    void setTimeout(const Timeout& timeout);

    /**
     * @brief 获取可读字节数
     * @param 可读字节数
     */
    size_t availableForRead();

    /**
     * @brief 读数据, 有3种情况会返回:
     *        (1)读到了请求的字节数, 这种情况下, 已读的字节数将匹配请求的字节数
     *        (2)超时, 这种情况下, 读到的字节数不匹配请求的字节数, 且没有触发异常, 但是触发了超时条件
     *        (3)触发异常
     * @param buffer 缓冲区
     * @param size 缓冲区大小(要读的字节数)
     * @return 读到的字节数
     */
    size_t read(char* buffer, size_t size);

    /**
     * @brief 写数据
     * @param data 数据内容
     * @param length 数据长度
     * @return 写入的数据长度
     */
    size_t write(const char* data, size_t length);

    /**
     * @brief 刷新(输入/输出)缓冲区
     */
    void flush();

    /**
     * @brief 刷新输入缓冲区
     */
    void flushInput();

    /**
     * @brief 刷新输出缓冲区
     */
    void flushOutput();

    /**
     * @brief 获取CD
     */
    bool getCD();

    /**
     * @brief 获取CTS
     */
    bool getCTS();

    /**
     * @brief 获取DSR
     */
    bool getDSR();

    /**
     * @brief 获取RI
     */
    bool getRI();

    /**
     * @brief 设置中断
     */
    void setBreak(bool set);

    /**
     * @brief 设置DTR
     */
    void setDTR(bool set);

    /**
     * @brief 设置RTS
     */
    void setRTS(bool set);

    /**
     * @brief 等待可读, 将会阻塞直到有数据可读取, 或触发readTimeoutConstant超时
     * @param timeout 超时时间(毫秒)
     * @return true-当处于可读状态时, false-由于触发超时或中断
     */
    bool waitReadable(unsigned int timeout);

    /**
     * @brief 阻塞一段时间(根据传入的字节数计算), 可以和waitReadable一起使用, 以便从端口读取更大的数据块
     * @param count 字节数
     */
    void waitByteTimes(size_t count);

    /**
     * @brief 阻塞直到CD, CTS, DSR, RI改变或被中断
     * @return true-有变化, false-无变化
     */
    bool waitForChange();

protected:
    /**
     * @brief 重新配置
     * @return 0-成功,1-串口未打开,2-获取串口状态错误,3-数据位无效,4-校验位无效,5-停止位无效,6-流控无效,7-串口设置失败,8-超时设置失败(Windows)|自定义波特率设置失败(POSIX)
     */
    int reconfig();

private:
#ifdef _WIN32
    HANDLE m_fd; /* 串口句柄 */
#else
    int m_fd; /* 串口句柄 */
#endif
    std::string m_port; /* 串口端口 */
    unsigned long m_baudrate; /* 波特率 */
    Databits m_databits; /* 数据位 */
    ParityType m_parity; /* 校验位 */
    Stopbits m_stopbits; /* 停止位 */
    FlowcontrolType m_flowcontrol; /* 流控 */
    Timeout m_timeout; /* 读操作超时 */
#ifndef _WIN32
    unsigned int m_byteTimeNS; /* 读/写单个字节时间(单位:纳秒) */
#endif
};
} // namespace serial
