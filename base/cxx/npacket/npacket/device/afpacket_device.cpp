#include "afpacket_device.h"
#ifdef __linux__

#include <arpa/inet.h>
#include <cstring>
#include <ifaddrs.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <pcap.h> /* 用于BPF编译 */
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>

namespace npacket
{
AfPacketDevice::~AfPacketDevice()
{
    close();
}

std::string AfPacketDevice::getName()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_name;
}

std::string AfPacketDevice::getDescribe()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_describe;
}

std::string AfPacketDevice::getIpv4Address()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_ipv4Address;
}

bool AfPacketDevice::isLoopback()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_isLoopback;
}

bool AfPacketDevice::open(const std::string& name, int direction, int snapLen, int promisc, int timeout, int bufferSize)
{
    if (name.empty() || name.size() >= IFNAMSIZ)
    {
        return false;
    }
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_sockfd >= 0)
    {
        return true;
    }
    /* 创建AF_PACKET套接字 */
    m_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (m_sockfd < 0)
    {
        return false;
    }
    /* 获取接口索引 */
    int ifIndex = if_nametoindex(name.c_str());
    if (0 == ifIndex)
    {
        close();
        return false;
    }
    /* 获取接口标志 */
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0'; /* 确保字符串终止 */
    if (ioctl(m_sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        close();
        return false;
    }
    m_isLoopback = (ifr.ifr_flags & IFF_LOOPBACK) != 0;
    /* 设置混杂模式 */
    if (promisc)
    {
        struct packet_mreq mr;
        memset(&mr, 0, sizeof(mr));
        mr.mr_ifindex = ifIndex;
        mr.mr_type = PACKET_MR_PROMISC;
        if (setsockopt(m_sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0)
        {
            /* 非致命错误, 继续 */
        }
    }
    /* 设置接收缓冲区大小 */
    if (bufferSize >= 65536)
    {
        setsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));
    }
    /* 设置方向过滤(Linux 3.2+支持) */
    if (direction == 1 || direction == 2)
    {
        /* 1=仅接收, 2=仅发送 -> 0=不忽略发送, 1=忽略发送(仅接收) */
        int ignore_outgoing = (direction == 1) ? 1 : 0;
        setsockopt(m_sockfd, SOL_PACKET, PACKET_IGNORE_OUTGOING, &ignore_outgoing, sizeof(ignore_outgoing));
    }
    /* 保存参数 */
    m_name = name;
    m_snapLen = (snapLen <= 0 || snapLen > 65536) ? 65536 : snapLen;
    m_timeoutMs = timeout;
    m_describe = name;
    /* 获取IPv4地址 */
    struct ifaddrs* ifaddr;
    if (0 == getifaddrs(&ifaddr))
    {
        for (struct ifaddrs* ifa = ifaddr; nullptr != ifa; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr && AF_INET == ifa->ifa_addr->sa_family && 0 == strcmp(ifa->ifa_name, name.c_str()))
            {
                char addrString[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, addrString, INET_ADDRSTRLEN);
                m_ipv4Address = addrString;
                break;
            }
        }
        freeifaddrs(ifaddr);
    }
    /* TPACKET_V3 配置 */
    m_useV3 = false;
    const size_t minV3BufferSize = 8 * 1024 * 1024; /* 至少8MB才启用V3 */
    const size_t v3BlockSize = 2 * 1024 * 1024; /* 块大小: 2MB (必须是2的幂) */
    if (bufferSize >= (int)(minV3BufferSize))
    {
        int version = TPACKET_V3; /* 启用TPACKET_V3版本 */
        if (0 == setsockopt(m_sockfd, SOL_PACKET, PACKET_VERSION, &version, sizeof(version)))
        {
            /* 配置V3环形缓冲区 */
            struct tpacket_req3 req;
            memset(&req, 0, sizeof(req));
            req.tp_block_size = v3BlockSize;
            req.tp_block_nr = bufferSize / req.tp_block_size;
            if (req.tp_block_nr == 0) /* 防止除零 */
            {
                close();
                return false;
            }
            req.tp_frame_size = m_snapLen;
            /* 检查整数溢出: tp_block_size * tp_block_nr */
            size_t totalSize = 0;
            if (__builtin_mul_overflow(req.tp_block_size, req.tp_block_nr, &totalSize))
            {
                close();
                return false;
            }
            req.tp_frame_nr = totalSize / req.tp_frame_size;
            /* 设置15ms超时, 避免块长时间未满 */
            req.tp_retire_blk_tov = 15; /* 15ms超时退役块 */
            req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH;
            if (0 == setsockopt(m_sockfd, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req)))
            {
                /* 内存映射环形缓冲区 */
                m_ringBufferSize = req.tp_block_size * req.tp_block_nr;
                m_ringBuffer = (uint8_t*)(mmap(nullptr, m_ringBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, m_sockfd, 0));
                if (MAP_FAILED != m_ringBuffer)
                {
                    m_useV3 = true;
                    m_blockCount = req.tp_block_nr;
                    m_currentBlock = (struct tpacket_block_desc*)(m_ringBuffer);
                    m_blockIndex = 0;
                    /* 锁定内存，防止被交换到磁盘 */
                    mlock(m_ringBuffer, m_ringBufferSize);
                }
            }
        }
    }
    /* 绑定到指定接口 */
    struct sockaddr_ll saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    saddr.sll_ifindex = ifIndex;
    if (bind(m_sockfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
    {
        close();
        return false;
    }
    /* 普通模式缓冲区 */
    if (!m_useV3)
    {
        m_buffer.resize(m_snapLen);
    }
    return true;
}

bool AfPacketDevice::setFilter(const std::string& bpf, int optimize, int netmask)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_sockfd < 0)
    {
        return false;
    }
    /* 使用libpcap编译BPF */
    struct bpf_program fp;
    pcap_t* pcap = pcap_open_dead(DLT_EN10MB, m_snapLen);
    if (!pcap)
    {
        return false;
    }
    if (pcap_compile(pcap, &fp, bpf.c_str(), optimize, netmask) < 0)
    {
        pcap_close(pcap);
        return false;
    }
    /* 应用BPF到套接字 */
    if (setsockopt(m_sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &fp, sizeof(fp)) < 0)
    {
        pcap_freecode(&fp);
        pcap_close(pcap);
        return false;
    }
    pcap_freecode(&fp);
    pcap_close(pcap);
    return true;
}

