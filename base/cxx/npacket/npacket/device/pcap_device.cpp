#include "pcap_device.h"

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

namespace npacket
{
void PcapDevice::onPacketArrived(uint8_t* user, const struct pcap_pkthdr* pkthdr, const uint8_t* packet)
{
    PcapDevice* dev = (PcapDevice*)user;
    if (!dev)
    {
        return;
    }
    std::function<void(const unsigned char* data, unsigned int dataLen)> onDataCallback = nullptr;
    {
        std::lock_guard<std::mutex> locker(dev->m_mutexOnDataCallback);
        onDataCallback = dev->m_onDataCallback;
    }
    if (onDataCallback)
    {
        onDataCallback(packet, pkthdr->caplen);
    }
}

PcapDevice::~PcapDevice()
{
    close();
    {
        /* 等待执行结束, 保证句柄已在抓包线程侧安全关闭 */
        std::unique_lock<std::recursive_mutex> locker(m_mutex);
        m_cvDispatch.wait(locker, [this] { return !m_dispatching; });
    }
}

std::string PcapDevice::getName()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_name;
}

std::string PcapDevice::getDescribe()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_describe;
}

std::string PcapDevice::getIpv4Address()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_ipv4Address;
}

bool PcapDevice::isLoopback()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_isLoopback;
}

bool PcapDevice::open(const std::string& name, int direction, int snapLen, int promisc, int timeout, int bufferSize)
{
    if (name.empty())
    {
        return false;
    }
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_pcap)
    {
        return true;
    }
    char errbuf[PCAP_ERRBUF_SIZE] = {};
    pcap_t* pcap = pcap_create(name.c_str(), errbuf);
    if (!pcap)
    {
        return false;
    }
    pcap_set_snaplen(pcap, snapLen <= 0 ? 65536 : snapLen);
    pcap_set_promisc(pcap, promisc);
    pcap_set_timeout(pcap, timeout > 0 ? timeout : 1000);
    if (bufferSize >= 100)
    {
        pcap_set_buffer_size(pcap, bufferSize);
    }
    if (0 != pcap_activate(pcap))
    {
        pcap_close(pcap);
        return false;
    }
    direction = (direction < 0 || direction > 2) ? 0 : direction;
    pcap_setdirection(pcap, (pcap_direction_t)direction); /* 设置方向接口必须得打开后调用才会生效 */
    /* pcap_close作为删除器, 句柄最后一个引用释放时自动关闭(延迟关闭由引用计数保证) */
    m_pcap = std::shared_ptr<pcap_t>(pcap, pcap_close);
    m_name = name;
    return true;
}

bool PcapDevice::setFilter(const std::string& bpf, int optimize, int netmask)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (!m_pcap)
    {
        return false;
    }
    if (m_dispatching)
    {
        return false;
    }
    struct bpf_program filter;
    if (0 != pcap_compile(m_pcap.get(), &filter, bpf.c_str(), optimize, netmask))
    {
        return false;
    }
    if (0 != pcap_setfilter(m_pcap.get(), &filter))
    {
        pcap_freecode(&filter);
        return false;
    }
    pcap_freecode(&filter);
    return true;
}

void PcapDevice::setDataCallback(const std::function<void(const unsigned char* data, unsigned int dataLen)>& cb)
{
    std::lock_guard<std::mutex> locker(m_mutexOnDataCallback);
    m_onDataCallback = cb;
}

int PcapDevice::captureOnce(unsigned int count)
{
    std::shared_ptr<pcap_t> pcap;
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (!m_pcap)
        {
            return -1;
        }
        if (!m_captureStarted)
        {
            return -1;
        }
        if (m_dispatching)
        {
            return 0;
        }
        pcap = m_pcap;
        m_dispatching = true;
    }
    int ret = pcap_dispatch(pcap.get(), count, onPacketArrived, (u_char*)this);
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        m_dispatching = false;
        m_cvDispatch.notify_all();
    }
    return ret;
}

bool PcapDevice::startCapture()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (!m_pcap)
    {
        return false;
    }
    m_captureStarted = true;
    return true;
}

void PcapDevice::stopCapture()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_captureStarted = false;
}

void PcapDevice::close()
{
    stopCapture();
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (m_pcap)
        {
            pcap_breakloop(m_pcap.get());
            m_pcap.reset();
        }
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
    if (err < 0)
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
} // namespace npacket
