#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "../npacket/analyzer.h"
#include "../npacket/pcap_device.h"
#include "../npacket/proto/ftp.h"

static npacket::Analyzer s_pktAnalyzer;
static std::string s_proto;

void printEthernet(const std::shared_ptr<npacket::EthernetIIHeader>& h)
{
    printf("=============== EthernetII ===============\n");
    printf("src mac: %s, dst mac: %s\n", h->srcMacStr().c_str(), h->dstMacStr().c_str());
    printf("type: 0x%04x\n", h->nextProtocol);
}

void printIPv4(const std::shared_ptr<npacket::Ipv4Header>& h)
{
    printf("    ----- IPv4 -----\n");
    printf("    version: %d, header len: %d, tos: %d, total len: %d\n", h->version, h->headerLen, h->tos, h->totalLen);
    printf("    identification: 0x%04x (%d)\n", h->identification, h->identification);
    printf("    reserved: %d, dont: %d, more: %d\n", h->flagRsrvd, h->flagDont, h->flagMore);
    printf("    frag offset: %d\n", h->fragOffset);
    printf("    ttl: %d\n", h->ttl);
    printf("    protocol: %d\n", h->nextProtocol);
    printf("    checksum: 0x%04x\n", h->checksum);
    printf("    src addr: %s, dst addr: %s\n", h->srcAddrStr().c_str(), h->dstAddrStr().c_str());
}

void printARP(const std::shared_ptr<npacket::ArpHeader>& h)
{
    printf("    ----- ARP -----\n");
    printf("    header len: %d\n", h->headerLen);
    printf("    hardware type: 0x%04x, hardware size: %d\n", h->hardwareType, h->hardwareSize);
    printf("    protocol type: 0x%04x, protocol size: %d\n", h->protocolType, h->protocolSize);
    printf("    opcode: %d\n", h->opcode);
    printf("    sender mac: %s, sender ip: %s\n", h->senderMacStr().c_str(), h->senderIpStr().c_str());
    printf("    target mac: %s, target ip: %s\n", h->targetMacStr().c_str(), h->targetIpStr().c_str());
}

void printIPv6(const std::shared_ptr<npacket::Ipv6Header>& h)
{
    printf("    ----- IPv6 -----\n");
    printf("    version: %d, traffic class: %d, flow label: %u, payload len: %d\n", h->version, h->trafficClass, h->flowLabel,
           h->payloadLen);
    printf("    next header: %d\n", h->nextHeader);
    printf("    hop limit: %d\n", h->hopLimit);
    printf("    srcAddr: %s, dstAddr: %s\n", h->srcAddrStr().c_str(), h->dstAddrStr().c_str());
    if (0 == h->nextHeader) /* 扩展包头: Hop-by-Hop */
    {
        printf("    [hop by hop] next header: %d\n", h->hopByHopHeader.nextHeader);
        printf("    [hop by hop] length: %d\n", h->hopByHopHeader.length);
        printf("    [hop by hop] options(%d):", h->hopByHopHeader.optionLen);
        if (h->hopByHopHeader.options)
        {
            for (int i = 0; i < h->hopByHopHeader.optionLen; ++i)
            {
                printf(" %02x", h->hopByHopHeader.options[i]);
            }
        }
        printf("\n");
    }
}

void printTCP(const std::shared_ptr<npacket::TcpHeader>& h)
{
    printf("        ----- TCP -----\n");
    printf("        src port: %d, dst port: %d\n", h->srcPort, h->dstPort);
    printf("        seq: %u, ack: %u\n", h->seq, h->ack);
    printf("        header len: %d\n", h->headerLen);
    printf("        reserved: %d, nonce: %d, cwr: %d, ecn_echo: %d, urgent: %d, ack: %d, push: %d, reset: %d, syn: %d, fin: %d\n",
           h->flagRsrvd, h->flagNonce, h->flagCwr, h->flagEce, h->flagUrg, h->flagAck, h->flagPsh, h->flagRst, h->flagSyn, h->flagFin);
    printf("        window: %d\n", h->window);
    printf("        checksum: 0x%04x\n", h->checksum);
    printf("        urgptr: %d\n", h->urgptr);
}

void printUDP(const std::shared_ptr<npacket::UdpHeader>& h)
{
    printf("        ----- UDP -----\n");
    printf("        src port: %d, dst port: %d\n", h->srcPort, h->dstPort);
    printf("        total len: %d\n", h->totalLen);
    printf("        checksum: 0x%04x\n", h->checksum);
}

void printICMP(const std::shared_ptr<npacket::IcmpHeader>& h)
{
    printf("        ----- ICMP -----\n");
    printf("        type: %d, code: %d, checksum: 0x%04x\n", h->type, h->code, h->checksum);
}

void printICMPv6(const std::shared_ptr<npacket::Icmpv6Header>& h)
{
    printf("        ----- ICMPv6 -----\n");
    printf("        type: %d, code: %d, checksum: 0x%04x\n", h->type, h->code, h->checksum);
}

