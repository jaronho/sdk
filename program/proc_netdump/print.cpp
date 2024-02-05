#include "print.h"

#include <stdio.h>

#include "filter.h"

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

void printTransportHeader(const std::shared_ptr<npacket::ProtocolHeader>& h)
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
    else if (npacket::NetworkProtocol::IPv6 == h->parent->getProtocol())
    {
        if (!Filter::getInstance().showIpv6())
        {
            printIPv6(std::dynamic_pointer_cast<npacket::Ipv6Header>(h->parent));
        }
    }
    if (npacket::TransportProtocol::TCP == h->getProtocol())
    {
        if (Filter::getInstance().showTcp())
        {
            printTCP(std::dynamic_pointer_cast<npacket::TcpHeader>(h));
        }
    }
    else if (npacket::TransportProtocol::UDP == h->getProtocol())
    {
        if (Filter::getInstance().showUdp())
        {
            printUDP(std::dynamic_pointer_cast<npacket::UdpHeader>(h));
        }
    }
    else if (npacket::TransportProtocol::ICMP == h->getProtocol())
    {
        if (Filter::getInstance().showIcmp())
        {
            printICMP(std::dynamic_pointer_cast<npacket::IcmpHeader>(h));
        }
    }
    else if (npacket::TransportProtocol::ICMPv6 == h->getProtocol())
    {
        if (Filter::getInstance().showIcmpv6())
        {
            printICMPv6(std::dynamic_pointer_cast<npacket::Icmpv6Header>(h));
        }
    }
}
