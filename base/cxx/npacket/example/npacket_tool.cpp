#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "../npacket/analyzer.h"
#include "../npacket/device/pcap_device.h"
#include "../npacket/proto/ftp.h"
#include "../npacket/proto/iec103.h"

static std::shared_ptr<npacket::Analyzer> s_pktAnalyzer = nullptr;
static std::string s_proto;

/* 打印以太网 */
void printEthernet(const npacket::EthernetIIHeader* h)
{
    printf("=============== EthernetII ===============\n");
    std::string srcMac, dstMac;
    h->srcMacStr(srcMac);
    h->dstMacStr(dstMac);
    printf("src mac: %s, dst mac: %s\n", srcMac.c_str(), dstMac.c_str());
    printf("type: 0x%04x\n", h->nextProtocol);
}

/* 打印IPv4 */
void printIPv4(const npacket::Ipv4Header* h)
{
    std::string srcAddr, dstAddr;
    h->srcAddrStr(srcAddr);
    h->dstAddrStr(dstAddr);
    printf("    ----- IPv4 -----\n");
    printf("    version: %d, header len: %d, tos: %d, total len: %d\n", h->version, h->headerLen, h->tos, h->totalLen);
    printf("    identification: 0x%04x (%d)\n", h->identification, h->identification);
    printf("    reserved: %d, dont: %d, more: %d\n", h->flagRsrvd, h->flagDont, h->flagMore);
    printf("    frag offset: %d\n", h->fragOffset);
    printf("    ttl: %d\n", h->ttl);
    printf("    protocol: %d\n", h->nextProtocol);
    printf("    checksum: 0x%04x\n", h->checksum);
    printf("    src addr: %s, dst addr: %s\n", srcAddr.c_str(), dstAddr.c_str());
}

/* 打印ARP */
void printARP(const npacket::ArpHeader* h)
{
    std::string senderMac, senderIp, targetMac, targetIp;
    h->senderMacStr(senderMac);
    h->senderIpStr(senderIp);
    h->targetMacStr(targetMac);
    h->targetIpStr(targetIp);
    printf("    ----- ARP -----\n");
    printf("    header len: %d\n", h->headerLen);
    printf("    hardware type: 0x%04x, hardware size: %d\n", h->hardwareType, h->hardwareSize);
    printf("    protocol type: 0x%04x, protocol size: %d\n", h->protocolType, h->protocolSize);
    printf("    opcode: %d\n", h->opcode);
    printf("    sender mac: %s, sender ip: %s\n", senderMac.c_str(), senderIp.c_str());
    printf("    target mac: %s, target ip: %s\n", targetMac.c_str(), targetIp.c_str());
}

/* 打印IPv6 */
void printIPv6(const npacket::Ipv6Header* h)
{
    std::string srcAddr, dstAddr;
    h->srcAddrStr(srcAddr);
    h->dstAddrStr(dstAddr);
    printf("    ----- IPv6 -----\n");
    printf("    version: %d, traffic class: %d, flow label: %u, payload len: %d\n", h->version, h->trafficClass, h->flowLabel,
           h->payloadLen);
    printf("    next header: %d\n", h->nextHeader);
    printf("    hop limit: %d\n", h->hopLimit);
    printf("    srcAddr: %s, dstAddr: %s\n", srcAddr.c_str(), dstAddr.c_str());
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

/* 打印TCP */
void printTCP(const npacket::TcpHeader* h)
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

/* 打印UDP */
void printUDP(const npacket::UdpHeader* h)
{
    printf("        ----- UDP -----\n");
    printf("        src port: %d, dst port: %d\n", h->srcPort, h->dstPort);
    printf("        total len: %d\n", h->totalLen);
    printf("        checksum: 0x%04x\n", h->checksum);
}

/* 打印ICMP */
void printICMP(const npacket::IcmpHeader* h)
{
    printf("        ----- ICMP -----\n");
    printf("        type: %d, code: %d, checksum: 0x%04x\n", h->type, h->code, h->checksum);
}

/* 打印ICMPv6 */
void printICMPv6(const npacket::Icmpv6Header* h)
{
    printf("        ----- ICMPv6 -----\n");
    printf("        type: %d, code: %d, checksum: 0x%04x\n", h->type, h->code, h->checksum);
}

/* 打印传输层(TCP)头部 */
void printTransportHeaderTcp(const npacket::ProtocolHeader* h)
{
    if (!s_proto.empty())
    {
        printEthernet((const npacket::EthernetIIHeader*)(h->parent->parent));
        if (npacket::NetworkProtocol::IPv4 == h->parent->getProtocol())
        {
            printIPv4((const npacket::Ipv4Header*)(h->parent));
        }
        else if (npacket::NetworkProtocol::IPv6 == h->parent->getProtocol())
        {
            printIPv6((const npacket::Ipv6Header*)(h->parent));
        }
        printTCP((const npacket::TcpHeader*)(h));
    }
}

/* 处理以太网层 */
bool handleEthernetLayer(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                         const npacket::ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen)
{
    auto h = (const npacket::EthernetIIHeader*)(header);
    if (s_proto.empty() || "ehternet" == s_proto)
    {
        printEthernet(h);
    }
    return true;
}

/* 处理网络层 */
bool handleNetworkLayer(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                        const npacket::ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen)
{
    switch ((npacket::NetworkProtocol)header->getProtocol())
    {
    case npacket::NetworkProtocol::IPv4: {
        auto h = (const npacket::Ipv4Header*)(header);
        if (s_proto.empty() || "ipv4" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet((const npacket::EthernetIIHeader*)(h->parent));
            }
            printIPv4(h);
        }
    }
    break;
    case npacket::NetworkProtocol::ARP: {
        auto h = (const npacket::ArpHeader*)(header);
        if (s_proto.empty() || "arp" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet((const npacket::EthernetIIHeader*)(h->parent));
            }
            printARP(h);
        }
    }
    break;
    case npacket::NetworkProtocol::IPv6: {
        auto h = (const npacket::Ipv6Header*)(header);
        if (s_proto.empty() || "ipv4" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet((const npacket::EthernetIIHeader*)(h->parent));
            }
            printIPv6(h);
        }
    }
    break;
    }
    return true;
}