bool handleEthernetLayer(uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload,
                         uint32_t payloadLen)
{
    auto h = std::dynamic_pointer_cast<npacket::EthernetIIHeader>(header);
    if (s_proto.empty() || "ehternet" == s_proto)
    {
        printEthernet(h);
    }
    return true;
}

bool handleNetworkLayer(uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload,
                        uint32_t payloadLen)
{
    switch ((npacket::NetworkProtocol)header->getProtocol())
    {
    case npacket::NetworkProtocol::IPv4: {
        auto h = std::dynamic_pointer_cast<npacket::Ipv4Header>(header);
        if (s_proto.empty() || "ipv4" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent));
            }
            printIPv4(h);
        }
    }
    break;
    case npacket::NetworkProtocol::ARP: {
        auto h = std::dynamic_pointer_cast<npacket::ArpHeader>(header);
        if (s_proto.empty() || "arp" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent));
            }
            printARP(h);
        }
    }
    break;
    case npacket::NetworkProtocol::IPv6: {
        auto h = std::dynamic_pointer_cast<npacket::Ipv6Header>(header);
        if (s_proto.empty() || "ipv4" == s_proto)
        {
            if (!s_proto.empty())
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

bool handleTransportLayer(uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload,
                          uint32_t payloadLen)
{
    switch ((npacket::TransportProtocol)header->getProtocol())
    {
    case npacket::TransportProtocol::TCP: {
        auto h = std::dynamic_pointer_cast<npacket::TcpHeader>(header);
        if (s_proto.empty() || "tcp" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent->parent));
                switch (h->parent->getProtocol())
                {
                case npacket::NetworkProtocol::IPv4:
                    printIPv4(std::dynamic_pointer_cast<npacket::Ipv4Header>(h->parent));
                    break;
                case npacket::NetworkProtocol::IPv6:
                    printIPv6(std::dynamic_pointer_cast<npacket::Ipv6Header>(h->parent));
                    break;
                }
            }
            printTCP(h);
        }
    }
    break;
    case npacket::TransportProtocol::UDP: {
        auto h = std::dynamic_pointer_cast<npacket::UdpHeader>(header);
        if (s_proto.empty() || "udp" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent->parent));
                switch (h->parent->getProtocol())
                {
                case npacket::NetworkProtocol::IPv4:
                    printIPv4(std::dynamic_pointer_cast<npacket::Ipv4Header>(h->parent));
                    break;
                case npacket::NetworkProtocol::IPv6:
                    printIPv6(std::dynamic_pointer_cast<npacket::Ipv6Header>(h->parent));
                    break;
                }
            }
            printUDP(h);
        }
    }
    break;
    case npacket::TransportProtocol::ICMP: {
        auto h = std::dynamic_pointer_cast<npacket::IcmpHeader>(header);
        if (s_proto.empty() || "icmp" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent->parent));
                if (npacket::NetworkProtocol::IPv4 == h->parent->getProtocol())
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
        if (s_proto.empty() || "icmpv6" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent->parent));
                if (npacket::NetworkProtocol::IPv6 == h->parent->getProtocol())
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

void handleApplicationFtpCtrl(uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const std::string& flag,
                              const std::string& param)
{
    if (s_proto.empty() || "ftp" == s_proto)
    {
        if (!s_proto.empty())
        {
            printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(header->parent->parent));
            if (npacket::NetworkProtocol::IPv4 == header->parent->getProtocol())
            {
                printIPv4(std::dynamic_pointer_cast<npacket::Ipv4Header>(header->parent));
            }
            else if (npacket::NetworkProtocol::IPv6 == header->parent->getProtocol())
            {
                printIPv6(std::dynamic_pointer_cast<npacket::Ipv6Header>(header->parent));
            }
            printTCP(std::dynamic_pointer_cast<npacket::TcpHeader>(header));
        }
        printf("            ----- FTP -----\n");
        printf("            %s: %s\n", (flag[0] >= 'A' && flag[0] <= 'Z') ? "cmd" : "code", flag.c_str());
        if (!param.empty())
        {
            printf("            param: %s\n", param.c_str());
        }
    }
}

void handleApplicationFtpData(uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, uint32_t mode, uint32_t flag,
                              const uint8_t* data, uint32_t dataLen)
{
    if (s_proto.empty() || "ftp-data" == s_proto)
    {
        if (!s_proto.empty())
        {
            printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(header->parent->parent));
            if (npacket::NetworkProtocol::IPv4 == header->parent->getProtocol())
            {
                printIPv4(std::dynamic_pointer_cast<npacket::Ipv4Header>(header->parent));
            }
            else if (npacket::NetworkProtocol::IPv6 == header->parent->getProtocol())
            {
                printIPv6(std::dynamic_pointer_cast<npacket::Ipv6Header>(header->parent));
            }
            printTCP(std::dynamic_pointer_cast<npacket::TcpHeader>(header));
        }
        if (1 == flag)
        {
            printf("            ----- FTP-DATA [%s][start] -----\n", 1 == mode ? "PORT" : "PASV");
        }
        else if (2 == flag)
        {
            printf("            ----- FTP-DATA [%s][%d] -----\n", 1 == mode ? "PORT" : "PASV", dataLen);
            printf("%s\n", std::string(data, data + dataLen).c_str());
        }
        else if (3 == flag)
        {
            printf("            ----- FTP-DATA [%s][finish] -----\n", 1 == mode ? "PORT" : "PASV");
        }
        else if (4 == flag)
        {
            printf("            ----- FTP-DATA [%s][timeout] -----\n", 1 == mode ? "PORT" : "PASV");
        }
    }
}

