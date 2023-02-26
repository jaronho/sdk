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
            printf("src mac: %s:%s:%s:%s:%s:%s, dst mac: %s:%s:%s:%s:%s:%s\n", h->src_mac[0].c_str(), h->src_mac[1].c_str(),
                   h->src_mac[2].c_str(), h->src_mac[3].c_str(), h->src_mac[4].c_str(), h->src_mac[5].c_str(), h->dst_mac[0].c_str(),
                   h->dst_mac[1].c_str(), h->dst_mac[2].c_str(), h->dst_mac[3].c_str(), h->dst_mac[4].c_str(), h->dst_mac[5].c_str());
            printf("type: 0x%04x\n", h->next_protocol);
            return true;
        },
        [&](uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen) {
            switch ((npacket::NetworkProtocol)header->getProtocol())
            {
            case npacket::NetworkProtocol::IPv4: {
                auto h = std::dynamic_pointer_cast<npacket::Ipv4Header>(header);
                printf("---------- IPv4 ----------\n");
                printf("version: %d, header len: %d, tos: %d, total len: %d\n", h->version, h->header_len, h->tos, h->total_len);
                printf("identification: 0x%04x (%d)\n", h->identification, h->identification);
                printf("reserved: %d, dont: %d, more: %d\n", h->flag_reserved, h->flag_dont, h->flag_more);
                printf("frag offset: %d\n", h->frag_offset);
                printf("ttl: %d\n", h->ttl);
                printf("protocol: %d\n", h->next_protocol);
                printf("checksum: 0x%04x\n", h->checksum);
                printf("src addr: %s, dst addr: %s\n", h->src_addr.c_str(), h->dst_addr.c_str());
            }
            break;
            case npacket::NetworkProtocol::ARP: {
                auto h = std::dynamic_pointer_cast<npacket::ArpHeader>(header);
                printf("---------- ARP ----------\n");
                printf("header len: %d\n", h->header_len);
                printf("hardware type: 0x%04x, hardware size: %d\n", h->hardware_type, h->hardware_size);
                printf("protocol type: 0x%04x, protocol size: %d\n", h->protocol_type, h->protocol_size);
                printf("opcode: %d\n", h->opcode);
                printf("sender mac: %s:%s:%s:%s:%s:%s, sender ip: %s\n", h->sender_mac[0].c_str(), h->sender_mac[1].c_str(),
                       h->sender_mac[2].c_str(), h->sender_mac[3].c_str(), h->sender_mac[4].c_str(), h->sender_mac[5].c_str(),
                       h->sender_ip.c_str());
                printf("target mac: %s:%s:%s:%s:%s:%s, target ip: %s\n", h->target_mac[0].c_str(), h->target_mac[1].c_str(),
                       h->target_mac[2].c_str(), h->target_mac[3].c_str(), h->target_mac[4].c_str(), h->target_mac[5].c_str(),
                       h->target_ip.c_str());
            }
            break;
            case npacket::NetworkProtocol::IPv6: {
                auto h = std::dynamic_pointer_cast<npacket::Ipv6Header>(header);
                printf("---------- IPv6 ----------\n");
                printf("version: %d, traffic class: %d, flow label: %u, payload len: %d\n", h->version, h->traffic_class, h->flow_label,
                       h->flow_label);
                printf("protocol: %d\n", h->next_protocol);
                printf("hop limit: %d\n", h->hop_limit);
                //printf("src_addr: %s\n", h->src_addr.c_str());
                //printf("dst_addr: %s\n", h->dst_addr.c_str());
                if (0 == h->next_protocol) /* 扩展包头: Hop-by-Hop */
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
                printf("src port: %d, dst port: %d\n", h->src_port, h->dst_port);
                printf("seq: %u, ack: %u\n", h->seq, h->ack);
                printf("header len: %d\n", h->header_len);
                printf("reserved: %d, nonce: %d, cwr: %d, ecn_echo: %d, urgent: %d, ack: %d, push: %d, reset: %d, syn: %d, fin: %d\n",
                       h->flag_reserved, h->flag_nonce, h->flag_cwr, h->flag_ecn_echo, h->flag_urgent, h->flag_ack, h->flag_push,
                       h->flag_reset, h->flag_syn, h->flag_fin);
                printf("window: %d\n", h->window);
                printf("checksum: 0x%04x\n", h->checksum);
                printf("urgptr: %d\n", h->urgptr);
            }
            break;
            case npacket::TransportProtocol::UDP: {
                auto h = std::dynamic_pointer_cast<npacket::UdpHeader>(header);
                printf("----- UDP -----\n");
                printf("src port: %d, dst port: %d\n", h->src_port, h->dst_port);
                printf("total len: %d\n", h->total_len);
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
