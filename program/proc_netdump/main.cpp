#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "filter.h"
#include "ftp_handler.h"
#include "iec103_handler.h"
#include "npacket/analyzer.h"
#include "npacket/pcap_device.h"
#include "npacket/proto/ftp.h"
#include "npacket/proto/iec103.h"
#include "print.h"
#include "utility/cmdline/cmdline.h"
#include "utility/strtool/strtool.h"

static npacket::Analyzer s_pktAnalyzer; /* 包分析器 */

/**
 * @brief 处理以太网层
 */
bool handleEthernetLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                         const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
{
    auto h = std::dynamic_pointer_cast<npacket::EthernetIIHeader>(header);
    if (Filter::getInstance().showEthernet())
    {
        printEthernet(h);
    }
    return true;
}

/**
 * @brief 处理网络层
 */
bool handleNetworkLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                        const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
{
    switch ((npacket::NetworkProtocol)header->getProtocol())
    {
    case npacket::NetworkProtocol::IPv4: {
        auto h = std::dynamic_pointer_cast<npacket::Ipv4Header>(header);
        if (Filter::getInstance().showIpv4())
        {
            if (!Filter::getInstance().showEthernet())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent));
            }
            printIPv4(h);
        }
    }
    break;
    case npacket::NetworkProtocol::ARP: {
        auto h = std::dynamic_pointer_cast<npacket::ArpHeader>(header);
        if (Filter::getInstance().showArp())
        {
            if (!Filter::getInstance().showEthernet())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent));
            }
            printARP(h);
        }
    }
    break;
    case npacket::NetworkProtocol::IPv6: {
        auto h = std::dynamic_pointer_cast<npacket::Ipv6Header>(header);
        if (Filter::getInstance().showIpv6())
        {
            if (!Filter::getInstance().showEthernet())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent));
            }
            printIPv6(h);
        }
    }
    break;
    }
    return true;
}

/**
 * @brief 处理传输层
 */
bool handleTransportLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                          const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
{
    switch ((npacket::TransportProtocol)header->getProtocol())
    {
    case npacket::TransportProtocol::TCP: {
        auto h = std::dynamic_pointer_cast<npacket::TcpHeader>(header);
        if (Filter::getInstance().showTcp())
        {
            if (!Filter::getInstance().showEthernet())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent->parent));
            }
            switch (h->parent->getProtocol())
            {
            case npacket::NetworkProtocol::IPv4:
                if (!Filter::getInstance().showIpv4())
                {
                    printIPv4(std::dynamic_pointer_cast<npacket::Ipv4Header>(h->parent));
                }
                break;
            case npacket::NetworkProtocol::IPv6:
                if (!Filter::getInstance().showIpv6())
                {
                    printIPv6(std::dynamic_pointer_cast<npacket::Ipv6Header>(h->parent));
                }
                break;
            }
            printTCP(h);
        }
    }
    break;
    case npacket::TransportProtocol::UDP: {
        auto h = std::dynamic_pointer_cast<npacket::UdpHeader>(header);
        if (Filter::getInstance().showUdp())
        {
            if (!Filter::getInstance().showEthernet())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent->parent));
            }
            switch (h->parent->getProtocol())
            {
            case npacket::NetworkProtocol::IPv4:
                if (!Filter::getInstance().showIpv4())
                {
                    printIPv4(std::dynamic_pointer_cast<npacket::Ipv4Header>(h->parent));
                }
                break;
            case npacket::NetworkProtocol::IPv6:
                if (!Filter::getInstance().showIpv6())
                {
                    printIPv6(std::dynamic_pointer_cast<npacket::Ipv6Header>(h->parent));
                }
                break;
            }
            printUDP(h);
        }
    }
    break;
    case npacket::TransportProtocol::ICMP: {
        auto h = std::dynamic_pointer_cast<npacket::IcmpHeader>(header);
        if (Filter::getInstance().showIcmp())
        {
            if (!Filter::getInstance().showEthernet())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent->parent));
            }
            if (npacket::NetworkProtocol::IPv4 == h->parent->getProtocol())
            {
                if (!Filter::getInstance().showIpv4())
                {
                    printIPv4(std::dynamic_pointer_cast<npacket::Ipv4Header>(h->parent));
                }
            }
            printICMP(h);
        }
    }
    break;
    case npacket::TransportProtocol::ICMPv6: {
        auto h = std::dynamic_pointer_cast<npacket::Icmpv6Header>(header);
        if (Filter::getInstance().showIcmpv6())
        {
            if (!Filter::getInstance().showEthernet())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent->parent));
            }
            if (npacket::NetworkProtocol::IPv6 == h->parent->getProtocol())
            {
                if (!Filter::getInstance().showIpv6())
                {
                    printIPv6(std::dynamic_pointer_cast<npacket::Ipv6Header>(h->parent));
                }
            }
            printICMPv6(h);
        }
    }
    break;
    }
    return true;
}

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    /* 关闭控制台程序的快速编辑模式, 否则会出现点击界面, 程序将会变成阻塞状态, 不按回车无法继续运行 */
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE; /* 移除快速编辑模式 */
    SetConsoleMode(hStdin, mode);
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("网络数据抓包工具.");
    parser.add("list", 'l', "显示所有网络设备.");
    parser.add<std::string>("interface", 'I', "指定要抓取数据的网络设备.", false);
    parser.add<std::string>("ip", 'i', "指定要抓取数据的IPv4地址.", false);