void AfPacketDevice::setDataCallback(const std::function<void(const unsigned char* data, unsigned int dataLen)>& cb)
{
    std::lock_guard<std::mutex> locker(m_mutexOnDataCallback);
    m_onDataCallback = cb;
}

int AfPacketDevice::captureOnce(unsigned int count)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_sockfd < 0 || !m_captureStarted.load())
    {
        return -1;
    }
    /* TPACKET_V3 模式 */
    if (m_useV3)
    {
        return (int)processV3Block();
    }
    /* 普通模式 */
    struct pollfd pfd;
    pfd.fd = m_sockfd;
    pfd.events = POLLIN;
    if (poll(&pfd, 1, m_timeoutMs) <= 0) /* 超时或无数据 */
    {
        return 0;
    }
    /* 获取回调(减少锁开销) */
    std::function<void(const unsigned char* data, unsigned int dataLen)> onDataCallback = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexOnDataCallback);
        onDataCallback = m_onDataCallback;
    }
    if (!onDataCallback) /* 无回调无需捕获 */
    {
        return 0;
    }
    size_t packets = 0; /* 实际处理的包数量 */
    while (1) /* 循环处理 */
    {
        if (count > 0 && packets >= count) /* 达到限制则退出 */
        {
            break;
        }
        ssize_t len = recvfrom(m_sockfd, m_buffer.data(), m_snapLen, MSG_DONTWAIT, nullptr, nullptr);
        if (len <= 0) /* EAGAIN或无数据 */
        {
            break;
        }
        ++packets;
        onDataCallback(m_buffer.data(), len);
    }
    return (int)packets;
}

bool AfPacketDevice::startCapture()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_sockfd < 0 || m_captureStarted.load())
    {
        return false;
    }
    m_captureStarted = true;
    return true;
}

void AfPacketDevice::stopCapture()
{
    m_captureStarted = false;
}

