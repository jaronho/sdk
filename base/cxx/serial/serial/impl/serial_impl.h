#pragma once
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
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
     * @return 0-成功, 1-端口为空, 2-打开失败, 3-配置失败, 4-串口已被其他进程打开(Linux)
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
    bool isOpened();

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
     * @param 可读字节数, 小于0表示异常(需要重新打开串口)
     */
    ssize_t availableForRead();

    /**
     * @brief 读数据, 有3种情况会返回:
     *        (1)读到了请求的字节数, 这种情况下, 已读的字节数将匹配请求的字节数
     *        (2)超时, 这种情况下, 读到的字节数不匹配请求的字节数, 且没有触发异常, 但是触发了超时条件
     *        (3)触发异常
     * @param buffer 缓冲区
     * @param size 缓冲区大小(要读的字节数)
     * @return 读到的字节数, 小于0表示异常(需要重新打开串口)
     */
    ssize_t read(char* buffer, size_t size);

    /**
     * @brief 写数据
     * @param data 数据内容
     * @param length 数据长度
     * @return 写入的数据长度, 小于0表示异常(需要重新打开串口)
     */
    ssize_t write(const char* data, size_t length);

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
     * @brief 获取CD(Carrier Detect - 载波检测)信号状态, 用于检测远程设备是否在线并发送有效载波信号, 在串口直连场景中常用于检测对端设备是否就绪.
     *        在modem通信中, CD信号表示拨号连接已建立, 也可用于检测设备热插拔状态
     * @return <0: 发生错误(串口未打开或API调用失败), 0: 信号无效(未检测到信号, 远程设备未连接或掉线), >0: 信号有效(检测到信号, 远程设备在线)
     */
    int getCD();

    /**
     * @brief 获取CTS(Clear to Send - 清除发送)信号状态, 硬件流控制的核心信号, 表示对端设备是否准备好接收数据, 用于防止发送方溢出接收方缓冲区.
     *        常与RTS信号配合使用实现硬件流控制, 在工业控制中可作为通用数字输入信号
     * @return <0: 发生错误(串口未打开或API调用失败), 0: 信号无效(对端设备忙, 应暂停发送数据), >0: 信号有效(对端设备就绪, 可以发送数据)
     */
    int getCTS();

    /**
     * @brief 获取DSR(Data Set Ready - 数据集就绪)信号状态, 表示DCE设备(如modem)已完成初始化并准备好建立通信链路.
     *        DTR/DSR信号对用于设备就绪状态的握手, 可用于判断设备电源/连接状态
     * @return <0: 发生错误(串口未打开或API调用失败), 0: 信号无效(设备未就绪或正在初始化), >0: 信号有效(设备已就绪)
     */
    int getDSR();

    /**
     * @brief 获取RI(Ring Indicator - 振铃指示)信号状态, 在modem应用中检测电话线路是否有来电振铃信号.
     *        主要用于传统modem来电检测, 现代USB转串口适配器可能不支持此信号
     * @return <0: 发生错误(串口未打开或API调用失败), 0: 信号无效(无来电), >0: 信号有效(检测到振铃信号)
     */
    int getRI();

    /**
     * @brief 设置BREAK信号状态, BREAK信号会将串口的TxD线路强制拉低(SPACE状态)并持续一段时间，常用于:
     *        1. 向远程设备发送异常通知;
     *        2. 触发远程设备重置;
     *        3. 进入设备特殊引导模式(如某些bootloader);
     *        典型应用: 在Windows下调用EscapeCommFunction(SETBREAK), Linux下调用ioctl(TIOCSBRK), 持续发送BREAK信号会阻塞正常数据传输, 使用后务必及时关闭
     * @param flag 信号状态, 值: true-激活信号(线路被强制拉低), false-取消信号(线路恢复正常)
     */
    void setBreak(bool flag);

    /**
     * @brief 设置DTR(Data Terminal Ready - 数据终端就绪)信号状态, DTR信号表示DTE设备(如计算机)已就绪, 可与远程DCE设备建立通信链路,
     *        与DSR信号配合使用构成设备就绪状态的握手协议, 典型应用:
     *        1. 硬件握手, 配合DSR信号建立可靠通信链路;
     *        2. 设备控制, 控制modem挂线/摘机, 或外部设备电源管理;
     *        3. 半双工通信, 触发对端设备进入接收/发送模式切换;
     * @param flag 信号状态, 值: true-激活信号(向对端表示本设备已就绪), false-关闭信号(通常会导致对端设备挂起或断开连接)
     */
    void setDTR(bool flag);

    /**
     * @brief 设置RTS(Request To Send - 请求发送)信号状态, RTS信号是硬件流控制的核心, 用于请求发送数据权限, 与CTS信号配合使用,
     *        当CTS/RTS流控启用时, DTE通过设置RTS通知DCE本端准备发送数据, 典型应用:
     *        1. 硬件流控, 防止高速传输时接收方缓冲区溢出;
     *        2. 半双工通信, 控制RS-485收发器的发送/接收模式切换;
     *        3. 通用GPIO, 作为可编程输出信号控制外部设备;
     * @param flag 信号状态, 值: true-激活信号(请求发送数据, 对端CTS有效时可立即发送), false-关闭信号(暂停数据发送请求)
     */
    void setRTS(bool flag);

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
    typedef void* HANDLE;
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
    unsigned int m_byteTimeNS; /* 读/写单个字节时间(单位:纳秒) */
};
} // namespace serial