#ifndef _WIN32
    parser.add<std::string>("direction", 'd', "抓包流向, 值范围: [inout-所有, in-接收, out-发送], 默认:", false, "inout",
                            cmdline::oneof(std::string("inout"), std::string("in"), std::string("out")));
#endif
    auto protos = utility::StrTool::join(Filter::protoList(), ", ");
    parser.add<std::string>("express", 'e', "设置过滤表达式, 目前只支持单个协议过滤, 例如: " + protos, false, "");
    parser.parse_check(argc, argv, " 用法 ", " 选项 ", "显示帮助信息并退出.");
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    bool showList = parser.exist("list");
    auto iface = parser.get<std::string>("interface");
    auto ip = parser.get<std::string>("ip");
    int direction = 0;
#ifndef _WIN32
    auto directionDesc = utility::StrTool::toLower(parser.get<std::string>("direction"));
    if ("in" == directionDesc)
    {
        direction = 1;
    }
    else if ("out" == directionDesc)
    {
        direction = 2;
    }
#endif
    auto express = utility::StrTool::toLower(parser.get<std::string>("express"));
    auto devList = npacket::PcapDevice::getAllDevices();
    if (showList)
    {
        printf("==================== 当前识别到的所有网卡设备 ====================\n");
        for (size_t i = 0, devCount = devList.size(); i < devCount; ++i)
        {
            if (i > 0)
            {
                printf("----------------------------------------\n");
            }
            printf("[%02zu] 设备名: %s\n", (i + 1), devList[i]->getName().c_str());
            printf("     描  述: %s\n", devList[i]->getDescribe().c_str());
            printf("       IPv4: %s\n", devList[i]->getIpv4Address().c_str());
            if (i == devCount - 1)
            {
                printf("----------------------------------------\n");
            }
        }
        printf("\n");
    }
    if (iface.empty() && ip.empty())
    {
        if (!showList)
        {
            printf("未指定要抓包的设备名或IPv4地址\n");
        }
        return 0;
    }
    if (!iface.empty())
    {
        printf("设备名: %s\n", iface.c_str());
    }
    if (!ip.empty())
    {
        printf("  IPv4: %s\n", ip.c_str());
    }
    printf("包流向: %s\n", 1 == direction ? "in(接收)" : (2 == direction ? "out(发送)" : "inout(所有)"));
    Filter::getInstance().setCondition(express);
    s_pktAnalyzer.setLayerCallback(handleEthernetLayer, handleNetworkLayer, handleTransportLayer);
    {
        auto ftpParser = std::make_shared<npacket::FtpParser>();
        ftpParser->setRequestCallback(handleApplicationFtpCtrlReq);
        ftpParser->setResponseCallback(handleApplicationFtpCtrlResp);
        ftpParser->setDataCallback(handleApplicationFtpData);
        s_pktAnalyzer.addProtocolParser(ftpParser);
    }
    {
        auto iec103Parser = std::make_shared<npacket::Iec103Parser>();
        iec103Parser->setFixedFrameCallback(handleApplicationIec103FixedFrame);
        iec103Parser->setVariableFrameCallback(handleApplicationIec103VariableFrame);
        s_pktAnalyzer.addProtocolParser(iec103Parser);
    }
    std::shared_ptr<npacket::PcapDevice> dev;
    for (size_t i = 0; i < devList.size(); ++i)
    {
        if ((!iface.empty() && devList[i]->getName() == iface) || (!ip.empty() && devList[i]->getIpv4Address() == ip))
        {
            dev = devList[i];
            break;
        }
    }
    if (!dev || !dev->open(dev->getName(), direction, 0, 0 == direction ? 1 : 0))
    {
        printf("未找到可用设备\n");
        return 0;
    }
    if (iface.empty())
    {
        printf("设备名: %s\n", dev->getName().c_str());
    }
    if (ip.empty())
    {
        printf("  IPv4: %s\n", dev->getIpv4Address().c_str());
    }
    printf("描  述: %s\n", dev->getDescribe().c_str());
    printf("开始抓包 ...\n");
    printf("\n");
    dev->setDataCallback([&](const unsigned char* data, int dataLen) { s_pktAnalyzer.parse(data, dataLen); });
    dev->startCapture();
    while (1)
    {
        dev->captureOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    dev->close();
    return 0;
}
