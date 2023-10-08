#include <chrono>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "../serial/serial.h"

serial::Serial g_com; /* 串口对象 */
std::chrono::steady_clock::time_point g_lastRecvTimestamp; /* 上次接收的时间戳 */
size_t g_totalRecvLength = 0; /* 总的接收长度(字节) */

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
                const serial::Stopbits& stopbits, const serial::FlowcontrolType& flowcontrol, int crlf, bool sendHex, bool showHex,
                bool autoLine, bool hideRecv)
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
            if (sendHex)
            {
                char* bytes;
                int len = hexStrToBytes(std::string(input) + endFlagHex, &bytes);
                if (bytes)
                {
                    g_com.write(bytes, len);
                    free(bytes);
                }
            }
            else
            {
                g_com.write(std::string(input) + endFlag);
            }
        }
    });
    th.detach();
    /* 监听串口数据, 坑爹: 这里打印只能输出到stderr, 用stdout的话会阻塞(不知道啥原因) */
    while (g_com.isOpened())
    {
        std::string bytes = g_com.readAll();
        if (!hideRecv)
        {
            if (!bytes.empty())
            {
                g_lastRecvTimestamp = std::chrono::steady_clock::now();
                if (0 == g_totalRecvLength)
                {
                    fprintf(stderr, "==================================================\n");
                }
                else if (autoLine)
                {
                    fprintf(stderr, "\n");
                }
                if (showHex)
                {
                    for (size_t i = 0, cnt = bytes.size(); i < cnt; ++i)
                    {
                        fprintf(stderr, "%02X", bytes[i]);
                        if (i < cnt - 1)
                        {
                            fprintf(stderr, " ");
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "%s", bytes.c_str());
                }
                g_totalRecvLength += bytes.size();
            }
            if (g_totalRecvLength > 0)
            {
                std::chrono::milliseconds elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_lastRecvTimestamp);
                if (elapsed.count() >= 400)
                {
                    fprintf(stderr, "\n========== 总的接收长度: %zu (字节)\n", g_totalRecvLength);
                    g_totalRecvLength = 0;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    fprintf(stderr, "串口被关闭.\n");
}

int main(int argc, char** argv)
{
    printf("*************************************************************************************************************\n");
    printf("** 说明: 串口调试工具, 支持: 收/发数据, ASCII/十六进制. 打开成功后在控制台输入字符并回车即可发送.          **\n");
    printf("**                                                                                                         **\n");
    printf("** 选项:                                                                                                   **\n");
    printf("**                                                                                                         **\n");
    printf("** [-s]                显示所有串口端口.                                                                   **\n");
#ifdef _WIN32
    printf("** [-port 端口号]      串口端口号(必填), 例如: COM1, COM2, COM3等.                                         **\n");
#else
    printf("** [-port 端口号]      串口端口号(必填), 例如: /dev/ttyS0, /dev/ttyUSB0等.                                 **\n");
#endif
    printf("** [-baud 波特率]      波特率(选填), 例如: 9600, 19200, 38400, 115200等, 默认: 115200.                     **\n");
    printf("** [-data 数据位]      数据位(选填), 值: [5, 6, 7, 8], 默认: 8.                                            **\n");
    printf("** [-parity 校验位]    校验位(选填), 值: [None: N|n, Even: E|e, Odd: O|o, Mark: M|m, Space: S|s], 默认: N. **\n");
    printf("** [-stop 停止位]      停止位(选填), 值: [1, 1.5, 2], 默认: 1.                                             **\n");
    printf("** [-flow 流控]        流控(选填), 值: [None: N|n, Software: S|s, Hardware: H|h], 默认: N.                 **\n");
    printf("** [-crlf 结束符]      发送结束符(选填), 值: [0: 无, 1: CR(回车), 2: LF(换行), 3: CRLF(回车换行)], 默认: 0.**\n");
    printf("** [--txhex]           使用十六进制格式发送数据(选填), 默认: ASCII.                                        **\n");
    printf("** [--rxhex]           使用十六进制格式显示接收数据(选填), 默认: ASCII.                                    **\n");
    printf("** [--rxline]          自动换行接收数据(选填), 默认: 不自动换行.                                           **\n");
    printf("** [--rxhide]          隐藏接收到的数据(选填), 默认: 显示.                                                 **\n");
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
    /* 参数标识: 0-没有找到, 1-值错误, 2-值正确 */
    int flagShow = 0;
    int flagPortName = 0;
    int flagBaurate = 2;
    int flagDatabits = 2;
    int flagParity = 2;
    int flagStopbits = 2;
    int flagFlowcontrol = 2;
    int flagCRLF = 2;
    int flagTxHex = 0;
    int flagRxHex = 0;
    int flagRxAutoLine = 0;
    int flagRxHide = 0;
    /* 错误的参数值 */
    std::string valDatabits;
    std::string valParity;
    std::string valStopbits;
    std::string valFlowcontrol;
    /* 参数值 */
    std::string portName;
    unsigned long baudrate = 115200;
    serial::Databits databits = serial::Databits::eight;
    serial::ParityType pariry = serial::ParityType::none;
    serial::Stopbits stopbits = serial::Stopbits::one;
    serial::FlowcontrolType flowcontrol = serial::FlowcontrolType::none;
    int crlf = 0;
    /* 解析参数 */
    for (int i = 1; i < argc;)
    {
        std::string key = argv[i];
        if (0 == key.compare("-s")) /* 显示 */
        {
            flagShow = 2;
            i += 1;
            continue;
        }
        else if (0 == key.compare("--txhex")) /* 发送十六进制 */
        {
            flagTxHex = 2;
            i += 1;
            continue;
        }
        else if (0 == key.compare("--rxhex")) /* 接收显示十六进制 */
        {
            flagRxHex = 2;
            i += 1;
            continue;
        }
        else if (0 == key.compare("--rxline")) /* 接收显示自动换行 */
        {
            flagRxAutoLine = 2;
            i += 1;
            continue;
        }
        else if (0 == key.compare("--rxhide")) /* 是否显示接收 */
        {
            flagRxHide = 2;
            i += 1;
            continue;
        }
        if (i + 1 >= argc)
        {
            break;
        }
        std::string val = argv[i + 1];
        if (0 == key.compare("-port")) /* 端口 */
        {
            flagPortName = 1;
            portName = val;
        }
        else if (0 == key.compare("-baud")) /* 波特率 */
        {
            flagBaurate = 2;
            baudrate = stoul(val);
        }
        else if (0 == key.compare("-data")) /* 数据位 */
        {
            flagDatabits = 2;
            if (0 == val.compare("5"))
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
                flagDatabits = 1;
                valDatabits = val;
            }
        }
        else if (0 == key.compare("-parity")) /* 校验位 */
        {
            flagParity = 2;
            if (0 == val.compare("N") || 0 == val.compare("n"))
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
                flagParity = 1;
                valParity = val;
            }
        }
        else if (0 == key.compare("-stop")) /* 停止位 */
        {
            flagStopbits = 2;
            if (0 == val.compare("1"))
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
                flagStopbits = 1;
                valStopbits = val;
            }
        }
        else if (0 == key.compare("-flow")) /* 流控 */
        {
            flagFlowcontrol = 2;
            if (0 == val.compare("N") || 0 == val.compare("n"))
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
                flagFlowcontrol = 1;
                valFlowcontrol = val;
            }
        }
        else if (0 == key.compare("-crlf")) /* 数据结束符 */
        {
            flagCRLF = 2;
            crlf = stoul(val);
        }
        i += 2;
    }
    /* 判断是否显示 */
    auto portList = serial::Serial::getAllPorts();
    if (2 == flagShow)
    {
        showAllPorts(portList);
    }
    /* 临时调试数据 */
