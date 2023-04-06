#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "../npacket/analyzer.h"
#include "../npacket/pcap_device.h"
#include "../npacket/proto/ftp.h"
#include "../npacket/proto/iec103.h"

static npacket::Analyzer s_pktAnalyzer;
static std::string s_proto;

/* 打印以太网 */
void printEthernet(const std::shared_ptr<npacket::EthernetIIHeader>& h)
{
    printf("=============== EthernetII ===============\n");
    printf("src mac: %s, dst mac: %s\n", h->srcMacStr().c_str(), h->dstMacStr().c_str());
    printf("type: 0x%04x\n", h->nextProtocol);
}

/* 打印IPv4 */
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

/* 打印ARP */
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

/* 打印IPv6 */
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

/* 打印TCP */
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

/* 打印UDP */
void printUDP(const std::shared_ptr<npacket::UdpHeader>& h)
{
    printf("        ----- UDP -----\n");
    printf("        src port: %d, dst port: %d\n", h->srcPort, h->dstPort);
    printf("        total len: %d\n", h->totalLen);
    printf("        checksum: 0x%04x\n", h->checksum);
}

/* 打印ICMP */
void printICMP(const std::shared_ptr<npacket::IcmpHeader>& h)
{
    printf("        ----- ICMP -----\n");
    printf("        type: %d, code: %d, checksum: 0x%04x\n", h->type, h->code, h->checksum);
}

/* 打印ICMPv6 */
void printICMPv6(const std::shared_ptr<npacket::Icmpv6Header>& h)
{
    printf("        ----- ICMPv6 -----\n");
    printf("        type: %d, code: %d, checksum: 0x%04x\n", h->type, h->code, h->checksum);
}

/* 打印传输层(TCP)头部 */
void printTransportHeaderTcp(const std::shared_ptr<npacket::ProtocolHeader>& h)
{
    if (!s_proto.empty())
    {
        printEthernet(std::dynamic_pointer_cast<npacket::EthernetIIHeader>(h->parent->parent));
        if (npacket::NetworkProtocol::IPv4 == h->parent->getProtocol())
        {
            printIPv4(std::dynamic_pointer_cast<npacket::Ipv4Header>(h->parent));
        }
        else if (npacket::NetworkProtocol::IPv6 == h->parent->getProtocol())
        {
            printIPv6(std::dynamic_pointer_cast<npacket::Ipv6Header>(h->parent));
        }
        printTCP(std::dynamic_pointer_cast<npacket::TcpHeader>(h));
    }
}

/* 处理以太网层 */
bool handleEthernetLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                         const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
{
    auto h = std::dynamic_pointer_cast<npacket::EthernetIIHeader>(header);
    if (s_proto.empty() || "ehternet" == s_proto)
    {
        printEthernet(h);
    }
    return true;
}

/* 处理网络层 */
bool handleNetworkLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                        const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
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

/* 处理传输层 */
bool handleTransportLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                          const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)
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

