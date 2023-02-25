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
            printf("===== EthernetII\n");
            printf("src mac: %s:%s:%s:%s:%s:%s, dst mac: %s:%s:%s:%s:%s:%s\n", h->src_mac[0].c_str(), h->src_mac[1].c_str(),
                   h->src_mac[2].c_str(), h->src_mac[3].c_str(), h->src_mac[4].c_str(), h->src_mac[5].c_str(), h->dst_mac[0].c_str(),
                   h->dst_mac[1].c_str(), h->dst_mac[2].c_str(), h->dst_mac[3].c_str(), h->dst_mac[4].c_str(), h->dst_mac[5].c_str());
            printf("type: 0x%04x\n", h->next_protocol);
            return true;
        },
        [&](uint32_t totalLen, const std::shared_ptr<npacket::ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen) {
            switch ((npacket::NetworkProtocolType)header->getProtocolType())
            {
            case npacket::NetworkProtocolType::IPv4: {
                auto h = std::dynamic_pointer_cast<npacket::Ipv4Header>(header);
                printf("--- IPv4\n");
                printf("version: %d\n", h->version);
                printf("header_len: %d\n", h->header_len);
                printf("tos: %d\n", h->tos);
                printf("total_len: %d\n", h->total_len);
                printf("identification: 0x%04x (%d)\n", h->identification, h->identification);
                printf("flag_reserved: %d\n", h->flag_reserved);
                printf("flag_dont: %d\n", h->flag_dont);
                printf("flag_more: %d\n", h->flag_more);
                printf("frag_offset: %d\n", h->frag_offset);
                printf("ttl: %d\n", h->ttl);
                printf("protocol: %d\n", h->next_protocol);
                printf("checksum: 0x%04x\n", h->checksum);
                printf("src_addr: %s\n", h->src_addr.c_str());
                printf("dst_addr: %s\n", h->dst_addr.c_str());
            }
            break;
            case npacket::NetworkProtocolType::ARP: {
                auto h = std::dynamic_pointer_cast<npacket::ArpHeader>(header);
            }
            break;
            case npacket::NetworkProtocolType::RARP: {
            }
            break;
            case npacket::NetworkProtocolType::IPv6: {
                auto h = std::dynamic_pointer_cast<npacket::Ipv6Header>(header);
                printf("--- IPv6\n");
                printf("version: %d\n", h->version);
                printf("traffic_class: %d\n", h->traffic_class);
                printf("flow_label: %u\n", h->flow_label);
                printf("payload_len: %d\n", h->payload_len);
                printf("protocol: %d\n", h->next_protocol);
                printf("hop_limit: %d\n", h->hop_limit);
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
            switch ((npacket::TransportProtocolType)header->getProtocolType())
            {
            case npacket::TransportProtocolType::TCP: {
                auto h = std::dynamic_pointer_cast<npacket::TcpHeader>(header);
                printf("--- TCP\n");
                printf("src_port: %d\n", h->src_port);
                printf("dst_port: %d\n", h->dst_port);
                printf("seq: %u\n", h->seq);
                printf("ack: %u\n", h->ack);
                printf("header_len: %d\n", h->header_len);
                printf("flag_reserved: %d\n", h->flag_reserved);
                printf("flag_nonce: %d\n", h->flag_nonce);
                printf("flag_cwr: %d\n", h->flag_cwr);
                printf("flag_ecn_echo: %d\n", h->flag_ecn_echo);
                printf("flag_urgent: %d\n", h->flag_urgent);
                printf("flag_ack: %d\n", h->flag_ack);
                printf("flag_push: %d\n", h->flag_push);
                printf("flag_reset: %d\n", h->flag_reset);
                printf("flag_syn: %d\n", h->flag_syn);
                printf("flag_fin: %d\n", h->flag_fin);
                printf("window: %d\n", h->window);
                printf("checksum: 0x%04x\n", h->checksum);
                printf("urgptr: %d\n", h->urgptr);
            }
            break;
            case npacket::TransportProtocolType::UDP: {
                auto h = std::dynamic_pointer_cast<npacket::UdpHeader>(header);
                printf("--- UDP\n");
                printf("src_port: %d\n", h->src_port);
                printf("dst_port: %d\n", h->dst_port);
                printf("total_len: %d\n", h->total_len);
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
