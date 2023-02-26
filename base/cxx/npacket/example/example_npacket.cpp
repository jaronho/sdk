#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "../npacket/packet_analyzer.h"
#include "pcap_device.h"

static npacket::PacketAnalyzer s_pktAnalyzer;

int main(int argc, char* argv[])
{
    std::string name;
    std::string ip;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-name")) /* 本地设备名称 */
        {
            ++i;
            if (i < argc)
            {
                name = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-ip")) /* 本地设备IP */
        {
            ++i;
            if (i < argc)
            {
                ip = argv[i];
                ++i;
            }
        }
    }
    if (name.empty() && ip.empty())
    {
        printf("device name and ip is empty\n");
        return 0;
    }
    printf("device name: %s, ip: %s\n", name.c_str(), ip.c_str());
    s_pktAnalyzer.setLayerCallback(
        [&](uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen) {
            auto h = std::dynamic_pointer_cast<npacket::EthernetIIHeader>(header);
            printf("=============== EthernetII ===============\n");
            printf("src mac: %s, dst mac: %s\n", h->srcMacStr().c_str(), h->dstMacStr().c_str());
            printf("type: 0x%04x\n", h->nextProtocol);
            return true;
        },
        [&](uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen) {
            switch ((npacket::NetworkProtocol)header->getProtocol())
            {
            case npacket::NetworkProtocol::IPv4: {
                auto h = std::dynamic_pointer_cast<npacket::Ipv4Header>(header);
                printf("---------- IPv4 ----------\n");
                printf("version: %d, header len: %d, tos: %d, total len: %d\n", h->version, h->headerLen, h->tos, h->totalLen);
                printf("identification: 0x%04x (%d)\n", h->identification, h->identification);
                printf("reserved: %d, dont: %d, more: %d\n", h->flagRsrvd, h->flagDont, h->flagMore);
                printf("frag offset: %d\n", h->fragOffset);
                printf("ttl: %d\n", h->ttl);
                printf("protocol: %d\n", h->nextProtocol);
                printf("checksum: 0x%04x\n", h->checksum);
                printf("src addr: %s, dst addr: %s\n", h->srcAddr.c_str(), h->dstAddr.c_str());
            }
            break;
            case npacket::NetworkProtocol::ARP: {
                auto h = std::dynamic_pointer_cast<npacket::ArpHeader>(header);
                printf("---------- ARP ----------\n");
                printf("header len: %d\n", h->headerLen);
                printf("hardware type: 0x%04x, hardware size: %d\n", h->hardwareType, h->hardwareSize);
                printf("protocol type: 0x%04x, protocol size: %d\n", h->protocolType, h->protocolSize);
                printf("opcode: %d\n", h->opcode);
                printf("sender mac: %s, sender ip: %s\n", h->senderMacStr().c_str(), h->senderIp.c_str());
                printf("target mac: %s, target ip: %s\n", h->targetMacStr().c_str(), h->targetIp.c_str());
            }
            break;
            case npacket::NetworkProtocol::IPv6: {
                auto h = std::dynamic_pointer_cast<npacket::Ipv6Header>(header);
                printf("---------- IPv6 ----------\n");
                printf("version: %d, traffic class: %d, flow label: %u, payload len: %d\n", h->version, h->trafficClass, h->flowLabel,
                       h->payloadLen);
                printf("protocol: %d\n", h->nextProtocol);
                printf("hop limit: %d\n", h->hopLimit);
                printf("srcAddr: %s, dstAddr: %s\n", h->srcAddrStr().c_str(), h->dstAddrStr().c_str());
                if (0 == h->nextProtocol) /* 扩展包头: Hop-by-Hop */
                {
                }
            }
            break;
            }
            return true;
        },
        [&](uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen) {
            switch ((npacket::TransportProtocol)header->getProtocol())
            {
            case npacket::TransportProtocol::TCP: {
                auto h = std::dynamic_pointer_cast<npacket::TcpHeader>(header);
                printf("----- TCP -----\n");
                printf("src port: %d, dst port: %d\n", h->srcPort, h->dstPort);
                printf("seq: %u, ack: %u\n", h->seq, h->ack);
                printf("header len: %d\n", h->headerLen);
                printf("reserved: %d, nonce: %d, cwr: %d, ecn_echo: %d, urgent: %d, ack: %d, push: %d, reset: %d, syn: %d, fin: %d\n",
                       h->flagRsrvd, h->flagNonce, h->flagCwr, h->flagEce, h->flagUrg, h->flagAck, h->flagPsh, h->flagRst, h->flagSyn,
                       h->flagFin);
                printf("window: %d\n", h->window);
                printf("checksum: 0x%04x\n", h->checksum);
                printf("urgptr: %d\n", h->urgptr);
            }
            break;
            case npacket::TransportProtocol::UDP: {
                auto h = std::dynamic_pointer_cast<npacket::UdpHeader>(header);
                printf("----- UDP -----\n");
                printf("src port: %d, dst port: %d\n", h->srcPort, h->dstPort);
                printf("total len: %d\n", h->totalLen);
                printf("checksum: 0x%04x\n", h->checksum);
            }
            break;
            }
            return true;
        });
    std::shared_ptr<PcapDevice> dev;
    auto devList = PcapDevice::getAllDevices();
    for (size_t i = 0; i < devList.size(); ++i)
    {
        if (0 == name.compare(devList[i]->getName()) || 0 == ip.compare(devList[i]->getIpv4Address()))
        {
            dev = devList[i];
            break;
        }
    }
    if (!dev || !dev->open())
    {
        printf("device not found\n");
        return 0;
    }
    printf("device found, name: %s, ipv4: %s, describe: %s\n", dev->getName().c_str(), dev->getIpv4Address().c_str(),
           dev->getDescribe().c_str());
    printf("start caputre ...\n");
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
