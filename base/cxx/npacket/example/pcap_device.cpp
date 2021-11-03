#include "pcap_device.h"

#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

/* On Mac OS X and FreeBSD timeout of -1 causes pcap_open_live to fail so value of 1ms is set here.
   On Linux and Windows this is not the case so we keep the -1 value */
#if defined(MAC_OS_X) || defined(FREEBSD)
#define LIBPCAP_OPEN_LIVE_TIMEOUT 1
#else
#define LIBPCAP_OPEN_LIVE_TIMEOUT -1
#endif

void PcapDevice::onPacketArrived(uint8_t* user, const struct pcap_pkthdr* pkthdr, const uint8_t* packet)
{
    PcapDevice* dev = (PcapDevice*)user;
    if (!dev)
    {
        return;
    }
    if (dev->m_onDataCallback)
    {
        dev->m_onDataCallback(packet, pkthdr->caplen);
    }
}

PcapDevice::~PcapDevice()
{
    close();
}

std::string PcapDevice::getName() const
{
    return m_name;
}

std::string PcapDevice::getDescribe() const
{
    return m_describe;
}

std::string PcapDevice::getIpv4Address() const
{
    return m_ipv4Address;
}

bool PcapDevice::isLoopback() const
{
    return m_isLoopback;
}

bool PcapDevice::open(int snapLen, int promisc, int timeout, int bufferSize)
{
    if (m_pcap)
    {
        return true;
    }
    char errbuf[PCAP_ERRBUF_SIZE] = {};
    m_pcap = pcap_create(m_name.c_str(), errbuf);
    if (!m_pcap)
    {
        return false;
    }
    pcap_set_snaplen(m_pcap, snapLen <= 0 ? 65536 : snapLen);
    pcap_set_promisc(m_pcap, promisc);
    pcap_set_timeout(m_pcap, timeout <= 0 ? LIBPCAP_OPEN_LIVE_TIMEOUT : timeout);
    if (bufferSize >= 100)
    {
        pcap_set_buffer_size(m_pcap, bufferSize);
    }
    if (0 != pcap_activate(m_pcap))
    {
        pcap_close(m_pcap);
        m_pcap = nullptr;
        return false;
    }
    return true;
}

void PcapDevice::setDataCallback(const std::function<void(const unsigned char* data, int dataLen)>& cb)
{
    m_onDataCallback = cb;
}

bool PcapDevice::startCapture()
{
    if (!m_pcap)
    {
        return false;
    }
    if (m_captureThread)
    {
        return true;
    }
    m_captureStarted = true;
    m_captureThread = new std::thread([&]() {
        while (m_pcap && m_captureStarted)
        {
            pcap_dispatch(m_pcap, -1, onPacketArrived, (u_char*)this);
        }
        m_captureStarted = false;
    });
    return true;
}

void PcapDevice::stopCapture()
{
    m_captureStarted = false;
    if (m_captureThread)
    {
        m_captureThread->join();
        m_captureThread = nullptr;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void PcapDevice::close()
{
    stopCapture();
    if (m_pcap)
    {
        pcap_close(m_pcap);
        m_pcap = nullptr;
    }
}

std::vector<std::shared_ptr<PcapDevice>> PcapDevice::getAllDevices(std::string* errorBuffer)
{
    if (errorBuffer)
    {
        errorBuffer->clear();
    }
    std::vector<std::shared_ptr<PcapDevice>> devList;
    pcap_if_t* allDevs = nullptr;
    char errbuf[PCAP_ERRBUF_SIZE];
    int err = pcap_findalldevs(&allDevs, errbuf);
    if (errbuf < 0)
    {
        if (errorBuffer)
        {
            *errorBuffer = errbuf;
        }
        return devList;
    }
    pcap_if_t* dev = allDevs;
    while (dev)
    {
        auto pd = std::make_shared<PcapDevice>();
        if (dev->name)
        {
            pd->m_name = dev->name;
        }
        if (dev->description)
        {
            pd->m_describe = dev->description;
        }
        pcap_addr* addr = dev->addresses;
        while (addr)
        {
            if (addr->addr)
            {
                if (AF_INET == addr->addr->sa_family)
                {
                    in_addr* ipv4Addr = &(((struct sockaddr_in*)addr->addr)->sin_addr);
                    if (ipv4Addr)
                    {
                        char addrString[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(((sockaddr_in*)addr->addr)->sin_addr), addrString, INET_ADDRSTRLEN);
                        pd->m_ipv4Address = addrString;
                    }
                }
                else
                {
                    char addrString[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, &(((sockaddr_in6*)addr->addr)->sin6_addr), addrString, INET6_ADDRSTRLEN);
                    // TODO: IPv6
                }
            }
            addr = addr->next;
        }
        if (PCAP_IF_LOOPBACK == (dev->flags & 0x1))
        {
            pd->m_isLoopback = true;
        }
        devList.emplace_back(pd);
        dev = dev->next;
    }
    pcap_freealldevs(allDevs);
    return devList;
}
