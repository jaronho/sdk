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
#include "../npacket/proto/s7comm.h"
#include "../npacket/proto/tpkt_cotp.h"

static std::shared_ptr<npacket::Analyzer> s_pktAnalyzer = nullptr;
static std::shared_ptr<npacket::S7CommParser> s_s7commParser = nullptr;
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
bool handleEthernetLayer(const npacket::ProtocolData& pd)
{
    auto h = (const npacket::EthernetIIHeader*)(pd.header);
    if (s_proto.empty() || "ehternet" == s_proto)
    {
        printEthernet(h);
    }
    return true;
}

/* 处理网络层 */
bool handleNetworkLayer(const npacket::ProtocolData& pd)
{
    switch ((npacket::NetworkProtocol)pd.header->getProtocol())
    {
    case npacket::NetworkProtocol::IPv4: {
        auto h = (const npacket::Ipv4Header*)(pd.header);
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
        auto h = (const npacket::ArpHeader*)(pd.header);
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
        auto h = (const npacket::Ipv6Header*)(pd.header);
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
bool handleTransportLayer(const npacket::ProtocolData& pd)
{
    switch ((npacket::TransportProtocol)pd.header->getProtocol())
    {
    case npacket::TransportProtocol::TCP: {
        auto h = (const npacket::TcpHeader*)(pd.header);
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
        auto h = (const npacket::UdpHeader*)(pd.header);
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
        auto h = (const npacket::IcmpHeader*)(pd.header);
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
        auto h = (const npacket::Icmpv6Header*)(pd.header);
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

const char* getRosctrDesc(npacket::s7comm::RosctrType rosctr)
{
    switch (rosctr)
    {
    case npacket::s7comm::RosctrType::JOB:
        return "JOB";
    case npacket::s7comm::RosctrType::ACK:
        return "ACK";
    case npacket::s7comm::RosctrType::ACK_DATA:
        return "ACK_DATA";
    case npacket::s7comm::RosctrType::JOB_ACK:
        return "JOB_ACK";
    case npacket::s7comm::RosctrType::DATA:
        return "DATA";
    case npacket::s7comm::RosctrType::USERDATA:
        return "USERDATA";
    case npacket::s7comm::RosctrType::NEGOTIATE:
        return "NEGOTIATE";
    case npacket::s7comm::RosctrType::CONFIRM:
        return "CONFIRM";
    case npacket::s7comm::RosctrType::ABORT:
        return "ABORT";
    default:
        return "Unknown";
    }
}

const char* getFunctionCodeDesc(npacket::s7comm::FunctionCode funcCode)
{
    switch (funcCode)
    {
    case npacket::s7comm::FunctionCode::CPU_SERVICES:
        return "CPU_SERVICES";
    case npacket::s7comm::FunctionCode::READ_VARIABLE:
        return "READ_VARIABLE";
    case npacket::s7comm::FunctionCode::WRITE_VARIABLE:
        return "WRITE_VARIABLE";
    case npacket::s7comm::FunctionCode::REQUEST_DOWNLOAD:
        return "REQUEST_DOWNLOAD";
    case npacket::s7comm::FunctionCode::DOWNLOAD_BLOCK:
        return "DOWNLOAD_BLOCK";
    case npacket::s7comm::FunctionCode::DOWNLOAD_ENDED:
        return "DOWNLOAD_ENDED";
    case npacket::s7comm::FunctionCode::START_UPLOAD:
        return "START_UPLOAD";
    case npacket::s7comm::FunctionCode::UPLOAD_BLOCK:
        return "UPLOAD_BLOCK";
    case npacket::s7comm::FunctionCode::END_UPLOAD:
        return "END_UPLOAD";
    case npacket::s7comm::FunctionCode::PLC_CONTROL:
        return "PLC_CONTROL";
    case npacket::s7comm::FunctionCode::PI_SERVICE:
        return "PI_SERVICE";
    case npacket::s7comm::FunctionCode::PLC_STOP:
        return "PLC_STOP";
    case npacket::s7comm::FunctionCode::COPY_RAM_TO_ROM:
        return "COPY_RAM_TO_ROM";
    case npacket::s7comm::FunctionCode::COMPRESS:
        return "COMPRESS";
    case npacket::s7comm::FunctionCode::DELETE_BLOCK:
        return "DELETE_BLOCK";
    case npacket::s7comm::FunctionCode::REPLACE_BLOCK:
        return "REPLACE_BLOCK";
    case npacket::s7comm::FunctionCode::BLOCK_STATUS:
        return "BLOCK_STATUS";
    case npacket::s7comm::FunctionCode::SETUP_COMMUNICATION:
        return "SETUP_COMMUNICATION";
    default:
        return "Unknown";
    }
}

/* 处理应用层S7COMM帧 */
void handleApplicationS7CommFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                  const npacket::ProtocolHeader* header, const npacket::TpktInfo& tpktInfo,
                                  const npacket::CotpInfo& cotpInfo, const npacket::s7comm::S7CommInfo& s7commInfo)
{
    if (s_proto.empty() || "iec103" == s_proto)
    {
        printTransportHeaderTcp(header);
        printf("            ----- S7COMM -----\n");
        printf("            tpkt\n");
        printf("                version: %d\n", tpktInfo.version);
        printf("                reserved: %d\n", tpktInfo.reserved);
        printf("                length: %d\n", tpktInfo.length);
        if (npacket::CotpInfo::SHORT == cotpInfo.type)
        {
            printf("            cotp[3, short]\n");
            printf("                length: %d\n", cotpInfo.sinfo.length);
            printf("                pduType: 0x%02x\n", cotpInfo.sinfo.pduType);
            printf("                tpduNumber: 0x%02x\n", cotpInfo.sinfo.tpduNumber);
            printf("                lastDataUnit: %d\n", cotpInfo.sinfo.lastDataUnit);
        }
        else if (npacket::CotpInfo::LONG == cotpInfo.type)
        {
            printf("            cotp[18, long]\n");
            printf("                length: %d\n", cotpInfo.linfo.length);
            printf("                pduType: 0x%02x\n", cotpInfo.linfo.pduType);
            printf("                dstRef: 0x%04x\n", cotpInfo.linfo.dstRef);
            printf("                srcRef: 0x%04x\n", cotpInfo.linfo.srcRef);
            printf("                nClass: %d\n", cotpInfo.linfo.nClass);
            printf("                extendedFormats: %s\n", cotpInfo.linfo.extendedFormats > 0 ? "true" : "false");
            printf("                noExplicitFlowControl: %s\n", cotpInfo.linfo.noExplicitFlowControl > 0 ? "true" : "false");
            printf("                parameterCode: 0x%02x\n", cotpInfo.linfo.parameterCode);
            printf("                parameterLength: %d\n", cotpInfo.linfo.parameterLength);
        }
        else
        {
            return;
        }
        printf("            s7comm\n");
        printf("                Header\n");
        printf("                    protocolId: 0x%02x\n", s7commInfo.header.protocolId);
        printf("                    rosctr: %s (%d)\n", getRosctrDesc(s7commInfo.header.rosctr), (int)s7commInfo.header.rosctr);
        printf("                    redundancyIdentification: 0x%04x\n", s7commInfo.header.redundancyIdentification);
        printf("                    protocolDataUnitReference: %d\n", s7commInfo.header.protocolDataUnitReference);
        printf("                    parameterLength: %d\n", s7commInfo.header.parameterLength);
        printf("                    dataLength: %d\n", s7commInfo.header.dataLength);
        if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr)
        {
            printf("                    errorClass: 0x%02x\n", s7commInfo.header.errorClass);
            printf("                    errorCode: 0x%02x\n", s7commInfo.header.errorCode);
        }
        printf("                Parameter\n");
        printf("                    functionCode: %s (0x%02x)\n",
               getFunctionCodeDesc((npacket::s7comm::FunctionCode)s7commInfo.functionCode), s7commInfo.functionCode);
        switch ((npacket::s7comm::FunctionCode)s7commInfo.functionCode)
        {
        case npacket::s7comm::FunctionCode::CPU_SERVICES: {
            printf("                    CpuServiceParam\n");
            printf("                        functionCode: %s (0x%02x)\n", getFunctionCodeDesc(npacket::s7comm::FunctionCode::CPU_SERVICES),
                   s7commInfo.cpuServiceParam.functionCode);
            printf("                        itemCount: %d\n", s7commInfo.cpuServiceParam.itemCount);
            for (const auto& cpuItem : s7commInfo.cpuServiceParam.items)
            {
                printf("                    Item\n");
                printf("                        variableSpecification: 0x%02x\n", cpuItem.variableSpecification);
                printf("                        lengthOfFollowingAddressSpecification: %d\n",
                       cpuItem.lengthOfFollowingAddressSpecification);
                printf("                        syntaxId: 0x%02x\n", cpuItem.syntaxId);
                printf("                        type: %s (%d)\n",
                       cpuItem.type == 1 ? "Request" : (cpuItem.type == 2 ? "Response" : "Unknown"), cpuItem.type);
                printf("                        functionGroup: ");
                switch (cpuItem.functionGroup)
                {
                case 0x00:
                    printf("Mode transition");
                    break;
                case 0x01:
                    printf("Programmer commands");
                    break;
                case 0x02:
                    printf("Cyclic data");
                    break;
                case 0x03:
                    printf("Block functions");
                    break;
                case 0x04:
                    printf("CPU functions");
                    break;
                case 0x05:
                    printf("Security");
                    break;
                case 0x06:
                    printf("PBC");
                    break;
                case 0x07:
                    printf("Time functions");
                    break;
                default:
                    printf("Unknown");
                    break;
                }
                printf(" (%d)\n", cpuItem.functionGroup);
                printf("                        subFunction: ");
                if (0x00 == cpuItem.functionGroup)
                {
                    switch (cpuItem.subFunction)
                    {
                    case 0x00:
                        printf("Stop");
                        break;
                    case 0x01:
                        printf("Warm restart");
                        break;
                    case 0x02:
                        printf("Run");
                        break;
                    case 0x03:
                        printf("Hot restart");
                        break;
                    case 0x04:
                        printf("Cold restart");
                        break;
                    default:
                        printf("Unknown");
                        break;
                    }
                }
                else if (0x03 == cpuItem.functionGroup)
                {
                    switch (cpuItem.subFunction)
                    {
                    case 0x01:
                        printf("List blocks");
                        break;
                    case 0x02:
                        printf("List blocks of type");
                        break;
                    case 0x03:
                        printf("Get block info");
                        break;
                    default:
                        printf("Unknown");
                        break;
                    }
                }
                else if (0x04 == cpuItem.functionGroup)
                {
                    switch (cpuItem.subFunction)
                    {
                    case 0x01:
                        printf("Read SZL");
                        break;
                    case 0x02:
                        printf("Message service");
                        break;
                    case 0x03:
                        printf("Diagnostic message");
                        break;
                    case 0x04:
                        printf("ALARM");
                        break;
                    case 0x05:
                        printf("ALARM_8");
                        break;
                    case 0x06:
                        printf("ALARM_8P");
                        break;
                    case 0x07:
                        printf("NOTIFY");
                        break;
                    case 0x08:
                        printf("AR_SEND");
                        break;
                    case 0x09:
                    case 0x0f:
                        printf("Reserved");
                        break;
                    case 0x10:
                        printf("Time functions");
                        break;
                    case 0x11:
                        printf("Time functions Set");
                        break;
                    case 0x12:
                        printf("Read clock");
                        break;
                    case 0x13:
                        printf("Set clock");
                        break;
                    case 0x14:
                        printf("Communication status");
                        break;
                    case 0x15:
                        printf("Block status");
                        break;
                    case 0x16:
                        printf("CPU status");
                        break;
                    default:
                        printf("Unknown");
                        break;
                    }
                }
                else if (0x07 == cpuItem.functionGroup)
                {
                    switch (cpuItem.subFunction)
                    {
                    case 0x01:
                        printf("Read clock");
                        break;
                    case 0x02:
                        printf("Set clock");
                        break;
                    default:
                        printf("Unknown");
                        break;
                    }
                }
                printf(" (%d)\n", cpuItem.subFunction);
                printf("                        sequenceNumber: %d\n", cpuItem.sequenceNumber);
                if (0x12 == cpuItem.syntaxId)
                {
                    printf("                        dataUnitReferenceNumber: %d\n", cpuItem.dataUnitReferenceNumber);
                    printf("                        lastDataUnit: %d\n", cpuItem.lastDataUnit);
                    printf("                        errorCode: 0x%04x\n", cpuItem.errorCode);
                }
            }
            if ((npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr
                 || npacket::s7comm::RosctrType::USERDATA == s7commInfo.header.rosctr)
                && s7commInfo.header.dataLength > 0)
            {
                printf("                    CpuServiceData\n");
                printf("                        returnCode: 0x%02x\n", s7commInfo.cpuServiceData.returnCode);
                printf("                        transportSize: 0x%02x\n", s7commInfo.cpuServiceData.transportSize);
                printf("                        length: %d\n", s7commInfo.cpuServiceData.length);
                if (s7commInfo.cpuServiceData.hasBlockList)
                {
                    const auto& bl = s7commInfo.cpuServiceData.blockList;
                    for (size_t i = 0; i < bl.size(); ++i)
                    {
                        printf("                        Item [%d]\n", i + 1);
                        printf("                            block type: 0x%02X\n", bl[i].blockType);
                        printf("                            block count: %d\n", bl[i].blockCount);
                    }
                }
                if (s7commInfo.cpuServiceData.hasBlockType)
                {
                    if (s7commInfo.cpuServiceData.blockListOfType.isRequest)
                    {
                        printf("                        block type: 0x%02X\n", s7commInfo.cpuServiceData.blockListOfType.request.blockType);
                    }
                    else
                    {
                        const auto& itemList = s7commInfo.cpuServiceData.blockListOfType.response.items;
                        for (size_t i = 0; i < itemList.size(); ++i)
                        {
                            printf("                        Item [%d]\n", i + 1);
                            printf("                            block number: %d\n", itemList[i].blockNumber);
                            printf("                            block flags: 0x%02x\n", itemList[i].blockFlags);
                            printf("                            block language: 0x%02X\n", itemList[i].blockLanguage);
                        }
                    }
                }
                if (s7commInfo.cpuServiceData.hasBlockInfo)
                {
                    printf("                    Block Data\n");
                    if (s7commInfo.cpuServiceData.blockInfo.isRequest)
                    {
                        const auto& bi = s7commInfo.cpuServiceData.blockInfo.request;
                        printf("                        block type: 0x%02X\n", bi.blockType);
                        printf("                        block number: %d\n", bi.blockNumber);
                        printf("                        filesystem: %c\n", bi.fileSystem);
                    }
                    else
                    {
                        const auto& bi = s7commInfo.cpuServiceData.blockInfo.response;
                        printf("                        block type: 0x%04X\n", bi.blockType);
                        printf("                        length of info: %d\n", bi.lengthOfInfo);
                        printf("                        unknown blockinfo2: 0x%04x\n", bi.unknownBlockinfo2);
                        printf("                        constant 3: 0x%04X (\"%c%c\")\n", bi.constant3, (bi.constant3 >> 8) & 0xFF,
                               bi.constant3 & 0xFF);
                        printf("                        unknown Byte1: 0x%02x\n", bi.unknownByte1);
                        printf("                        block flags: 0x%02x\n", bi.rawBlockFlags);
                        printf("                            Linked: %d\n", bi.blockFlagsLinked);
                        printf("                            Standard block: %d\n", bi.blockFlagsStandardBlock);
                        printf("                            Non Retain: %d\n", bi.blockFlagsNonRetain);
                        printf("                        block language: 0x%02X\n", bi.blockLanguage);
                        printf("                        subblk type: %d\n", bi.subblkType);
                        printf("                        block number: %d\n", bi.blockNumber);
                        printf("                        length load memory: %d\n", bi.lengthLoadMemory);
                        printf("                        block security: %d\n", bi.blockSecurity);
                        printf("                        code timestamp: ");
                        for (int i = 0; i < 6; ++i)
                        {
                            printf("%02x ", bi.codeTimestamp[i]);
                        }
                        printf("\n");
                        printf("                        interface timestamp: ");
                        for (int i = 0; i < 6; ++i)
                        {
                            printf("%02x ", bi.interfaceTimestamp[i]);
                        }
                        printf("\n");
                        printf("                        SSB length: %d\n", bi.ssbLength);
                        printf("                        ADD length: %d\n", bi.addLength);
                        printf("                        Localdata length: %d\n", bi.localdataLength);
                        printf("                        MC7 code length: %d\n", bi.mc7CodeLength);
                        printf("                        author: %.8s\n", bi.author);
                        printf("                        family: %.8s\n", bi.family);
                        printf("                        name: %.8s\n", bi.name);
                        printf("                        version: %d.%d\n", bi.versionMajor, bi.versionMinor);
                        printf("                        unknown byte2: 0x%02x\n", bi.unknownByte2);
                        printf("                        block checksum: 0x%04x\n", bi.blockChecksum);
                        printf("                        reserved 1: 0x%08x\n", bi.reserved1);
                        printf("                        reserved 2: 0x%08x\n", bi.reserved2);
                    }
                }
                if (s7commInfo.cpuServiceData.hasSzl)
                {
                    const auto& hdr = s7commInfo.cpuServiceData.szlHeader;
                    printf("                    SZL Data\n");
                    printf("                        SZL-ID: 0x%04x\n", hdr.szlId.rawId);
                    printf("                            diagnostic type: %d\n", hdr.szlId.diagnosticType);
                    printf("                            extract number: %d\n", hdr.szlId.extractNumber);
                    printf("                            partial list ID: 0x%02x\n", hdr.szlId.partialListId);
                    printf("                        SZL-Index: 0x%04x\n", hdr.szlIndex);
                    if (hdr.listCount > 0)
                    {
                        printf("                        list length in bytes: %d\n", hdr.listLength);
                        printf("                        list count: %d\n", hdr.listCount);
                        for (size_t i = 0; i < s7commInfo.cpuServiceData.szlDatas.size(); ++i)
                        {
                            const auto& rec = s7commInfo.cpuServiceData.szlDatas[i];
                            printf("                        SZL data (list count no. %zu)\n", i + 1);
                            printf("                            (%zu bytes): ", rec.length);
                            for (uint16_t n = 0; n < rec.length; ++n)
                            {
                                printf("%02x ", rec.data[n]);
                            }
                            printf("\n");
                        }
                    }
                }
                if (s7commInfo.cpuServiceData.hasMessageService)
                {
                    const auto& msrv = s7commInfo.cpuServiceData.msgService;
                    printf("                    Message Service\n");
                    if (msrv.isRequest)
                    {
                        printf("                        [subscribed events] mode-transition: %d\n", msrv.request.modeTransition);
                        printf("                        [subscribed events] system-diagnostics: %d\n", msrv.request.systemDiagnostics);
                        printf("                        [subscribed events] userdefined: %d\n", msrv.request.userDefined);
                        printf("                        [subscribed events] alarms: %d\n", msrv.request.alarms);
                        printf("                        reserved: 0x%02x\n", msrv.request.reserved);
                        printf("                        username: %s\n", msrv.request.username.c_str());
                    }
                    else
                    {
                        printf("                        result: 0x%02x\n", msrv.response.result);
                        printf("                        reserved: 0x%02x\n", msrv.response.reserved);
                    }
                }
                if (s7commInfo.cpuServiceData.hasTimestamp)
                {
                    const auto& ts = s7commInfo.cpuServiceData.timestamp;
                    printf("                    S7 Timestamp\n");
                    printf("                        reserved: 0x%02x\n", ts.reserved);
                    printf("                        year 1: %d\n", ts.year1);
                    printf("                        year 2: %d\n", ts.year2);
                    printf("                        month: %d\n", ts.month);
                    printf("                        day: %d\n", ts.day);
                    printf("                        hour: %d\n", ts.hour);
                    printf("                        minute: %d\n", ts.minute);
                    printf("                        second: %d\n", ts.second);
                    printf("                        milliseconds: %d\n", ts.milliseconds);
                    printf("                        weekday: %d\n", ts.weekday);
                }
            }
        }
        break;
        case npacket::s7comm::FunctionCode::READ_VARIABLE:
        case npacket::s7comm::FunctionCode::WRITE_VARIABLE: {
            printf("                    ReadWriteParam\n");
            printf("                        functionCode: %s (0x%02x)\n",
                   getFunctionCodeDesc((npacket::s7comm::FunctionCode)s7commInfo.rwParam.functionCode), s7commInfo.rwParam.functionCode);
            printf("                        itemCount: %d\n", s7commInfo.rwParam.itemCount);
            for (const auto& rwItem : s7commInfo.rwParam.items)
            {
                printf("                        Item\n");
                printf("                            variableSpecification: 0x%02x\n", rwItem.variableSpecification);
                printf("                            addressLength: %d\n", rwItem.addressLength);
                printf("                            syntaxId: 0x%02x\n", rwItem.syntaxId);
                printf("                            transportSize: 0x%02x\n", rwItem.transportSize);
                printf("                            length: %d\n", rwItem.length);
                printf("                            dbNumber: %d\n", rwItem.dbNumber);
                printf("                            area: 0x%02x\n", rwItem.area);
                printf("                            address: 0x%06x\n", rwItem.address);
            }
        }
        break;
        case npacket::s7comm::FunctionCode::REQUEST_DOWNLOAD: {
            printf("                    RequestDownloadParam\n");
            printf("                        functionCode: %s (0x%02x)\n",
                   getFunctionCodeDesc(npacket::s7comm::FunctionCode::REQUEST_DOWNLOAD), s7commInfo.requestDownloadParam.functionCode);
            if (s7commInfo.header.parameterLength > 1)
            {
                printf("                        functionStatus.moreDataFollowing: %d\n",
                       s7commInfo.requestDownloadParam.functionStatus.moreDataFollowing);
                printf("                        functionStatus.error: %d\n", s7commInfo.requestDownloadParam.functionStatus.error);
                if (s7commInfo.header.parameterLength > 8)
                {
                    printf("                        unknownByteInBlockControl1: 0x%04x\n",
                           s7commInfo.requestDownloadParam.unknownByteInBlockControl1);
                    printf("                        unknownByteInBlockControl2: 0x%08x\n",
                           s7commInfo.requestDownloadParam.unknownByteInBlockControl2);
                    if (s7commInfo.downloadBlockParam.filenameLen > 0)
                    {
                        printf("                        filenameLen: %d\n", s7commInfo.requestDownloadParam.filenameLen);
                        printf("                        filename.fileIdentifier: %c\n",
                               s7commInfo.requestDownloadParam.filename.fileIdentifier);
                        printf("                        filename.blockType: %c%c\n", s7commInfo.requestDownloadParam.filename.blockType[0],
                               s7commInfo.requestDownloadParam.filename.blockType[1]);
                        printf("        filename.blockNumber: %c%c%c%c%c\n", s7commInfo.requestDownloadParam.filename.blockNumber[0],
                               s7commInfo.requestDownloadParam.filename.blockNumber[1],
                               s7commInfo.requestDownloadParam.filename.blockNumber[2],
                               s7commInfo.requestDownloadParam.filename.blockNumber[3],
                               s7commInfo.requestDownloadParam.filename.blockNumber[4]);
                        printf("                        filename.destinationFilesystem: %c\n",
                               s7commInfo.requestDownloadParam.filename.destinationFilesystem);
                    }
                    printf("                        lengthPart2: %d\n", s7commInfo.requestDownloadParam.lengthPart2);
                    printf("                        unknownCharBeforeLoadMem: %c\n",
                           s7commInfo.requestDownloadParam.unknownCharBeforeLoadMem);
                    printf("                        lengthOfLoadMemory: %c%c%c%c%c%c\n",
                           s7commInfo.requestDownloadParam.lengthOfLoadMemory[0], s7commInfo.requestDownloadParam.lengthOfLoadMemory[1],
                           s7commInfo.requestDownloadParam.lengthOfLoadMemory[2], s7commInfo.requestDownloadParam.lengthOfLoadMemory[3],
                           s7commInfo.requestDownloadParam.lengthOfLoadMemory[4], s7commInfo.requestDownloadParam.lengthOfLoadMemory[5]);
                    printf("                        lengthOfMc7Code: %c%c%c%c%c%c\n", s7commInfo.requestDownloadParam.lengthOfMc7Code[0],
                           s7commInfo.requestDownloadParam.lengthOfMc7Code[1], s7commInfo.requestDownloadParam.lengthOfMc7Code[2],
                           s7commInfo.requestDownloadParam.lengthOfMc7Code[3], s7commInfo.requestDownloadParam.lengthOfMc7Code[4],
                           s7commInfo.requestDownloadParam.lengthOfMc7Code[5]);
                }
            }
        }
        break;
        case npacket::s7comm::FunctionCode::DOWNLOAD_BLOCK: {
            printf("                    DownloadBlockParam\n");
            printf("                        functionCode: %s (0x%02x)\n",
                   getFunctionCodeDesc(npacket::s7comm::FunctionCode::DOWNLOAD_BLOCK), s7commInfo.downloadBlockParam.functionCode);
            if (s7commInfo.header.parameterLength > 1)
            {
                printf("                        functionStatus.moreDataFollowing: %d\n",
                       s7commInfo.downloadBlockParam.functionStatus.moreDataFollowing);
                printf("                        functionStatus.error: %d\n", s7commInfo.downloadBlockParam.functionStatus.error);
                if (s7commInfo.header.parameterLength > 8)
                {
                    printf("                        unknownByteInBlockControl1: 0x%04x\n",
                           s7commInfo.downloadBlockParam.unknownByteInBlockControl1);
                    printf("                        unknownByteInBlockControl2: 0x%08x\n",
                           s7commInfo.downloadBlockParam.unknownByteInBlockControl2);
                    if (s7commInfo.downloadBlockParam.filenameLen > 0)
                    {
                        printf("                        filenameLen: %d\n", s7commInfo.downloadBlockParam.filenameLen);
                        printf("                        filename.fileIdentifier: %c\n",
                               s7commInfo.downloadBlockParam.filename.fileIdentifier);
                        printf("                        filename.blockType: %c%c\n", s7commInfo.downloadBlockParam.filename.blockType[0],
                               s7commInfo.downloadBlockParam.filename.blockType[1]);
                        printf("                        filename.blockNumber: %c%c%c%c%c\n",
                               s7commInfo.downloadBlockParam.filename.blockNumber[0], s7commInfo.downloadBlockParam.filename.blockNumber[1],
                               s7commInfo.downloadBlockParam.filename.blockNumber[2], s7commInfo.downloadBlockParam.filename.blockNumber[3],
                               s7commInfo.downloadBlockParam.filename.blockNumber[4]);
                        printf("                        filename.destinationFilesystem: %c\n",
                               s7commInfo.downloadBlockParam.filename.destinationFilesystem);
                    }
                }
            }
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    DownloadBlockData\n");
                printf("                        length: %d\n", s7commInfo.downloadBlockData.length);
                printf("                        unknownByteInBlockControl: 0x%04x\n",
                       s7commInfo.downloadBlockData.unknownByteInBlockControl);
                if (s7commInfo.downloadBlockData.data && s7commInfo.downloadBlockData.length > 0)
                {
                    printf("                        data: ");
                    for (uint16_t i = 0; i < s7commInfo.downloadBlockData.length; ++i)
                    {
                        printf("%02x ", s7commInfo.downloadBlockData.data[i]);
                    }
                    printf("\n");
                }
            }
        }
        break;
        case npacket::s7comm::FunctionCode::DOWNLOAD_ENDED: {
            printf("                    downloadEndedParam\n");
            printf("                        functionCode: %s (0x%02x)\n",
                   getFunctionCodeDesc(npacket::s7comm::FunctionCode::DOWNLOAD_ENDED), s7commInfo.downloadEndedParam.functionCode);
            if (s7commInfo.header.parameterLength > 1)
            {
                printf("                        functionStatus.moreDataFollowing: %d\n",
                       s7commInfo.downloadEndedParam.functionStatus.moreDataFollowing);
                printf("                        functionStatus.error: %d\n", s7commInfo.downloadEndedParam.functionStatus.error);
                if (s7commInfo.header.parameterLength > 8)
                {
                    printf("                        errorCode: 0x%04x\n", s7commInfo.downloadEndedParam.errorCode);
                    printf("                        unknownByteInBlockControl: 0x%08x\n",
                           s7commInfo.downloadEndedParam.unknownByteInBlockControl);
                }
            }
            if (s7commInfo.downloadEndedParam.filenameLen > 0)
            {
                printf("                        filenameLen: %d\n", s7commInfo.downloadEndedParam.filenameLen);
                printf("                        filename.fileIdentifier: %c\n", s7commInfo.downloadEndedParam.filename.fileIdentifier);
                printf("                        filename.blockType: %c%c\n", s7commInfo.downloadEndedParam.filename.blockType[0],
                       s7commInfo.downloadEndedParam.filename.blockType[1]);
                printf("                        filename.blockNumber: %c%c%c%c%c\n", s7commInfo.downloadEndedParam.filename.blockNumber[0],
                       s7commInfo.downloadEndedParam.filename.blockNumber[1], s7commInfo.downloadEndedParam.filename.blockNumber[2],
                       s7commInfo.downloadEndedParam.filename.blockNumber[3], s7commInfo.downloadEndedParam.filename.blockNumber[4]);
                printf("                        filename.destinationFilesystem: %c\n",
                       s7commInfo.downloadEndedParam.filename.destinationFilesystem);
            }
        }
        break;
        case npacket::s7comm::FunctionCode::START_UPLOAD: {
            printf("                    StartUploadParam\n");
            printf("                        functionCode: %s (0x%02x)\n", getFunctionCodeDesc(npacket::s7comm::FunctionCode::START_UPLOAD),
                   s7commInfo.startUploadParam.functionCode);
            printf("                        blockTypeLen: %d\n", s7commInfo.startUploadParam.blockTypeLen);
            printf("                        blockNumLen: %d\n", s7commInfo.startUploadParam.blockNumLen);
            printf("                        fileSystemLen: %d\n", s7commInfo.startUploadParam.fileSystemLen);
            printf("                        blockType: 0x%04x\n", s7commInfo.startUploadParam.blockType);
            printf("                        blockNumber: %s\n", s7commInfo.startUploadParam.blockNumber);
            printf("                        fileSystem: %c\n", s7commInfo.startUploadParam.fileSystem);
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    StartUploadData\n");
                printf("                        uploadId: 0x%08x\n", s7commInfo.startUploadData.uploadId);
                printf("                        status: %d\n", s7commInfo.startUploadData.status);
            }
        }
        break;
        case npacket::s7comm::FunctionCode::UPLOAD_BLOCK: {
            printf("                    UploadParam\n");
            printf("                        functionCode: %s (0x%02x)\n", getFunctionCodeDesc(npacket::s7comm::FunctionCode::UPLOAD_BLOCK),
                   s7commInfo.uploadParam.functionCode);
            printf("                        uploadId: 0x%08x\n", s7commInfo.uploadParam.uploadId);
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    UploadData\n");
                printf("                        dataOffset: %d\n", s7commInfo.uploadData.dataOffset);
                printf("                        dataLen: %d\n", s7commInfo.uploadData.dataLen);
                printf("                        isLastBlock: %s\n", s7commInfo.uploadData.isLastBlock ? "true" : "false");
                if (s7commInfo.uploadData.blockData && s7commInfo.uploadData.dataLen > 0)
                {
                    printf("                        blockData: ");
                    for (uint16_t i = 0; i < s7commInfo.uploadData.dataLen; ++i)
                    {
                        printf("%02x ", s7commInfo.uploadData.blockData[i]);
                    }
                    printf("\n");
                }
            }
        }
        break;
        case npacket::s7comm::FunctionCode::END_UPLOAD: {
            printf("                    EndUploadParam\n");
            printf("                        functionCode: %s (0x%02x)\n", getFunctionCodeDesc(npacket::s7comm::FunctionCode::END_UPLOAD),
                   s7commInfo.endUploadParam.functionCode);
            printf("                        uploadId: 0x%08x\n", s7commInfo.endUploadParam.uploadId);
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    EndUploadData\n");
                printf("                        returnCode: 0x%02x\n", s7commInfo.endUploadData.returnCode);
                printf("                        errorClass: 0x%02x\n", s7commInfo.endUploadData.errorClass);
                printf("                        errorCode: 0x%02x\n", s7commInfo.endUploadData.errorCode);
            }
        }
        break;
        case npacket::s7comm::FunctionCode::PLC_CONTROL: {
            printf("                    PlcControlParam\n");
            printf("                        functionCode: %s (0x%02x)\n", getFunctionCodeDesc(npacket::s7comm::FunctionCode::PLC_CONTROL),
                   s7commInfo.plcCtrlParam.functionCode);
            printf("                        paramCount: %d\n", s7commInfo.plcCtrlParam.paramCount);
            for (const auto& str : s7commInfo.plcCtrlParam.params)
            {
                printf("                        param: %s\n", str.c_str());
            }
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    PlcControlData\n");
                printf("                        returnCode: 0x%02x\n", s7commInfo.plcCtrlData.returnCode);
                if (!s7commInfo.plcCtrlData.statusMsg.empty())
                {
                    printf("                        statusMsg: %s\n", s7commInfo.plcCtrlData.statusMsg.c_str());
                }
            }
        }
        break;
        case npacket::s7comm::FunctionCode::PI_SERVICE: {
            printf("                    PiServiceParam\n");
            printf("                        functionCode: %s (0x%02x)\n", getFunctionCodeDesc(npacket::s7comm::FunctionCode::PI_SERVICE),
                   s7commInfo.piParam.functionCode);
            printf("                        unknownBytes: 0x%016llx\n", (unsigned long long)s7commInfo.piParam.unknownBytes);
            if (s7commInfo.piParam.parameterBlockLength > 0)
            {
                printf("                        parameterBlockLength: %d\n", s7commInfo.piParam.parameterBlockLength);
                printf("                        parameterBlock.numberOfBlocks: %d\n", s7commInfo.piParam.parameterBlock.numberOfBlocks);
                printf("                        parameterBlock.unknownByte: 0x%02x\n", s7commInfo.piParam.parameterBlock.unknownByte);
                printf("                        parameterBlock.filename.blockType: %c%c\n",
                       s7commInfo.piParam.parameterBlock.filename.blockType[0], s7commInfo.piParam.parameterBlock.filename.blockType[1]);
                printf("                        parameterBlock.filename.blockNumber: %c%c%c%c%c\n",
                       s7commInfo.piParam.parameterBlock.filename.blockNumber[0], s7commInfo.piParam.parameterBlock.filename.blockNumber[1],
                       s7commInfo.piParam.parameterBlock.filename.blockNumber[2], s7commInfo.piParam.parameterBlock.filename.blockNumber[3],
                       s7commInfo.piParam.parameterBlock.filename.blockNumber[4]);
                printf("                        parameterBlock.filename.destinationFilesystem: %c\n",
                       s7commInfo.piParam.parameterBlock.filename.destinationFilesystem);
            }
            if (s7commInfo.piParam.serviceNameLength > 0)
            {
                printf("                        serviceNameLength: %d\n", s7commInfo.piParam.serviceNameLength);
                printf("                        serviceName: %s\n", s7commInfo.piParam.serviceName);
            }
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    PiServiceData\n");
                printf("                        returnCode: 0x%02x\n", s7commInfo.piData.returnCode);
                printf("                        dataLen: %d\n", s7commInfo.piData.dataLen);
                if (s7commInfo.piData.responseData && s7commInfo.piData.dataLen > 0)
                {
                    printf("                        responseData: ");
                    for (uint16_t i = 0; i < s7commInfo.piData.dataLen; ++i)
                    {
                        printf("%02x ", s7commInfo.piData.responseData[i]);
                    }
                    printf("\n");
                }
            }
        }
        break;
        case npacket::s7comm::FunctionCode::PLC_STOP: {
            printf("                    PlcStopParam\n");
            printf("                        functionCode: %s (0x%02x)\n", getFunctionCodeDesc(npacket::s7comm::FunctionCode::PLC_STOP),
                   s7commInfo.plcStopParam.functionCode);
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    PlcStopData\n");
                printf("                        returnCode: 0x%02x\n", s7commInfo.plcStopData.returnCode);
                printf("                        stopStatus: %d\n", s7commInfo.plcStopData.stopStatus);
            }
        }
        break;
        case npacket::s7comm::FunctionCode::COPY_RAM_TO_ROM: {
            printf("                    CopyRamToRomParam\n");
            printf("                        functionCode: %s (0x%02x)\n",
                   getFunctionCodeDesc(npacket::s7comm::FunctionCode::COPY_RAM_TO_ROM), s7commInfo.copyRamToRomParam.functionCode);
            printf("                        paramCount: %d\n", s7commInfo.copyRamToRomParam.paramCount);
            printf("                        blockType: 0x%04x\n", s7commInfo.copyRamToRomParam.blockType);
            printf("                        blockNumber: %s\n", s7commInfo.copyRamToRomParam.blockNumber);
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    CopyRamToRomData\n");
                printf("                        returnCode: 0x%02x\n", s7commInfo.copyRamToRomData.returnCode);
                printf("                        copyStatus: %d\n", s7commInfo.copyRamToRomData.copyStatus);
            }
        }
        break;
        case npacket::s7comm::FunctionCode::COMPRESS: {
            printf("                    CompressParam\n");
            printf("                        functionCode: %s (0x%02x)\n", getFunctionCodeDesc(npacket::s7comm::FunctionCode::COMPRESS),
                   s7commInfo.compressParam.functionCode);
            printf("                        memoryType: %d\n", s7commInfo.compressParam.memoryType);
            printf("                        reserved: 0x%02x\n", s7commInfo.compressParam.reserved);
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    CompressData\n");
                printf("                        returnCode: 0x%02x\n", s7commInfo.compressData.returnCode);
                printf("                        compressStatus: %d\n", s7commInfo.compressData.compressStatus);
                printf("                        freedBytes: %u\n", s7commInfo.compressData.freedBytes);
            }
        }
        break;
        case npacket::s7comm::FunctionCode::DELETE_BLOCK:
        case npacket::s7comm::FunctionCode::REPLACE_BLOCK:
        case npacket::s7comm::FunctionCode::BLOCK_STATUS: {
            printf("                    BlockOperationParam\n");
            printf("                        functionCode: %s (0x%02x)\n",
                   getFunctionCodeDesc((npacket::s7comm::FunctionCode)s7commInfo.blockOpParam.functionCode),
                   s7commInfo.blockOpParam.functionCode);
            printf("                        paramCount: %d\n", s7commInfo.blockOpParam.paramCount);
            printf("                        blockType: 0x%04x\n", s7commInfo.blockOpParam.blockType);
            printf("                        blockNumber: %s\n", s7commInfo.blockOpParam.blockNumber);
            printf("                        fileSystem: %c\n", s7commInfo.blockOpParam.fileSystem);
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    BlockOperationData\n");
                printf("                        returnCode: 0x%02x\n", s7commInfo.blockOpData.returnCode);
                printf("                        blockStatus: %d\n", s7commInfo.blockOpData.blockStatus);
                printf("                        errorClass: 0x%02x\n", s7commInfo.blockOpData.errorClass);
                printf("                        errorCode: 0x%02x\n", s7commInfo.blockOpData.errorCode);
            }
        }
        break;
        case npacket::s7comm::FunctionCode::SETUP_COMMUNICATION: {
            printf("                    SetupCommParam\n");
            printf("                        functionCode: %s (0x%02x)\n",
                   getFunctionCodeDesc(npacket::s7comm::FunctionCode::SETUP_COMMUNICATION), s7commInfo.setupCommParam.functionCode);
            printf("                        reserved: 0x%02x\n", s7commInfo.setupCommParam.reserved);
            printf("                        maxAmqCalling: %d\n", s7commInfo.setupCommParam.maxAmqCalling);
            printf("                        maxAmqCalled: %d\n", s7commInfo.setupCommParam.maxAmqCalled);
            printf("                        pduLength: %d\n", s7commInfo.setupCommParam.pduLength);
            if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr && s7commInfo.header.dataLength > 0)
            {
                printf("                    SetupCommData\n");
                printf("                        returnCode: 0x%02x\n", s7commInfo.setupCommData.returnCode);
                printf("                        maxAmqCaller: %d\n", s7commInfo.setupCommData.maxAmqCaller);
                printf("                        maxAmqCallee: %d\n", s7commInfo.setupCommData.maxAmqCallee);
                printf("                        pduLength: %d\n", s7commInfo.setupCommData.pduLength);
            }
        }
        break;
        }
        if (!s7commInfo.rwData.empty())
        {
            printf("                    Data\n");
            for (const auto& item : s7commInfo.rwData)
            {
                printf("                        Item\n");
                if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr
                    && npacket::s7comm::FunctionCode::WRITE_VARIABLE == (npacket::s7comm::FunctionCode)s7commInfo.functionCode)
                {
                    /* 写响应: 仅returnCode */
                    printf("                            returnCode: ");
                    switch (item.returnCode)
                    {
                    case 0xFF:
                        printf("Success");
                        break;
                    case 0x0A:
                        printf("Object does not exist");
                        break;
                    case 0x05:
                        printf("Invalid address");
                        break;
                    case 0x03:
                        printf("Access not allowed");
                        break;
                    case 0x06:
                        printf("Data type not supported");
                        break;
                    case 0x07:
                        printf("Data type inconsistent");
                        break;
                    case 0x01:
                        printf("Hardware error");
                        break;
                    case 0x81:
                        printf("Application error");
                        break;
                    default:
                        printf("Unknown");
                        break;
                    }
                    printf(" (0x%02x)\n", item.returnCode);
                    continue; /* 写响应无后续字段 */
                }
                else if (npacket::s7comm::RosctrType::ACK_DATA == s7commInfo.header.rosctr) /* 读响应或写请求: 有完整数据项格式 */
                {
                    printf("                            returnCode: ");
                    switch (item.returnCode)
                    {
                    case 0xFF:
                        printf("Success");
                        break;
                    case 0x0A:
                        printf("Object does not exist");
                        break;
                    case 0x05:
                        printf("Invalid address");
                        break;
                    case 0x03:
                        printf("Access not allowed");
                        break;
                    case 0x06:
                        printf("Data type not supported");
                        break;
                    case 0x07:
                        printf("Data type inconsistent");
                        break;
                    case 0x01:
                        printf("Hardware error");
                        break;
                    case 0x81:
                        printf("Application error");
                        break;
                    default:
                        printf("Unknown");
                        break;
                    }
                    printf(" (0x%02x)\n", item.returnCode);
                    if (0xFF != item.returnCode) /* 非成功项无有效数据 */
                    {
                        continue;
                    }
                }
                else /* 写请求: 无returnCode, 打印占位 */
                {
                    printf("                            returnCode: N/A (Write Request)\n");
                }
                printf("                            transportSize: ");
                switch (item.transportSize)
                {
                case 0x01:
                    printf("BIT");
                    break;
                case 0x02:
                    printf("BYTE");
                    break;
                case 0x03:
                    printf("BIT_ARRAY");
                    break;
                case 0x04:
                    printf("BYTE/WORD/DWORD");
                    break;
                case 0x05:
                    printf("INTEGER");
                    break;
                case 0x06:
                    printf("DWORD");
                    break;
                case 0x07:
                    printf("REAL");
                    break;
                case 0x09:
                    printf("OCTET STRING");
                    break;
                default:
                    printf("Unknown");
                    break;
                }
                printf(" (0x%02x)\n", item.transportSize);
                printf("                            length: %u (bytes)\n", item.length);
                if (item.length > 0 && item.data)
                {
                    printf("                            [rawData]: ");
                    for (uint16_t i = 0; i < item.length; ++i)
                    {
                        printf("%02x ", item.data[i]);
                    }
                    printf("\n");
                    if (0x01 == item.transportSize && 1 == item.length) /* BIT */
                    {
                        printf("                            BIT value: %d\n", item.data[0] & 0x01);
                    }
                    else if (0x04 == item.transportSize) /* BYTE/WORD/DWORD */
                    {
                        if (1 == item.length)
                        {
                            printf("                            BYTE value: 0x%02x\n", item.data[0]);
                        }
                        else if (2 == item.length)
                        {
                            uint16_t wValue = ((uint16_t)item.data[0] << 8) | item.data[1];
                            printf("                            WORD value: 0x%04x (%u)\n", wValue, wValue);
                        }
                        else if (4 == item.length)
                        {
                            uint32_t dwValue = ((uint32_t)item.data[0] << 24) | ((uint32_t)item.data[1] << 16)
                                               | ((uint32_t)item.data[2] << 8) | item.data[3];
                            printf("                            DWORD value: 0x%08x (%u)\n", dwValue, dwValue);
                        }
                    }
                    else if (0x05 == item.transportSize && 2 == item.length) /* INTEGER */
                    {
                        int16_t iValue = ((int16_t)item.data[0] << 8) | item.data[1];
                        printf("                            INTEGER value: %d\n", iValue);
                    }
                    else if (0x07 == item.transportSize && 4 == item.length) /* REAL */
                    {
                        float fValue = 0.0f;
                        memcpy(&fValue, item.data, sizeof(float));
                        printf("                            REAL value: %f\n", fValue);
                    }
                    else if (0x09 == item.transportSize && 10 == item.length) /* OCTET STRING: S7 Timestamp */
                    {
                        printf("                            S7 Timestamp - Reserved: 0x%02x\n", item.data[0]);
                        printf("                            S7 Timestamp - Year 1: %d\n", item.data[1]);
                        printf("                            S7 Timestamp - Year 2: %d\n", item.data[2]);
                        printf("                            S7 Timestamp - Month: %d\n", item.data[3]);
                        printf("                            S7 Timestamp - Day: %d\n", item.data[4]);
                        printf("                            S7 Timestamp - Hour: %d\n", item.data[5]);
                        printf("                            S7 Timestamp - Minute: %d\n", item.data[6]);
                        printf("                            S7 Timestamp - Second: %d\n", item.data[7]);
                        printf("                            S7 Timestamp - Milliseconds: %d\n", item.data[8]);
                        printf("                            S7 Timestamp - Weekday: %d\n", item.data[9] & 0xF);
                    }
                }
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
    {
        auto tpktCotpParser = std::make_shared<npacket::TpktCotpParser>();
        tpktCotpParser->setDataCallback([&](size_t flag, size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                            const npacket::ProtocolHeader* header, const npacket::TpktInfo& tpktInfo,
                                            const npacket::CotpInfo& cotpInfo, const uint8_t* payload, uint32_t payloadLen) {
            if (s_s7commParser && s_s7commParser->parse(flag, num, ntp, totalLen, header, tpktInfo, cotpInfo, payload, payloadLen))
            {
                return;
            }
            // TODO: MMS协议
        });
        s_pktAnalyzer->addProtocolParser(tpktCotpParser);
    }
    {
        s_s7commParser = std::make_shared<npacket::S7CommParser>();
        s_s7commParser->setFrameCallback(handleApplicationS7CommFrame);
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
        static size_t num = 0;
        ++num;
        auto ntp = std::chrono::steady_clock::now();
        s_pktAnalyzer->parse(0, num, ntp, data, dataLen);
        if (s_s7commParser) /* 定时清空S7COMM分片缓存 */
        {
            static std::chrono::steady_clock::time_point s_s7time = ntp;
            if (ntp - s_s7time >= std::chrono::seconds(1))
            {
                s_s7time = ntp;
                s_s7commParser->cleanupFragmentCache(ntp);
            }
        }
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