/* 处理传输层 */
bool handleTransportLayer(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                          const npacket::ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen)
{
    switch ((npacket::TransportProtocol)header->getProtocol())
    {
    case npacket::TransportProtocol::TCP: {
        auto h = (const npacket::TcpHeader*)(header);
        if (s_proto.empty() || "tcp" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet((const npacket::EthernetIIHeader*)(h->parent->parent));
                switch (h->parent->getProtocol())
                {
                case npacket::NetworkProtocol::IPv4:
                    printIPv4((const npacket::Ipv4Header*)(h->parent));
                    break;
                case npacket::NetworkProtocol::IPv6:
                    printIPv6((const npacket::Ipv6Header*)(h->parent));
                    break;
                }
            }
            printTCP(h);
        }
    }
    break;
    case npacket::TransportProtocol::UDP: {
        auto h = (const npacket::UdpHeader*)(header);
        if (s_proto.empty() || "udp" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet((const npacket::EthernetIIHeader*)(h->parent->parent));
                switch (h->parent->getProtocol())
                {
                case npacket::NetworkProtocol::IPv4:
                    printIPv4((const npacket::Ipv4Header*)(h->parent));
                    break;
                case npacket::NetworkProtocol::IPv6:
                    printIPv6((const npacket::Ipv6Header*)(h->parent));
                    break;
                }
            }
            printUDP(h);
        }
    }
    break;
    case npacket::TransportProtocol::ICMP: {
        auto h = (const npacket::IcmpHeader*)(header);
        if (s_proto.empty() || "icmp" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet((const npacket::EthernetIIHeader*)(h->parent->parent));
                if (npacket::NetworkProtocol::IPv4 == h->parent->getProtocol())
                {
                    printIPv4((const npacket::Ipv4Header*)(h->parent));
                }
            }
            printICMP(h);
        }
    }
    break;
    case npacket::TransportProtocol::ICMPv6: {
        auto h = (const npacket::Icmpv6Header*)(header);
        if (s_proto.empty() || "icmpv6" == s_proto)
        {
            if (!s_proto.empty())
            {
                printEthernet((const npacket::EthernetIIHeader*)(h->parent->parent));
                if (npacket::NetworkProtocol::IPv6 == h->parent->getProtocol())
                {
                    printIPv6((const npacket::Ipv6Header*)(h->parent));
                }
            }
            printICMPv6(h);
        }
    }
    break;
    }
    return true;
}

/* 处理应用层FTP控制请求 */
void handleApplicationFtpCtrlReq(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const npacket::ProtocolHeader* header,
                                 const std::string& flag, const std::string& param)
{
    if (s_proto.empty() || "ftp" == s_proto)
    {
        printTransportHeaderTcp(header);
        printf("            ----- FTP [request] -----\n");
        printf("            %s: %s\n", (flag[0] >= 'A' && flag[0] <= 'Z') ? "cmd" : "code", flag.c_str());
        if (!param.empty())
        {
            printf("            param: %s\n", param.c_str());
        }
    }
}

/* 处理应用层FTP控制应答 */
void handleApplicationFtpCtrlResp(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                  const npacket::ProtocolHeader* header, const std::string& flag, const std::string& param)
{
    if (s_proto.empty() || "ftp" == s_proto)
    {
        printTransportHeaderTcp(header);
        printf("            ----- FTP [response] -----\n");
        printf("            %s: %s\n", (flag[0] >= 'A' && flag[0] <= 'Z') ? "cmd" : "code", flag.c_str());
        if (!param.empty())
        {
            printf("            param: %s\n", param.c_str());
        }
    }
}

