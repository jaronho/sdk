#include <chrono>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <thread>

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
    printf("==================== list all serial port ====================\n");
    for (size_t i = 0, portCount = portList.size(); i < portCount; ++i)
    {
        if (i > 0)
        {
            printf("----------------------------------------\n");
        }
        if (portCount < 10)
        {
            printf("[%zu]        Name: %s\n", (i + 1), portList[i].port.c_str());
        }
        else if (portCount < 100)
        {
            printf("[%02zu]       Name: %s\n", (i + 1), portList[i].port.c_str());
        }
        else
        {
            printf("[%03zu]      Name: %s\n", (i + 1), portList[i].port.c_str());
        }
        printf("    Description: %s\n", portList[i].description.c_str());
        printf("    Hardware ID: %s\n", portList[i].hardwareId.c_str());
        if (i == portCount - 1)
        {
            printf("----------------------------------------\n");
        }
    }
}

/**
 * @brief 打开串口 
 */
void openSerial(const std::string& port, unsigned long baudrate, const serial::Databits& databits, const serial::ParityType& parity,
                const serial::Stopbits& stopbits, const serial::FlowcontrolType& flowcontrol, bool sendHex, bool showHex, bool autoLine)
{
    /* 串口设置及打开 */
    g_com.setPort(port);
    g_com.setBaudrate(baudrate);
    g_com.setDatabits(databits);
    g_com.setParity(parity);
    g_com.setStopbits(stopbits);
    g_com.setFlowcontrol(flowcontrol);
    if (!g_com.open())
    {
        printf("serial open failed!\n");
        return;
    }
    printf("serial open success.\n");
    /* 开线程用于发送 */
    std::thread th([&]() {
        while (1)
        {
            char str[1024] = {0};
            std::cin.getline(str, sizeof(str));
            fprintf(stderr, "=====%s=====\n", str);
            if (sendHex)
            {
                char* bytes;
                int len = hexStrToBytes(str, &bytes);
                if (bytes)
                {
                    g_com.write(bytes, len);
                    free(bytes);
                }
            }
            else
            {
                g_com.write(str, strlen(str));
            }
        }
    });
    th.detach();
    /* 监听串口数据, 坑爹: 这里打印只能输出到stderr, 用stdout的话会阻塞(不知道啥原因) */
    while (1)
    {
        std::string bytes = g_com.readAll();
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
                fprintf(stderr, "\n========== total receive length: %zu (byte)\n", g_totalRecvLength);
                g_totalRecvLength = 0;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main(int argc, char** argv)
{
    printf("***********************************************************************************************************\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   show all serial port.                                                          **\n");
    printf("** [-port name]           specify the name of the port to open, e.g. COM1, /dev/ttyS0, /dev/ttyUSB0.     **\n");
    printf("** [-baud baudrate]       specify the baudrate.                                                          **\n");
    printf("** [-data databits]       specify the data bits, [ 5, 6, 7, 8 ].                                         **\n");
    printf("** [-parity parity]       specify the parity, [ None: N|n, Even: E|e, Odd: O|o, Mark: M|m, Space: S|s ]. **\n");
    printf("** [-stop stopbits]       specify the stop bits, [ 1, 1.5, 2 ].                                          **\n");
    printf("** [-flow flowcontrol]    specify the flow control, [ None: N|n, Software: S|s, Hardware: H|h ].         **\n");
    printf("** [-txhex]               specify the send format with hex.                                              **\n");
    printf("** [-rxhex]               specify the receive show with hex.                                             **\n");
    printf("** [-rxline]              specify the receive auto new line.                                             **\n");
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    /* 参数标识: 0-没有找到, 1-值错误, 2-值正确 */
    int flagShow = 0;
    int flagPortName = 0;
    int flagBaurate = 2;
    int flagDatabits = 2;
    int flagParity = 2;
    int flagStopbits = 2;
    int flagFlowcontrol = 2;
    int flagTxHex = 0;
    int flagRxHex = 0;
    int flagRxAutoLine = 0;
    /* 错误的参数值 */
    std::string valDatabits;
    std::string valParity;
    std::string valStopbits;
    std::string valFlowcontrol;
    /* 参数值 */
    std::string portName;
    unsigned long baudrate = 115200;
    serial::Databits databits = serial::Databits::EIGHT;
    serial::ParityType pariry = serial::ParityType::EVEN;
    serial::Stopbits stopbits = serial::Stopbits::ONE;
    serial::FlowcontrolType flowcontrol = serial::FlowcontrolType::NONE;
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
        else if (0 == key.compare("-txhex")) /* 发送十六进制 */
        {
            flagTxHex = 2;
            i += 1;
            continue;
        }
        else if (0 == key.compare("-rxhex")) /* 接收显示十六进制 */
        {
            flagRxHex = 2;
            i += 1;
            continue;
        }
        else if (0 == key.compare("-rxline")) /* 接收显示自动换行 */
        {
            flagRxAutoLine = 2;
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
                databits = serial::Databits::FIVE;
            }
            else if (0 == val.compare("6"))
            {
                databits = serial::Databits::SIX;
            }
            else if (0 == val.compare("7"))
            {
                databits = serial::Databits::SEVEN;
            }
            else if (0 == val.compare("8"))
            {
                databits = serial::Databits::EIGHT;
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
                pariry = serial::ParityType::NONE;
            }
            else if (0 == val.compare("O") || 0 == val.compare("o"))
            {
                pariry = serial::ParityType::ODD;
            }
            else if (0 == val.compare("E") || 0 == val.compare("e"))
            {
                pariry = serial::ParityType::EVEN;
            }
            else if (0 == val.compare("M") || 0 == val.compare("m"))
            {
                pariry = serial::ParityType::MARK;
            }
            else if (0 == val.compare("S") || 0 == val.compare("s"))
            {
                pariry = serial::ParityType::SPACE;
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
                stopbits = serial::Stopbits::ONE;
            }
            else if (0 == val.compare("1.5"))
            {
                stopbits = serial::Stopbits::ONE_AND_HALF;
            }
            else if (0 == val.compare("2"))
            {
                stopbits = serial::Stopbits::TWO;
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
                flowcontrol = serial::FlowcontrolType::NONE;
            }
            else if (0 == val.compare("S") || 0 == val.compare("s"))
            {
                flowcontrol = serial::FlowcontrolType::SOFTWARE;
            }
            else if (0 == val.compare("H") || 0 == val.compare("h"))
            {
                flowcontrol = serial::FlowcontrolType::HARDWARE;
            }
            else
            {
                flagFlowcontrol = 1;
                valFlowcontrol = val;
            }
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
    portName = "COM26";
    baudrate = 38400;
    databits = serial::Databits::SEVEN;
    pariry = serial::ParityType::EVEN;
    stopbits = serial::Stopbits::TWO;
    flowcontrol = serial::FlowcontrolType::NONE;
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
            printf("Error: missing -port option.\n");
            return 0;
        }
    }
    else if (1 == flagPortName)
    {
        printf("Error: port '%s' is invalid.\n", portName.c_str());
        return 0;
    }
    std::string errorStr;
    /* 判断波特率 */
    if (0 == flagBaurate)
    {
        errorStr += "Error: missing -baud option.\n";
    }
    /* 判断数据位 */
    if (0 == flagDatabits)
    {
        errorStr += "Error: missing -data option.\n";
    }
    else if (1 == flagDatabits)
    {
        errorStr += "Error: data bits '" + valDatabits + "' is invalid.\n";
    }
    /* 判断校验位 */
    if (0 == flagParity)
    {
        errorStr += "Error: missing -parity option.\n";
    }
    else if (1 == flagParity)
    {
        errorStr += "Error: parity '" + valParity + "' is invalid.\n";
    }
    /* 判断停止位 */
    if (0 == flagStopbits)
    {
        errorStr += "Error: missing -stop option.\n";
    }
    else if (1 == flagStopbits)
    {
        errorStr += "Error: stop bits '" + valStopbits + "' is invalid.\n";
    }
    /* 判断流控 */
    if (0 == flagFlowcontrol)
    {
        errorStr += "Error: missing -flow option.\n";
    }
    else if (1 == flagFlowcontrol)
    {
        errorStr += "Error: flow control '" + valFlowcontrol + "' is invalid.\n";
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
    openSerial(portName, baudrate, databits, pariry, stopbits, flowcontrol, 2 == flagTxHex, 2 == flagRxHex, 2 == flagRxAutoLine);

    return 0;
}