int main(int argc, char* argv[])
{
    printf("*************************************************************************************************************\n");
    printf("** 说明: 网络数据抓包工具.                                                                                 **\n");
    printf("**                                                                                                         **\n");
    printf("** 选项:                                                                                                   **\n");
    printf("**                                                                                                         **\n");
    printf("** [-l]                显示所有网络设备.                                                                   **\n");
#ifdef _WIN32
    printf("** [-i 名称]           网络设备名, 例如: {873ADB71-0F07-4857-9482-50B4DE0F6A68}等.                         **\n");
#else
    printf("** [-i 名称]           网络设备名, 例如: br0, enp1s0等.                                                    **\n");
#endif
    printf("** [-ip IP]            IPv4地址, 例如: 192.168.34.12.                                                      **\n");
#ifndef _WIN32
    printf("** [-Q 流向]           抓包流向, 值范围: [inout-所有(默认), in-接收, out-发送]                             **\n");
#endif
    printf("** [-p 协议]           指定只显示的协议, 例如: ehternet, ipv4, arp, ipv6, tcp, udp, icmp, icmpv6等         **\n");
    printf("**                                                                                                         **\n");
    printf("** 示例:                                                                                                   **\n");
    printf("**       npacket_tool.exe -i enp2s0 -Q out                                                                 **\n");
    printf("**                                                                                                         **\n");
    printf("*************************************************************************************************************\n");
    printf("\n");
    bool showList = false;
    std::string name;
    std::string ip;
    int direction = 0;
    for (int i = 1; i < argc;)
    {
        std::string key = argv[i];
        if (0 == key.compare("-l")) /* 显示设备列表 */
        {
            showList = true;
            i += 1;
        }
        else if (0 == key.compare("-i")) /* 设备接口 */
        {
            ++i;
            if (i < argc)
            {
                name = argv[i];
                ++i;
            }
        }
        else if (0 == key.compare("-ip")) /* 设备IP */
        {
            ++i;
            if (i < argc)
            {
                ip = argv[i];
                ++i;
            }
        }
#ifndef _WIN32
        else if (0 == key.compare("-Q")) /* 抓包流向 */
        {
            ++i;
            if (i < argc)
            {
                std::string value(argv[i]);
                std::transform(value.begin(), value.end(), value.begin(), tolower);
                if ("in" == value)
                {
                    direction = 1;
                }
                else if ("out" == value)
                {
                    direction = 2;
                }
                ++i;
            }
        }
#endif
        else if (0 == key.compare("-p")) /* 协议 */
        {
            ++i;
            if (i < argc)
            {
                s_proto = argv[i];
                std::transform(s_proto.begin(), s_proto.end(), s_proto.begin(), tolower);
                ++i;
            }
        }
    }
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
    if (name.empty() && ip.empty())
    {
        printf("未指定要抓包的设备名或IPv4地址\n");
        return 0;
    }
    if (!name.empty())
    {
        printf("设备名: %s\n", name.c_str());
    }
    if (!ip.empty())
    {
        printf("  IPv4: %s\n", ip.c_str());
    }
    printf("包流向: %s\n", 1 == direction ? "in(接收)" : (2 == direction ? "out(发送)" : "inout(所有)"));
    s_pktAnalyzer.setLayerCallback([&](uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload,
                                       uint32_t payloadLen) { return handleEthernetLayer(totalLen, header, payload, payloadLen); },
                                   [&](uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload,
                                       uint32_t payloadLen) { return handleNetworkLayer(totalLen, header, payload, payloadLen); },
                                   [&](uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload,
                                       uint32_t payloadLen) { return handleTransportLayer(totalLen, header, payload, payloadLen); });
    auto ftpParser = std::make_shared<npacket::FtpParser>();
    ftpParser->setRequestCallback(handleApplicationFtpCtrl);
    ftpParser->setResponseCallback(handleApplicationFtpCtrl);
    ftpParser->setDataCallback(handleApplicationFtpData);
    s_pktAnalyzer.addProtocolParser(ftpParser);
    std::shared_ptr<npacket::PcapDevice> dev;
    for (size_t i = 0; i < devList.size(); ++i)
    {
        if ((!name.empty() && devList[i]->getName() == name) || (!ip.empty() && devList[i]->getIpv4Address() == ip))
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
    if (name.empty())
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
