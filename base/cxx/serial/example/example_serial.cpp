#include <chrono>
#include <fstream>
#include <iostream>
#include <string.h>
#include <thread>

#include "../serial/serial.h"

serial::Serial g_com;
std::chrono::steady_clock::time_point g_lastRecvTimestamp;
size_t g_totalRecvLength = 0;

void openSerial(const std::string& port, unsigned long baudrate, const serial::Databits& databits, const serial::ParityType& parity,
                const serial::Stopbits& stopbits, const serial::FlowcontrolType& flowcontrol)
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
    printf("serial open success\n");
    /* 监听串口数据 */
    const size_t BUF_SIZE = 1024;
    char buf[BUF_SIZE] = {0};
    while (1)
    {
        size_t canReadCount = g_com.availableForRead();
        while (canReadCount > 0)
        {
            size_t willReadCount = BUF_SIZE;
            if (canReadCount < willReadCount)
            {
                willReadCount = canReadCount;
            }
            size_t len = g_com.read(buf, willReadCount);
            if (len > 0)
            {
                fprintf(stderr, "%s", buf);
                g_lastRecvTimestamp = std::chrono::steady_clock::now();
                g_totalRecvLength += len;
            }
            canReadCount -= len;
        }
        if (g_totalRecvLength > 0)
        {
            auto nowTimestamp = std::chrono::steady_clock::now();
            std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(nowTimestamp - g_lastRecvTimestamp);
            if (elapsed.count() >= 1000)
            {
                fprintf(stderr, "\n========== total receive length: %zu\n", g_totalRecvLength);
                g_totalRecvLength = 0;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main(int argc, char** argv)
{
    printf("Options:\n");
    printf("\n");
    printf("[-s]                   show all serial port.\n");
    printf("\n");
    printf("[-port name]           specify the name of the port to open.\n");
    printf("[-baud baudrate]       specify the serial baudrate.\n");
    printf("[-data databits]       specify the serial data bits, [5, 6, 7, 8].\n");
    printf("[-parity parity]       specify the serial parity, [None: N|n, Even: E|e, Odd: O|o, Mark: M|m, Space: S|s].\n");
    printf("[-stop stopbits]       specify the serial stop bits, [1, 1.5, 2].\n");
    printf("[-flow flowcontrol]    specify the flow control, [None: N|n, Software: S|s, Hardware: H|h].\n");
    printf("\n");
    printf("\n");
    /* 获取当前所有端口 */
    auto portList = serial::Serial::getAllPorts();
    if (2 == argc && 0 == strcmp("-s", argv[1])) /* 打印端口列表 */
    {
        printf("========== list all serial port:\n");
        for (size_t i = 0; i < portList.size(); ++i)
        {
            printf("[%zu]\n", (i + 1));
            printf("           name: %s\n", portList[i].port.c_str());
            printf("    description: %s\n", portList[i].description.c_str());
            printf("    hardware ID: %s\n", portList[i].hardwareId.c_str());
        }
    }
    else if (argc >= 13)
    {
        /* 参数标识: 0-没有找到, 1-值错误, 2-值正确 */
        int flagPortName = 0;
        int flagBaurate = 0;
        int flagDatabits = 0;
        int flagParity = 0;
        int flagStopbits = 0;
        int flagFlowcontrol = 0;
        /* 错误的参数值 */
        std::string valDatabits;
        std::string valParity;
        std::string valStopbits;
        std::string valFlowcontrol;
        /* 参数值 */
        std::string portName;
        unsigned long baudrate;
        serial::Databits databits;
        serial::ParityType pariry;
        serial::Stopbits stopbits;
        serial::FlowcontrolType flowcontrol;
        /* 解析参数 */
        for (int i = 1; i < 13; i += 2)
        {
            std::string key = argv[i];
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
        }
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
            printf("Error: missing -port option.\n");
            return 0;
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
            printf("%s\n", errorStr.c_str());
            return 0;
        }
        /* 打开串口 */
        openSerial(portName, baudrate, databits, pariry, stopbits, flowcontrol);
    }
    return 0;
}