/* 处理应用层FTP控制请求 */
void handleApplicationFtpCtrlReq(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                 const std::shared_ptr<npacket::ProtocolHeader>& header, const std::string& flag, const std::string& param)
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
                                  const std::shared_ptr<npacket::ProtocolHeader>& header, const std::string& flag, const std::string& param)
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
void handleApplicationFtpData(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                              const std::shared_ptr<npacket::ProtocolHeader>& header, const npacket::FtpParser::CtrlInfo& ctrl,
                              const npacket::FtpParser::DataFlag& flag, const uint8_t* data, uint32_t dataLen)
{
    if (s_proto.empty() || "ftp-data" == s_proto)
    {
        printTransportHeaderTcp(header);
        std::string modeDesc = npacket::FtpParser::DataMode::active == ctrl.mode ? "PORT" : "PASV";
        if (npacket::FtpParser::DataFlag::ready == flag)
        {
            printf("            ----- FTP-DATA [%s][start][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), ctrl.clientIp.c_str(),
                   ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
        }
        else if (npacket::FtpParser::DataFlag::body == flag)
        {
            printf("            ----- FTP-DATA [%s][%d][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), dataLen,
                   ctrl.clientIp.c_str(), ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
            printf("%s\n", std::string(data, data + dataLen).c_str());
        }
        else if (npacket::FtpParser::DataFlag::finish == flag)
        {
            printf("            ----- FTP-DATA [%s][finish][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), ctrl.clientIp.c_str(),
                   ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
        }
        else if (npacket::FtpParser::DataFlag::abnormal == flag)
        {
            printf("            ----- FTP-DATA [%s][timeout][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(),
                   ctrl.clientIp.c_str(), ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
        }
    }
}

/* 处理应用层IEC103固定帧 */
void handleApplicationIec103FixedFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                       const std::shared_ptr<npacket::ProtocolHeader>& header,
                                       const std::shared_ptr<npacket::iec103::FixedFrame>& frame)
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
                                          const std::shared_ptr<npacket::ProtocolHeader>& header,
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
            switch (frame->asdu->identify.type)
            {
            case 0x01: {
                auto asdu1 = std::static_pointer_cast<npacket::iec103::Asdu1>(frame->asdu);
                if (asdu1)
                {
                    printf("                  func: %d\n", asdu1->func);
                    printf("                  inf: %d\n", asdu1->inf);
                    printf("                  dpi: %d\n", asdu1->dpi);
                    printf("                  time: %02d:%02d %d(ms) summer[%d]\n", asdu1->tm.hour, asdu1->tm.minute, asdu1->tm.millisecond,
                           asdu1->tm.summerTime);
                    printf("                  sin: %d\n", asdu1->sin);
                }
            }
            break;
            case 0x02: {
                auto asdu2 = std::static_pointer_cast<npacket::iec103::Asdu2>(frame->asdu);
                if (asdu2)
                {
                    printf("                  func: %d\n", asdu2->func);
                    printf("                  inf: %d\n", asdu2->inf);
                    printf("                  dpi: %d\n", asdu2->dpi);
                    printf("                  ret: %d\n", asdu2->ret);
                    printf("                  fan: %d\n", asdu2->fan);
                    printf("                  time: %02d:%02d %d(ms) summer[%d]\n", asdu2->tm.hour, asdu2->tm.minute, asdu2->tm.millisecond,
                           asdu2->tm.summerTime);
                    printf("                  sin: %d\n", asdu2->sin);
                }
            }
            break;
            case 0x03: {
                auto asdu3 = std::static_pointer_cast<npacket::iec103::Asdu3>(frame->asdu);
                if (asdu3)
                {
                    printf("                  func: %d\n", asdu3->func);
                    printf("                  inf: %d\n", asdu3->inf);
                    for (size_t i = 0; i < asdu3->meaList.size(); ++i)
                    {
                        const auto& mea = asdu3->meaList[i];
                        printf("                  mea[%03zu]: ov[%d], er[%d], res[%d], mval[%f]\n", i, mea.ov, mea.er, mea.res, mea.mval);
                    }
                }
            }
            break;
            case 0x04: {
                auto asdu4 = std::static_pointer_cast<npacket::iec103::Asdu4>(frame->asdu);
                if (asdu4)
                {
                    printf("                  func: %d\n", asdu4->func);
                    printf("                  inf: %d\n", asdu4->inf);
                    printf("                  scl: %f\n", asdu4->scl);
                    printf("                  ret: %d\n", asdu4->ret);
                    printf("                  fan: %d\n", asdu4->fan);
                    printf("                  time: %02d:%02d %d(ms) summer[%d]\n", asdu4->tm.hour, asdu4->tm.minute, asdu4->tm.millisecond,
                           asdu4->tm.summerTime);
                }
            }
            break;
            case 0x05: {
                auto asdu5 = std::static_pointer_cast<npacket::iec103::Asdu5>(frame->asdu);
                if (asdu5)
                {
                    printf("                  func: %d\n", asdu5->func);
                    printf("                  inf: %d\n", asdu5->inf);
                    printf("                  col: %d\n", asdu5->col);
                    printf("                  ascii1: %c\n", asdu5->ascii1);
                    printf("                  ascii2: %c\n", asdu5->ascii2);
                    printf("                  ascii3: %c\n", asdu5->ascii3);
                    printf("                  ascii4: %c\n", asdu5->ascii4);
                    printf("                  ascii5: %c\n", asdu5->ascii5);
                    printf("                  ascii6: %c\n", asdu5->ascii6);
                    printf("                  ascii7: %c\n", asdu5->ascii7);
                    printf("                  ascii8: %c\n", asdu5->ascii8);
                    printf("                  freeValue1: %d\n", asdu5->freeValue1);
                    printf("                  freeValue2: %d\n", asdu5->freeValue2);
                    printf("                  freeValue3: %d\n", asdu5->freeValue3);
                    printf("                  freeValue4: %d\n", asdu5->freeValue4);
                }
            }
            break;
            case 0x06: {
                auto asdu6 = std::static_pointer_cast<npacket::iec103::Asdu6>(frame->asdu);
                if (asdu6)
                {
                    printf("                  func: %d\n", asdu6->func);
                    printf("                  inf: %d\n", asdu6->inf);
                    printf("                  time: %02d:%02d %d(ms) summer[%d]\n", asdu6->tm.hour, asdu6->tm.minute, asdu6->tm.millisecond,
                           asdu6->tm.summerTime);
                }
            }
            break;
            case 0x07: {
                auto asdu7 = std::static_pointer_cast<npacket::iec103::Asdu7>(frame->asdu);
                if (asdu7)
                {
                    printf("                  func: %d\n", asdu7->func);
                    printf("                  inf: %d\n", asdu7->inf);
                    printf("                  scn: %d\n", asdu7->scn);
                }
            }
            break;
            case 0x08: {
                auto asdu8 = std::static_pointer_cast<npacket::iec103::Asdu8>(frame->asdu);
                if (asdu8)
                {
                    printf("                  func: %d\n", asdu8->func);
                    printf("                  inf: %d\n", asdu8->inf);
                    printf("                  scn: %d\n", asdu8->scn);
                }
            }
            break;
            case 0x09: {
                auto asdu9 = std::static_pointer_cast<npacket::iec103::Asdu9>(frame->asdu);
                if (asdu9)
                {
                    printf("                  func: %d\n", asdu9->func);
                    printf("                  inf: %d\n", asdu9->inf);
                    for (size_t i = 0; i < asdu9->meaList.size(); ++i)
                    {
                        const auto& mea = asdu9->meaList[i];
                        printf("                  mea[%03zu]: ov[%d], er[%d], res[%d], mval[%f]\n", i, mea.ov, mea.er, mea.res, mea.mval);
                    }
                }
            }
            break;
            case 0x0A: {
                auto asdu10 = std::static_pointer_cast<npacket::iec103::Asdu10>(frame->asdu);
                if (asdu10)
                {
                    printf("                  func: %d\n", asdu10->func);
                    printf("                  inf: %d\n", asdu10->inf);
                    printf("                  rii: %d\n", asdu10->rii);
                    printf("                  ngd: no[%d], count[%d], cont[%d]\n", asdu10->ngd.no, asdu10->ngd.count, asdu10->ngd.cont);
                    for (size_t i = 0; i < asdu10->dataSet.size(); ++i)
                    {
                        const auto& dataSet10 = asdu10->dataSet[i];
                        printf("                  dataSet[%03zu]: gin(group[%d], entry[%d])\n", i, dataSet10.gin.group,
                               dataSet10.gin.entry);
                        printf("                                kod[%d]\n", dataSet10.kod);
                        printf("                                gdd(dataType[%d], dataSize[%d], number[%d], cont[%d])\n",
                               dataSet10.gdd.dataType, dataSet10.gdd.dataSize, dataSet10.gdd.number, dataSet10.gdd.cont);
                        for (size_t j = 0; j < dataSet10.gidList.size(); ++j)
                        {
                            const auto& gid = dataSet10.gidList[j];
                            printf("                                gid[%03zu]: ", j);
                            switch (dataSet10.gdd.dataType)
                            {
                            case 1:
                                printf("ascii[%c]", gid.ascii);
                                break;
                            case 2:
                                printf("bsc[%d, %d, %d, %d]", gid.bsi[0], gid.bsi[1], gid.bsi[2], gid.bsi[3]);
                                break;
                            case 3:
                                printf("uintValue[%u]", gid.uintValue);
                                break;
                            case 4:
                                printf("intValue[%d]", gid.intValue);
                                break;
                            case 5:
                                printf("urealValue[%f]", gid.urealValue);
                                break;
                            case 6:
                                printf("realValue[%f]", gid.realValue);
                                break;
                            case 7:
                                printf("real32Value[%f]", gid.real32Value);
                                break;
                            case 8:
                                printf("real32Value[%f]", gid.real64Value);
                                break;
                            case 9:
                                printf("dpi[%d]", gid.dpi);
                                break;
                            case 10:
                                printf("spi[%d]", gid.spi);
                                break;
                            case 11:
                                printf("tedpi[%d]", gid.tedpi);
                                break;
                            case 12:
                                printf("mea(ov[%d], er[%d], res[%d], mval[%f])", gid.mea.ov, gid.mea.er, gid.mea.res, gid.mea.mval);
                                break;
                            case 14:
                                printf("time(%04d-%02d-%02d %02d:%02d %d(ms) wday[%d] summer[%d])", gid.tm.year, gid.tm.month, gid.tm.day,
                                       gid.tm.hour, gid.tm.minute, gid.tm.millisecond, gid.tm.wday, gid.tm.summerTime);
                                break;
                            case 15:
                                printf("gin(group[%d], entry[%d])", gid.gin.group, gid.gin.entry);
                                break;
                            case 16:
                                printf("ret[%d]", gid.ret);
                                break;
                            case 17:
                                printf("func[%d], inf[%d]", gid.func, gid.inf);
                                break;
                            case 18:
                                printf("dpi[%d], time(%02d:%02d %d(ms) summer[%d]), sin[%d]", gid.dpiWithTime.dpi, gid.dpiWithTime.tm.hour,
                                       gid.dpiWithTime.tm.minute, gid.dpiWithTime.tm.millisecond, gid.dpiWithTime.tm.summerTime,
                                       gid.dpiWithTime.sin);
                                break;
                            case 19:
                                printf("dpi[%d], ret[%d], fan[%d], time(%02d:%02d %d(ms) summer[%d]), sin[%d]", gid.dpiWithRet.dpi,
                                       gid.dpiWithRet.ret, gid.dpiWithRet.fan, gid.dpiWithRet.tm.hour, gid.dpiWithRet.tm.minute,
                                       gid.dpiWithRet.tm.millisecond, gid.dpiWithRet.tm.summerTime, gid.dpiWithRet.sin);
                                break;
                            case 20:
                                printf("real32Value[%f], ret[%d], fan[%d], time(%02d:%02d %d(ms) summer[%d])", gid.valWithRet.real32Value,
                                       gid.valWithRet.ret, gid.valWithRet.fan, gid.valWithRet.tm.hour, gid.valWithRet.tm.minute,
                                       gid.valWithRet.tm.millisecond, gid.valWithRet.tm.summerTime);
                                break;
                            case 22:
                                printf("grc[%d]", gid.grc);
                                break;
                            case 23:
                                printf("gdd(dataType[%d], dataSize[%d], number[%d], cont[%d])", gid.gdd.dataType, gid.gdd.dataSize,
                                       gid.gdd.number, gid.gdd.cont);
                                break;
                            }
                            printf("\n");
                        }
                    }
                }
            }
            break;
            case 0x0B: {
                auto asdu11 = std::static_pointer_cast<npacket::iec103::Asdu11>(frame->asdu);
                if (asdu11)
                {
                    printf("                  func: %d\n", asdu11->func);
                    printf("                  inf: %d\n", asdu11->inf);
                    printf("                  rii: %d\n", asdu11->rii);
                    printf("                  gin: group[%d], entry[%d]\n", asdu11->gin.group, asdu11->gin.entry);
                    printf("                  nde: no[%d], count[%d], cont[%d]\n", asdu11->nde.no, asdu11->nde.count, asdu11->nde.cont);
                    for (size_t i = 0; i < asdu11->dataSet.size(); ++i)
                    {
                        const auto& dataSet11 = asdu11->dataSet[i];
                        printf("                  dataSet[%03zu]: kod[%d]\n", i, dataSet11.kod);
                        printf("                                gdd(dataType[%d], dataSize[%d], number[%d], cont[%d])\n",
                               dataSet11.gdd.dataType, dataSet11.gdd.dataSize, dataSet11.gdd.number, dataSet11.gdd.cont);
                        for (size_t j = 0; j < dataSet11.gidList.size(); ++j)
                        {
                            const auto& gid = dataSet11.gidList[j];
                            printf("                                gid[%03zu]: ", j);
                            switch (dataSet11.gdd.dataType)
                            {
                            case 1:
                                printf("ascii[%c]", gid.ascii);
                                break;
                            case 2:
                                printf("bsc[%d, %d, %d, %d]", gid.bsi[0], gid.bsi[1], gid.bsi[2], gid.bsi[3]);
                                break;
                            case 3:
                                printf("uintValue[%u]", gid.uintValue);
                                break;
                            case 4:
                                printf("intValue[%d]", gid.intValue);
                                break;
                            case 5:
                                printf("urealValue[%f]", gid.urealValue);
                                break;
                            case 6:
                                printf("realValue[%f]", gid.realValue);
                                break;
                            case 7:
                                printf("real32Value[%f]", gid.real32Value);
                                break;
                            case 8:
                                printf("real32Value[%f]", gid.real64Value);
                                break;
                            case 9:
                                printf("dpi[%d]", gid.dpi);
                                break;
                            case 10:
                                printf("spi[%d]", gid.spi);
                                break;
                            case 11:
                                printf("tedpi[%d]", gid.tedpi);
                                break;
                            case 12:
                                printf("mea(ov[%d], er[%d], res[%d], mval[%f])", gid.mea.ov, gid.mea.er, gid.mea.res, gid.mea.mval);
                                break;
                            case 14:
                                printf("time(%04d-%02d-%-02d %02d:%02d %d(ms) wday[%d] summer[%d])", gid.tm.year, gid.tm.month, gid.tm.day,
                                       gid.tm.hour, gid.tm.minute, gid.tm.millisecond, gid.tm.wday, gid.tm.summerTime);
                                break;
                            case 15:
                                printf("gin(group[%d], entry[%d])", gid.gin.group, gid.gin.entry);
                                break;
                            case 16:
                                printf("ret[%d]", gid.ret);
                                break;
                            case 17:
                                printf("func[%d], inf[%d]", gid.func, gid.inf);
                                break;
                            case 18:
                                printf("dpi[%d], time(%02d:%02d %d(ms) summer[%d]), sin[%d]", gid.dpiWithTime.dpi, gid.dpiWithTime.tm.hour,
                                       gid.dpiWithTime.tm.minute, gid.dpiWithTime.tm.millisecond, gid.dpiWithTime.tm.summerTime,
                                       gid.dpiWithTime.sin);
                                break;
                            case 19:
                                printf("dpi[%d], ret[%d], fan[%d], time(%02d:%02d %d(ms) summer[%d]), sin[%d]", gid.dpiWithRet.dpi,
                                       gid.dpiWithRet.ret, gid.dpiWithRet.fan, gid.dpiWithRet.tm.hour, gid.dpiWithRet.tm.minute,
                                       gid.dpiWithRet.tm.millisecond, gid.dpiWithRet.tm.summerTime, gid.dpiWithRet.sin);
                                break;
                            case 20:
                                printf("real32Value[%f], ret[%d], fan[%d], time(%02d:%02d %d(ms) summer[%d])", gid.valWithRet.real32Value,
                                       gid.valWithRet.ret, gid.valWithRet.fan, gid.valWithRet.tm.hour, gid.valWithRet.tm.minute,
                                       gid.valWithRet.tm.millisecond, gid.valWithRet.tm.summerTime);
                                break;
                            case 22:
                                printf("grc[%d]", gid.grc);
                                break;
                            case 23:
                                printf("gdd(dataType[%d], dataSize[%d], number[%d], cont[%d])", gid.gdd.dataType, gid.gdd.dataSize,
                                       gid.gdd.number, gid.gdd.cont);
                                break;
                            }
                            printf("\n");
                        }
                    }
                }
            }
            break;
            case 0x14: {
                auto asdu20 = std::static_pointer_cast<npacket::iec103::Asdu20>(frame->asdu);
                if (asdu20)
                {
                    printf("                  func: %d\n", asdu20->func);
                    printf("                  inf: %d\n", asdu20->inf);
                    printf("                  dco: %d\n", asdu20->dco);
                    printf("                  rii: %d\n", asdu20->rii);
                }
            }
            break;
            case 0x15: {
                auto asdu21 = std::static_pointer_cast<npacket::iec103::Asdu21>(frame->asdu);
                if (asdu21)
                {
                    printf("                  func: %d\n", asdu21->func);
                    printf("                  inf: %d\n", asdu21->inf);
                    printf("                  rii: %d\n", asdu21->rii);
                    printf("                  nog: %d\n", asdu21->nog);
                    for (size_t i = 0; i < asdu21->dataSet.size(); ++i)
                    {
                        const auto& dataSet21 = asdu21->dataSet[i];
                        printf("                  dataSet[%03zu]: gin(group[%d], entry[%d]), kod(%d)\n", i, dataSet21.gin.group,
                               dataSet21.gin.entry, dataSet21.kod);
                    }
                }
            }
            break;
            case 0x17: {
                auto asdu23 = std::static_pointer_cast<npacket::iec103::Asdu23>(frame->asdu);
                if (asdu23)
                {
                    printf("                  func: %d\n", asdu23->func);
                    for (size_t i = 0; i < asdu23->dataSet.size(); ++i)
                    {
                        const auto& dataSet23 = asdu23->dataSet[i];
                        printf("                  dataSet[%03zu]: fan[%d]\n", i, dataSet23.fan);
                        printf("                                sof(tp[%d], tm[%d], test[%d], otev[%d], res[%d])\n", dataSet23.sof.tp,
                               dataSet23.sof.tm, dataSet23.sof.test, dataSet23.sof.otev, dataSet23.sof.res);
                        printf("                                time(%04d-%02d-%-02d %02d:%02d %d(ms) wday[%d] summer[%d])\n",
                               dataSet23.tm.year, dataSet23.tm.month, dataSet23.tm.day, dataSet23.tm.hour, dataSet23.tm.minute,
                               dataSet23.tm.millisecond, dataSet23.tm.wday, dataSet23.tm.summerTime);
                    }
                }
            }
            break;
            case 0x18: {
                auto asdu24 = std::static_pointer_cast<npacket::iec103::Asdu24>(frame->asdu);
                if (asdu24)
                {
                    printf("                  func: %d\n", asdu24->func);
                    printf("                  too: %d\n", asdu24->too);
                    printf("                  tov: %d\n", asdu24->tov);
                    printf("                  fan: %d\n", asdu24->fan);
                    printf("                  acc: %d\n", asdu24->acc);
                }
            }
            break;
            case 0x19: {
                auto asdu25 = std::static_pointer_cast<npacket::iec103::Asdu25>(frame->asdu);
                if (asdu25)
                {
                    printf("                  func: %d\n", asdu25->func);
                    printf("                  too: %d\n", asdu25->too);
                    printf("                  tov: %d\n", asdu25->tov);
                    printf("                  fan: %d\n", asdu25->fan);
                    printf("                  acc: %d\n", asdu25->acc);
                }
            }
            break;
            case 0x1A: {
                auto asdu26 = std::static_pointer_cast<npacket::iec103::Asdu26>(frame->asdu);
                if (asdu26)
                {
                    printf("                  func: %d\n", asdu26->func);
                    printf("                  tov: %d\n", asdu26->tov);
                    printf("                  fan: %d\n", asdu26->fan);
                    printf("                  nof: %d\n", asdu26->nof);
                    printf("                  noc: %d\n", asdu26->noc);
                    printf("                  noe: %d\n", asdu26->noe);
                    printf("                  interval: %d\n", asdu26->interval);
                    printf("                  time: %02d:%02d %d(ms) summer[%d]\n", asdu26->tm.hour, asdu26->tm.minute,
                           asdu26->tm.millisecond, asdu26->tm.summerTime);
                }
            }
            break;
            case 0x1B: {
                auto asdu27 = std::static_pointer_cast<npacket::iec103::Asdu27>(frame->asdu);
                if (asdu27)
                {
                    printf("                  func: %d\n", asdu27->func);
                    printf("                  tov: %d\n", asdu27->tov);
                    printf("                  fan: %d\n", asdu27->fan);
                    printf("                  acc: %d\n", asdu27->acc);
                    printf("                  rpv: %f\n", asdu27->rpv);
                    printf("                  rsv: %f\n", asdu27->rsv);
                    printf("                  rfa: %f\n", asdu27->rfa);
                }
            }
            break;
            case 0x1C: {
                auto asdu28 = std::static_pointer_cast<npacket::iec103::Asdu28>(frame->asdu);
                if (asdu28)
                {
                    printf("                  func: %d\n", asdu28->func);
                    printf("                  fan: %d\n", asdu28->fan);
                }
            }
            break;
            case 0x1D: {
                auto asdu29 = std::static_pointer_cast<npacket::iec103::Asdu29>(frame->asdu);
                if (asdu29)
                {
                    printf("                  func: %d\n", asdu29->func);
                    printf("                  fan: %d\n", asdu29->fan);
                    printf("                  not: %d\n", asdu29->not );
                    printf("                  tap: %d\n", asdu29->tap);
                    for (size_t i = 0; i < asdu29->dataSet.size(); ++i)
                    {
                        const auto& dataSet29 = asdu29->dataSet[i];
                        printf("                  dataSet[%03zu]: func[%d], inf[%d], dpi[%d]\n", i, dataSet29.func, dataSet29.inf,
                               dataSet29.dpi);
                    }
                }
            }
            break;
            case 0x1E: {
                auto asdu30 = std::static_pointer_cast<npacket::iec103::Asdu30>(frame->asdu);
                if (asdu30)
                {
                    printf("                  func: %d\n", asdu30->func);
                    printf("                  tov: %d\n", asdu30->tov);
                    printf("                  fan: %d\n", asdu30->fan);
                    printf("                  acc: %d\n", asdu30->acc);
                    printf("                  ndv: %d\n", asdu30->ndv);
                    printf("                  nfe: %d\n", asdu30->nfe);
                    printf("                  sdv: ");
                    for (size_t i = 0; i < asdu30->sdvList.size(); ++i)
                    {
                        if (0 != i)
                        {
                            printf(", ");
                        }
                        printf("%f", asdu30->sdvList[i]);
                    }
                    printf("\n");
                }
            }
            break;
            case 0x1F: {
                auto asdu31 = std::static_pointer_cast<npacket::iec103::Asdu31>(frame->asdu);
                if (asdu31)
                {
                    printf("                  func: %d\n", asdu31->func);
                    printf("                  too: %d\n", asdu31->too);
                    printf("                  tov: %d\n", asdu31->tov);
                    printf("                  fan: %d\n", asdu31->fan);
                    printf("                  acc: %d\n", asdu31->acc);
                }
            }
            break;
            case 0x26: {
                auto asdu38 = std::static_pointer_cast<npacket::iec103::Asdu38>(frame->asdu);
                if (asdu38)
                {
                    printf("                  func: %d\n", asdu38->func);
                    printf("                  inf: %d\n", asdu38->inf);
                    printf("                  vti: type[%d], value[%d]\n", asdu38->vti.type, asdu38->vti.value);
                    printf("                  qds: iv[%d], nt[%d], sb[%d], bl[%d], res[%d], ov[%d]\n", asdu38->qds.iv, asdu38->qds.nt,
                           asdu38->qds.sb, asdu38->qds.bl, asdu38->qds.res, asdu38->qds.ov);
                }
            }
            break;
            case 0x2A: {
                auto asdu42 = std::static_pointer_cast<npacket::iec103::Asdu42>(frame->asdu);
                if (asdu42)
                {
                    printf("                  func: %d\n", asdu42->func);
                    printf("                  inf: %d\n", asdu42->inf);
                    printf("                  dpi: ");
                    for (size_t i = 0; i < asdu42->dpiList.size(); ++i)
                    {
                        if (0 != i)
                        {
                            printf(", ");
                        }
                        printf("%d", asdu42->dpiList[i]);
                    }
                    printf("\n");
                    printf("                  sin: %d\n", asdu42->sin);
                }
            }
            break;
            }
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