void AfPacketDevice::close()
{
    stopCapture();
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    /* 清理TPACKET_V3资源 */
    if (m_useV3 && m_ringBuffer)
    {
        /* 解除内存锁定 */
        if (m_ringBufferSize > 0)
        {
            munlock(m_ringBuffer, m_ringBufferSize);
        }
        /* 取消内存映射 */
        if (MAP_FAILED != m_ringBuffer)
        {
            munmap(m_ringBuffer, m_ringBufferSize);
        }
        /* 停止环形缓冲区 */
        struct tpacket_req3 req = {0};
        setsockopt(m_sockfd, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req));
        m_ringBuffer = nullptr;
        m_ringBufferSize = 0;
        m_blockCount = 0;
        m_useV3 = false;
    }
    if (m_sockfd >= 0)
    {
        ::close(m_sockfd);
        m_sockfd = -1;
    }
}

std::vector<std::shared_ptr<AfPacketDevice>> AfPacketDevice::getAllDevices(std::string* errorBuffer)
{
    if (errorBuffer)
    {
        errorBuffer->clear();
    }
    std::vector<std::shared_ptr<AfPacketDevice>> devList;
    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) < 0)
    {
        if (errorBuffer)
        {
            *errorBuffer = strerror(errno);
        }
        return devList;
    }
    for (struct ifaddrs* ifa = ifaddr; nullptr != ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_name || !(ifa->ifa_flags & IFF_UP))
        {
            continue;
        }
        /* 查找是否已存在相同接口的设备 */
        bool found = false;
        for (const auto& dev : devList)
        {
            if (dev->m_name == ifa->ifa_name)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            continue;
        }
        /* 创建新设备 */
        auto dev = std::make_shared<AfPacketDevice>();
        dev->m_name = ifa->ifa_name;
        dev->m_describe = std::string(ifa->ifa_name);
        dev->m_isLoopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;
        /* 获取接口地址 */
        if (ifa->ifa_addr)
        {
            if (AF_INET == ifa->ifa_addr->sa_family)
            {
                char addrString[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, addrString, INET_ADDRSTRLEN);
                dev->m_ipv4Address = addrString;
            }
        }
        devList.emplace_back(dev);
    }
    freeifaddrs(ifaddr);
    return devList;
}

size_t AfPacketDevice::processV3Block()
{
    if (!m_useV3 || !m_ringBuffer || !m_captureStarted.load())
    {
        return 0;
    }
    size_t packetsProcessed = 0;
    /* 遍历所有块, 处理就绪的块 */
    for (unsigned int i = 0; i < m_blockCount; ++i)
    {
        ::tpacket_block_desc* block = (::tpacket_block_desc*)(m_ringBuffer + (i * m_ringBufferSize / m_blockCount));
        /* 检查块是否准备好被用户处理 */
        if (0 == (block->hdr.bh1.block_status & TP_STATUS_USER))
        {
            continue;
        }
        /* 遍历块中的所有帧 */
        ::tpacket3_hdr* ppd = (::tpacket3_hdr*)((uint8_t*)(block) + block->hdr.bh1.offset_to_first_pkt);
        unsigned int pktIndex = 0;
        /* 获取回调(减少锁开销) */
        std::function<void(const unsigned char* data, unsigned int dataLen)> onDataCallback = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexOnDataCallback);
            onDataCallback = m_onDataCallback;
        }
        if (!onDataCallback) /* 无回调无需处理 */
        {
            block->hdr.bh1.block_status = TP_STATUS_KERNEL; /* 直接归还内核 */
            continue;
        }
        while (ppd && pktIndex < block->hdr.bh1.num_pkts)
        {
            ++packetsProcessed;
            /* 验证帧数据指针和长度 */
            uint8_t* data = (uint8_t*)(ppd) + ppd->tp_mac;
            unsigned int dataLen = ppd->tp_snaplen;
            if (dataLen > m_snapLen) /* 防止回调处理越界数据 */
            {
                dataLen = m_snapLen;
            }
            /* 调用回调 */
            onDataCallback(data, dataLen);
            /* 移动到下一帧, 检查tp_next_offset防止越界 */
            if (ppd->tp_next_offset == 0 || ppd->tp_next_offset > (m_ringBufferSize / m_blockCount))
            {
                break; /* 异常偏移, 防止越界访问 */
            }
            ppd = (::tpacket3_hdr*)((uint8_t*)(ppd) + ppd->tp_next_offset);
            ++pktIndex;
        }
        block->hdr.bh1.block_status = TP_STATUS_KERNEL; /* 块处理完毕, 归还内核 */
    }
    return packetsProcessed;
}
} // namespace npacket
#endif