#if 0
    flagBaurate = 2;
    flagDatabits = 2;
    flagParity = 2;
    flagStopbits = 2;
    flagFlowcontrol = 2;
    flagTxHex = 0;
    flagRxHex = 0;
    flagRxAutoLine = 0;
    flagRxHide = 0;
    portName = "COM27";
    baudrate = 115200;
    databits = serial::Databits::eight;
    pariry = serial::ParityType::none;
    stopbits = serial::Stopbits::one;
    flowcontrol = serial::FlowcontrolType::none;
#endif
    /* 判断端口是否正确 */
    for (size_t i = 0; i < portList.size(); ++i)
    {
        if (0 == portName.compare(portList[i].port))
        {
            flagPortName = 2;
            break;
        }
    }
    if (0 == flagPortName)
    {
        if (0 == flagShow)
        {
            printf("错误: 缺少端口选项 -port\n");
            return 0;
        }
    }
    else if (1 == flagPortName)
    {
        printf("错误: 端口 '%s' 无效.\n", portName.c_str());
        return 0;
    }
    std::string errorStr;
    /* 判断波特率 */
    if (0 == flagBaurate)
    {
        errorStr += "错误: 缺少波特率选项 -baud\n";
    }
    /* 判断数据位 */
    if (0 == flagDatabits)
    {
        errorStr += "错误: 缺少数据位选项 -data\n";
    }
    else if (1 == flagDatabits)
    {
        errorStr += "错误: 数据位 '" + valDatabits + "' 无效.\n";
    }
    /* 判断校验位 */
    if (0 == flagParity)
    {
        errorStr += "错误: 缺少校验位选项 -parity\n";
    }
    else if (1 == flagParity)
    {
        errorStr += "错误: 校验位 '" + valParity + "' 无效.\n";
    }
    /* 判断停止位 */
    if (0 == flagStopbits)
    {
        errorStr += "错误: 缺少停止位选项 -stop\n";
    }
    else if (1 == flagStopbits)
    {
        errorStr += "错误: 停止位 '" + valStopbits + "' 无效.\n";
    }
    /* 判断流控 */
    if (0 == flagFlowcontrol)
    {
        errorStr += "错误: 缺少流控选项 -flow\n";
    }
    else if (1 == flagFlowcontrol)
    {
        errorStr += "错误: 流控 '" + valFlowcontrol + "' 无效.\n";
    }
    /* 参数判断 */
    if (!errorStr.empty())
    {
        if (0 == flagShow)
        {
            printf("%s\n", errorStr.c_str());
        }
        return 0;
    }
    /* 打开串口 */
    openSerial(portName, baudrate, databits, pariry, stopbits, flowcontrol, crlf, 2 == flagTxHex, 2 == flagRxHex, 2 == flagRxAutoLine,
               2 == flagRxHide);
    return 0;
}
