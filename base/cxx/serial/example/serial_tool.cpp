#include <chrono>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "../serial/serial.h"

serial::Serial g_com; /* 串口对象 */
std::chrono::steady_clock::time_point g_lastRecvTimestamp; /* 上次接收的时间戳 */
size_t g_totalRecvLength = 0; /* 总的接收长度(字节) */

std::string getDateTime()
{
    struct tm t;
    time_t now;
    time(&now);
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    t = *localtime(&now);
#endif
    char buf1[20] = {0};
    strftime(buf1, sizeof(buf1), "%Y-%m-%d %H:%M:%S", &t);
    char buf2[4] = {0};
    struct timeb tb;
    ftime(&tb);
#ifdef _WIN32
    sprintf_s(buf2, sizeof(buf2), "%03d", tb.millitm);
#else
    sprintf(buf2, "%03d", tb.millitm);
#endif
    return std::string(buf1).append(".").append(buf2);
}

/**
 * @brief 十六进制字符串转字节流
 * @param hexStr 待转换的十六进制字符串
 * @param bytes 保存转换后的字节流
 */
int hexStrToBytes(const std::string& hexStr, char** bytes)
{
    *bytes = nullptr;
    std::string str;
    for (size_t i = 0; i < hexStr.size(); ++i)
    {
        if (' ' != hexStr[i])
        {
            str.push_back(hexStr[i]);
        }
    }
    if (0 != (str.size() % 2))
    {
        return 0;
    }
    *bytes = (char*)malloc(str.size() / 2);
    if (!(*bytes))
    {
        return 0;
    }
    unsigned char highByte, lowByte;
    for (size_t i = 0; i < str.size(); i += 2)
    {
        /* 转换成大写字母 */
        highByte = toupper(str[i]);
        lowByte = toupper(str[i + 1]);
        /* 转换编码 */
        if (highByte > 0x39)
        {
            highByte -= 0x37;
        }
        else
        {
            highByte -= 0x30;
        }
        if (lowByte > 0x39)
        {
            lowByte -= 0x37;
        }
        else
        {
            lowByte -= 0x30;
        }
        /* 高4位和低4位合并成一个字节 */
        (*bytes)[i / 2] = (highByte << 4) | lowByte;
    }
    return (str.size() / 2);
}

/**
 * @brief 显示所有串口端口 
 */
void showAllPorts(const std::vector<serial::PortInfo> portList)
{
    printf("==================== 当前识别到的所有串口 ====================\n");
    for (size_t i = 0, portCount = portList.size(); i < portCount; ++i)
    {
        if (i > 0)
        {
            printf("----------------------------------------\n");
        }
        if (portCount < 10)
        {
            printf("[%zu] 端口号: %s\n", (i + 1), portList[i].port.c_str());
            printf("    硬件ID: %s\n", portList[i].hardwareId.c_str());
            printf("    描  述: %s\n", portList[i].description.c_str());
            printf("  本地属性: %s\n", portList[i].location.c_str());
        }
        else if (portCount < 100)
        {
            printf("[%02zu] 端口号: %s\n", (i + 1), portList[i].port.c_str());
            printf("     硬件ID: %s\n", portList[i].hardwareId.c_str());
            printf("     描  述: %s\n", portList[i].description.c_str());
            printf("   本地属性: %s\n", portList[i].location.c_str());
        }
        else
        {
            printf("[%03zu] 端口号: %s\n", (i + 1), portList[i].port.c_str());
            printf("      硬件ID: %s\n", portList[i].hardwareId.c_str());
            printf("      描  述: %s\n", portList[i].description.c_str());
            printf("    本地属性: %s\n", portList[i].location.c_str());
        }
        if (i == portCount - 1)
        {
            printf("----------------------------------------\n");
        }
    }
    printf("\n");
}

/**
 * @brief 打开串口 
 */