/* 处理应用层FTP数据 */
void handleApplicationFtpData(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const npacket::ProtocolHeader* header,
                              const npacket::FtpParser::CtrlInfo& ctrl, const npacket::FtpParser::DataFlag& flag, const uint8_t* data,
                              uint32_t dataLen)
{
    if (s_proto.empty() || "ftp-data" == s_proto)
    {
        printTransportHeaderTcp(header);
        std::string modeDesc = npacket::FtpParser::DataMode::ACTIVE == ctrl.mode ? "PORT" : "PASV";
        if (npacket::FtpParser::DataFlag::READY == flag)
        {
            printf("            ----- FTP-DATA [%s][start][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), ctrl.clientIp.c_str(),
                   ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
        }
        else if (npacket::FtpParser::DataFlag::BODY == flag)
        {
            printf("            ----- FTP-DATA [%s][%d][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), dataLen,
                   ctrl.clientIp.c_str(), ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
            printf("%s\n", std::string(data, data + dataLen).c_str());
        }
        else if (npacket::FtpParser::DataFlag::FINISH == flag)
        {
            printf("            ----- FTP-DATA [%s][finish][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), ctrl.clientIp.c_str(),
                   ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
        }
        else if (npacket::FtpParser::DataFlag::ABNORMAL == flag)
        {
            printf("            ----- FTP-DATA [%s][timeout][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(),
                   ctrl.clientIp.c_str(), ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
        }
    }
}

/* 处理应用层IEC103固定帧 */
void handleApplicationIec103FixedFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                       const npacket::ProtocolHeader* header, const std::shared_ptr<npacket::iec103::FixedFrame>& frame)
{
    if (s_proto.empty() || "iec103" == s_proto)
    {
        printTransportHeaderTcp(header);
        printf("            ----- IEC103 [fixed frame] -----\n");
        printf("            PRM: %d(%s)\n", frame->prm, 1 == frame->prm ? "mater to slave" : "slave to master");
        printf("            %s: %d\n", 1 == frame->prm ? "FCB" : "ACD", frame->fcb_acd);
        printf("            %s: %d\n", 1 == frame->prm ? "FCV" : "DFC", frame->fcv_dfc);
        printf("            FUNC: %d\n", frame->func);
        printf("            ADDR: %d\n", frame->addr);
    }
}

/* 处理应用层IEC103可变帧 */
void handleApplicationIec103VariableFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                          const npacket::ProtocolHeader* header,
                                          const std::shared_ptr<npacket::iec103::VariableFrame>& frame)
{
    if (s_proto.empty() || "iec103" == s_proto)
    {
        printTransportHeaderTcp(header);
        printf("            ----- IEC103 [variable frame] -----\n");
        printf("            PRM: %d(%s)\n", frame->prm, 1 == frame->prm ? "mater to slave" : "slave to master");
        printf("            %s: %d\n", 1 == frame->prm ? "FCB" : "ACD", frame->fcb_acd);
        printf("            %s: %d\n", 1 == frame->prm ? "FCV" : "DFC", frame->fcv_dfc);
        printf("            FUNC: %d\n", frame->func);
        printf("            ADDR: %d\n", frame->addr);
        if (frame->asdu)
        {
            printf("            ASDU%d:\n", frame->asdu->identify.type);
            printf("                  type: %d\n", frame->asdu->identify.type);
            printf("                  vsq: continuous[%d], num[%d]\n", frame->asdu->identify.vsq.continuous, frame->asdu->identify.vsq.num);
            printf("                  cot: %d\n", frame->asdu->identify.cot);
            printf("                  commonAddr: %d\n", frame->asdu->identify.commonAddr);
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
    printf("** [-p 协议]           指定只显示的协议, 例如: ehternet, ipv4, arp, ipv6, tcp, udp, icmp, icmpv6,          **\n");
    printf("**                     ftp, ftp-data, iec103等                                                             **\n");
    printf("**                                                                                                         **\n");
    printf("** 示例:                                                                                                   **\n");
    printf("**       npacket_tool.exe -i enp2s0 -Q out -p ftp                                                          **\n");
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
    npacket::CallbackConfig cbCfg;
    cbCfg.ethernetLayerCb = handleEthernetLayer;
    cbCfg.networkLayerCb = handleNetworkLayer;
    cbCfg.transportLayerCb = handleTransportLayer;
    s_pktAnalyzer = std::make_shared<npacket::Analyzer>(cbCfg);
    {
        auto ftpParser = std::make_shared<npacket::FtpParser>();
        ftpParser->setRequestCallback(handleApplicationFtpCtrlReq);
        ftpParser->setResponseCallback(handleApplicationFtpCtrlResp);
        ftpParser->setDataCallback(handleApplicationFtpData);
        s_pktAnalyzer->addProtocolParser(ftpParser);
    }
    {
        auto iec103Parser = std::make_shared<npacket::Iec103Parser>();
        iec103Parser->setFixedFrameCallback(handleApplicationIec103FixedFrame);
        iec103Parser->setVariableFrameCallback(handleApplicationIec103VariableFrame);
        s_pktAnalyzer->addProtocolParser(iec103Parser);
    }
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
    dev->setDataCallback([&](const unsigned char* data, int dataLen) {
        static size_t num = 1;
        s_pktAnalyzer->parse(num++, data, dataLen);
    });
    dev->startCapture();
    while (1)
    {
        dev->captureOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    dev->close();
    return 0;
}