void openSerial(const std::string& port, unsigned long baudrate, const serial::Databits& databits, const serial::ParityType& parity,
                const serial::Stopbits& stopbits, const serial::FlowcontrolType& flowcontrol, int crlf, bool showTime, bool sendHex,
                bool showHex, bool autoLine, bool hideRecv)
{
    /* 串口设置及打开 */
    g_com.setPort(port);
    g_com.setBaudrate(baudrate);
    g_com.setDatabits(databits);
    g_com.setParity(parity);
    g_com.setStopbits(stopbits);
    g_com.setFlowcontrol(flowcontrol);
    printf("端口号: %s\n", port.c_str());
    printf("波特率: %ld\n", baudrate);
    switch (databits)
    {
    case serial::Databits::five:
        printf("数据位: 5\n");
        break;
    case serial::Databits::six:
        printf("数据位: 6\n");
        break;
    case serial::Databits::seven:
        printf("数据位: 7\n");
        break;
    case serial::Databits::eight:
        printf("数据位: 8\n");
        break;
    }
    switch (parity)
    {
    case serial::ParityType::none:
        printf("校验位: None\n");
        break;
    case serial::ParityType::odd:
        printf("校验位: Odd\n");
        break;
    case serial::ParityType::even:
        printf("校验位: Even\n");
        break;
    case serial::ParityType::mark:
        printf("校验位: Mark\n");
        break;
    case serial::ParityType::space:
        printf("校验位: Space\n");
        break;
    }
    switch (stopbits)
    {
    case serial::Stopbits::one:
        printf("停止位: 1\n");
        break;
    case serial::Stopbits::one_and_half:
        printf("停止位: 1.5\n");
        break;
    case serial::Stopbits::two:
        printf("停止位: 2\n");
        break;
    }
    switch (flowcontrol)
    {
    case serial::FlowcontrolType::none:
        printf("流  控: None\n");
        break;
    case serial::FlowcontrolType::software:
        printf("流  控: Software\n");
        break;
    case serial::FlowcontrolType::hardware:
        printf("流  控: Hardware\n");
        break;
    }
    if (1 == crlf)
    {
        printf("结束符: CR(0x0D 回车)\n");
    }
    else if (2 == crlf)
    {
        printf("结束符: LF(0x0A 换行)\n");
    }
    else if (3 == crlf)
    {
        printf("结束符: CRLF(0x0D 0x0A 回车换行)\n");
    }
    else
    {
        printf("结束符: 无\n");
    }
    printf("显示时间: %s\n", showTime ? "显示" : "隐藏");
    printf("发送格式: %s\n", sendHex ? "Hex(十六进制)" : "ASCII字符");
    printf("接收格式: %s\n", showHex ? "Hex(十六进制)" : "ASCII字符");
    printf("接收换行: %s\n", autoLine ? "自动换行" : "不自动换行");
    printf("接收显示: %s\n", hideRecv ? "隐藏" : "显示");
    printf("\n");
    auto ret = g_com.open();
    if (0 != ret)
    {
        if (1 == ret)
        {
            printf("串口打开失败, 端口为空!\n");
        }
        else if (2 == ret)
        {
#ifdef _WIN32
            DWORD dw = GetLastError();
            LPVOID lpMsgBuf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
            printf("串口打开失败, 错误码: %d, 错误描述: %s\n", dw, (char*)lpMsgBuf);
            LocalFree(lpMsgBuf);
#else
            printf("串口打开失败, 错误码: %d, 错误描述: %s\n", errno, strerror(errno));
#endif
        }
        else if (3 == ret)
        {
            printf("串口打开失败, 配置失败\n");
        }
        else if (4 == ret) /* Linux平台 */
        {
            printf("串口打开失败, 该串口已被其他进程打开\n");
        }
        else
        {
            printf("串口打开失败[%d]\n", ret);
        }
        return;
    }
    printf("串口打开成功.\n");
    /* 开线程用于发送 */
    std::thread th([&, crlf]() {
        std::string endFlag, endFlagHex;
        if (1 == crlf)
        {
            endFlag = "\r";
            endFlagHex = "0D";
        }
        else if (2 == crlf)
        {
            endFlag = "\n";
            endFlagHex = "0A";
        }
        else if (3 == crlf)
        {
            endFlag = "\r\n";
            endFlagHex = "0D0A";
        }
        while (g_com.isOpened())
        {
            char input[1024] = {0};
            std::cin.getline(input, sizeof(input));
            if (strlen(input) == 0)
            {
                continue;
            }
            if (sendHex)
            {
                char* bytes;
                auto len = hexStrToBytes(std::string(input) + endFlagHex, &bytes);
                if (bytes)
                {
                    if (showTime)
                    {
                        fprintf(stderr, "[Tx][%s] %s\n", getDateTime().c_str(), input);
                    }
                    g_com.write(bytes, len);
                    free(bytes);
                }
            }
            else
            {
                if (showTime)
                {
                    fprintf(stderr, "[Tx][%s] %s\n", getDateTime().c_str(), input);
                }
                g_com.write(std::string(input) + endFlag);
            }
        }
    });
    th.detach();
    /* 监听串口数据, 坑: 这里打印只能输出到stderr, 用stdout的话会阻塞(不知道啥原因) */
    bool newLine = true;
    while (g_com.isOpened())
    {
        auto bytes = g_com.readAll();
        if (hideRecv)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        if (!bytes.empty())
        {
            /* 打印接收时间 */
            if (autoLine || newLine)
            {
                newLine = false;
                if (showTime)
                {
                    fprintf(stderr, "[Rx][%s] ", getDateTime().c_str());
                }
            }
            /* 打印串口数据 */
            for (size_t i = 0, len = bytes.size(); i < len; ++i)
            {
                if (newLine) /* 打印接收时间 */
                {
                    newLine = false;
                    if (showTime)
                    {
                        fprintf(stderr, "[Rx][%s] ", getDateTime().c_str());
                    }
                }
                if (showHex)
                {
                    fprintf(stderr, "%02X", bytes[i]);
                    if (i < len - 1)
                    {
                        fprintf(stderr, " ");
                    }
                }
                else
                {
                    if ('\n' == bytes[i])
                    {
                        newLine = true;
                    }
                    else if ('\r' == bytes[i])
                    {
                        newLine = true;
                        if (i < len - 1 && ('\n' == bytes[i + 1]))
                        {
                            ++i;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "%c", bytes[i]);
                    }
                }
                if (newLine) /* 打印换行符 */
                {
                    fprintf(stderr, "\n");
                }
            }
            /* 打印后续格式 */
            if (autoLine)
            {
                if (!newLine)
                {
                    fprintf(stderr, "\n");
                }
            }
            else
            {
                if (showHex)
                {
                    fprintf(stderr, " ");
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    fprintf(stderr, "串口被关闭.\n");
}

bool isOptionName(const std::string& str)
{
    if ("-port" == str || "-baud" == str || "-data" == str || "-parity" == str || "-stop" == str || "-flow" == str || "-crlf" == str
        || "-list" == str || "--time" == str || "--txhex" == str || "--rxhex" == str || "--rxline" == str || "--rxhide" == str)
    {
        return true;
    }
    return false;
}

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    printf("*************************************************************************************************************\n");
    printf("** 说明: 串口调试工具, 支持: 收/发数据, ASCII/十六进制. 打开成功后在控制台输入字符并回车即可发送.          **\n");
    printf("**                                                                                                         **\n");
    printf("** 选项:                                                                                                   **\n");
    printf("**                                                                                                         **\n");
#ifdef _WIN32
    printf("** [-port 串口号]      串口号(必填), 例如: COM1, COM2, COM3等.                                             **\n");
#else
    printf("** [-port 串口号]      串口号(必填), 例如: /dev/ttyS0, /dev/ttyUSB0等.                                     **\n");
#endif
    printf("** [-baud 波特率]      波特率(选填), 例如: 9600, 19200, 38400, 115200等, 默认: 115200.                     **\n");
    printf("** [-data 数据位]      数据位(选填), 值: [5, 6, 7, 8], 默认: 8.                                            **\n");
    printf("** [-parity 校验位]    校验位(选填), 值: [None: N|n, Even: E|e, Odd: O|o, Mark: M|m, Space: S|s], 默认: N. **\n");
    printf("** [-stop 停止位]      停止位(选填), 值: [1, 1.5, 2], 默认: 1.                                             **\n");
    printf("** [-flow 流控]        流控(选填), 值: [None: N|n, Software: S|s, Hardware: H|h], 默认: N.                 **\n");
    printf("** [-crlf 结束符]      发送结束符(选填), 值: [0: 无, 1: CR(回车), 2: LF(换行), 3: CRLF(回车换行)], 默认: 0.**\n");
    printf("** [--list]            显示所有串口, 默认: 不显示.                                                         **\n");
    printf("** [--time]            显示发送/接收数据时间(选填), 默认: 不显示.                                          **\n");
    printf("** [--txhex]           使用十六进制格式发送数据(选填), 默认: ASCII.                                        **\n");
    printf("** [--rxhex]           使用十六进制格式显示接收数据(选填), 默认: ASCII.                                    **\n");
    printf("** [--rxline]          自动换行接收数据(选填), 默认: 不自动换行.                                           **\n");
    printf("** [--rxhide]          不显示接收到的数据(选填), 默认: 显示.                                               **\n");
    printf("**                                                                                                         **\n");
    printf("** 示例:                                                                                                   **\n");
#ifdef _WIN32
    printf("**       serial_tool.exe -baud 115200 -data 8 -parity N -stop 1 -flow N --rxhex -port COM1                 **\n");
#else
    printf("**       ./serial_tool -baud 115200 -data 8 -parity N -stop 1 -flow N --rxhex -port /dev/ttyUSB0           **\n");
#endif
    printf("**                                                                                                         **\n");
    printf("*************************************************************************************************************\n");
    printf("\n");
    /* 参数标识 */
    bool flagPort = false;
    bool flagList = false;
    bool flagTime = false;
    bool flagTxHex = false;
    bool flagRxHex = false;
    bool flagRxLine = false;
    bool flagRxHide = false;
    /* 参数值 */
    std::string portName;
    unsigned long baudrate = 115200;
    serial::Databits databits = serial::Databits::eight;
    serial::ParityType pariry = serial::ParityType::none;
    serial::Stopbits stopbits = serial::Stopbits::one;
    serial::FlowcontrolType flowcontrol = serial::FlowcontrolType::none;
    int crlf = 0;
    /* 解析参数 */
    auto portList = serial::Serial::getAllPorts();
    std::string errorStr;
    for (int i = 1; i < argc; ++i)
    {
        std::string key = argv[i];
        if (0 == key.compare("-port")) /* 串口 */
        {
            flagPort = true;
            ++i;
            if (i >= argc)
            {
                errorStr += "错误: 串口 -port 缺少参数\n";
            }
            else
            {
                std::string val(argv[i]);
                if (isOptionName(val))
                {
                    --i;
                    errorStr += "错误: 串口 -port 缺少参数\n";
                }
                else
                {
                    for (size_t i = 0; i < portList.size(); ++i)
                    {
                        if (0 == val.compare(portList[i].port))
                        {
                            portName = val;
                            break;
                        }
                    }
                    if (portName.empty())
                    {
                        errorStr += "错误: 串口 " + val + " 不存在\n";
                    }
                }
            }
        }
        else if (0 == key.compare("-baud")) /* 波特率 */
        {
            ++i;
            if (i >= argc)
            {
                errorStr += "错误: 波特率 -baud 缺少参数\n";
            }
            else
            {
                std::string val(argv[i]);
                if (isOptionName(val))
                {
                    --i;
                    errorStr += "错误: 波特率 -baud 缺少参数\n";
                }
                else
                {
                    baudrate = std::atoi(argv[i]);
                }
            }
        }
        else if (0 == key.compare("-data")) /* 数据位 */
        {
            ++i;
            if (i >= argc)
            {
                errorStr += "错误: 数据位 -data 缺少参数\n";
            }
            else
            {
                std::string val(argv[i]);
                if (isOptionName(val))
                {
                    --i;
                    errorStr += "错误: 数据位 -data 缺少参数\n";
                }
                else if (0 == val.compare("5"))
                {
                    databits = serial::Databits::five;
                }
                else if (0 == val.compare("6"))
                {
                    databits = serial::Databits::six;
                }
                else if (0 == val.compare("7"))
                {
                    databits = serial::Databits::seven;
                }
                else if (0 == val.compare("8"))
                {
                    databits = serial::Databits::eight;
                }
                else
                {
                    errorStr += "错误: 数据位 " + val + " 无效.\n";
                }
            }
        }
        else if (0 == key.compare("-parity")) /* 校验位 */
        {
            ++i;
            if (i >= argc)
            {
                errorStr += "错误: 校验位 -parity 缺少参数\n";
            }
            else
            {
                std::string val(argv[i]);
                if (isOptionName(val))
                {
                    --i;
                    errorStr += "错误: 校验位 -parity 缺少参数\n";
                }
                else if (0 == val.compare("N") || 0 == val.compare("n"))
                {
                    pariry = serial::ParityType::none;
                }
                else if (0 == val.compare("O") || 0 == val.compare("o"))
                {
                    pariry = serial::ParityType::odd;
                }
                else if (0 == val.compare("E") || 0 == val.compare("e"))
                {
                    pariry = serial::ParityType::even;
                }
                else if (0 == val.compare("M") || 0 == val.compare("m"))
                {
                    pariry = serial::ParityType::mark;
                }
                else if (0 == val.compare("S") || 0 == val.compare("s"))
                {
                    pariry = serial::ParityType::space;
                }
                else
                {
                    errorStr += "错误: 校验位 " + val + " 无效.\n";
                }
            }
        }
        else if (0 == key.compare("-stop")) /* 停止位 */
        {
            ++i;
            if (i >= argc)
            {
                errorStr += "错误: 停止位 -stop 缺少参数\n";
            }
            else
            {
                std::string val(argv[i]);
                if (isOptionName(val))
                {
                    --i;
                    errorStr += "错误: 停止位 -stop 缺少参数\n";
                }
                else if (0 == val.compare("1"))
                {
                    stopbits = serial::Stopbits::one;
                }
                else if (0 == val.compare("1.5"))
                {
                    stopbits = serial::Stopbits::one_and_half;
                }
                else if (0 == val.compare("2"))
                {
                    stopbits = serial::Stopbits::two;
                }
                else
                {
                    errorStr += "错误: 停止位 " + val + " 无效.\n";
                }
            }
        }
        else if (0 == key.compare("-flow")) /* 流控 */
        {
            ++i;
            if (i >= argc)
            {
                errorStr += "错误: 流控 -flow 缺少参数\n";
            }
            else
            {
                std::string val(argv[i]);
                if (isOptionName(val))
                {
                    --i;
                    errorStr += "错误: 流控 -flow 缺少参数\n";
                }
                else if (0 == val.compare("N") || 0 == val.compare("n"))
                {
                    flowcontrol = serial::FlowcontrolType::none;
                }
                else if (0 == val.compare("S") || 0 == val.compare("s"))
                {
                    flowcontrol = serial::FlowcontrolType::software;
                }
                else if (0 == val.compare("H") || 0 == val.compare("h"))
                {
                    flowcontrol = serial::FlowcontrolType::hardware;
                }
                else
                {
                    errorStr += "错误: 流控 " + val + " 无效.\n";
                }
            }
        }
        else if (0 == key.compare("-crlf")) /* 数据结束符 */
        {
            ++i;
            if (i >= argc)
            {
                errorStr += "错误: 结束符 -crlf 缺少参数\n";
            }
            else
            {
                std::string val(argv[i]);
                if (isOptionName(val))
                {
                    --i;
                    errorStr += "错误: 结束符 -crlf 缺少参数\n";
                }
                else if (0 == val.compare("0") || 0 == val.compare("1") || 0 == val.compare("2") || 0 == val.compare("3"))
                {
                    crlf = std::atoi(val.c_str());
                }
                else
                {
                    errorStr += "错误: 结束符 " + val + " 无效.\n";
                }
            }
        }
        else if (0 == key.compare("--list")) /* 显示列表 */
        {
            flagList = true;
            showAllPorts(portList);
        }
        else if (0 == key.compare("--time")) /* 发送/接收显示时间 */
        {
            flagTime = true;
        }
        else if (0 == key.compare("--txhex")) /* 发送十六进制 */
        {
            flagTxHex = true;
        }
        else if (0 == key.compare("--rxhex")) /* 接收显示十六进制 */
        {
            flagRxHex = true;
        }
        else if (0 == key.compare("--rxline")) /* 接收显示自动换行 */
        {
            flagRxLine = true;
        }
        else if (0 == key.compare("--rxhide")) /* 是否显示接收 */
        {
            flagRxHide = true;
        }
        else
        {
            errorStr += "错误: 不支持 " + key + " 选项\n";
        }
    }
    /* 测试数据 */
#if 0
    flagPort = true;
    portName = "COM207";
    crlf = 1;
    flagTime = false;
    flagTxHex = false;
    flagRxHex = false;
    flagRxLine = false;
    flagRxHide = false;
#endif
    /* 参数判断 */
    if (flagList && errorStr.empty())
    {
        return 0;
    }
    if (!flagPort)
    {
        errorStr += "错误: 未指定串口\n";
    }
    if (!errorStr.empty())
    {
        printf("%s", errorStr.c_str());
        return 0;
    }
    /* 打开串口 */
    openSerial(portName, baudrate, databits, pariry, stopbits, flowcontrol, crlf, flagTime, flagTxHex, flagRxHex, flagRxLine, flagRxHide);
    return 0;
}
