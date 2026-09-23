#include "net.h"

#include <map>
#include <string.h>
#ifdef _WIN32
#include <WinSock2.h>
/* 需要在WinSock2.h后包含 */
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace utility
{
bool Net::isIPv4(const std::string& ip, uint8_t out[4])
{
    if (ip.empty())
    {
        return false;
    }
    int part = 0; /* 当前段的值 */
    int dots = 0; /* 点的数量 */
    int digits = 0; /* 当前段的位数 */
    uint8_t seg[4] = {0}; /* 临时缓存, 校验通过后再写出 */
    for (size_t i = 0, n = ip.size(); i < n; ++i)
    {
        auto ch = ip[i];
        if (ch >= '0' && ch <= '9')
        {
            if (++digits > 3) /* 每段最多3位 */
            {
                return false;
            }
            part = part * 10 + (ch - '0');
            if (part > 255) /* 每段范围值: 0~255 */
            {
                return false;
            }
        }
        else if ('.' == ch)
        {
            if (0 == digits) /* 空段, 如: "1..2" */
            {
                return false;
            }
            if (++dots > 3) /* 点不能超过3个 */
            {
                return false;
            }
            seg[dots - 1] = (uint8_t)part; /* 存下刚结束的这一段 */
            part = 0;
            digits = 0;
        }
        else /* 非法字符 */
        {
            return false;
        }
    }
    if (3 != dots || digits <= 0) /* 恰好3个点, 且最后一段非空 */
    {
        return false;
    }
    seg[3] = (uint8_t)part; /* 最后一段 */
    if (out) /* 仅在校验全部通过后写出 */
    {
        out[0] = seg[0];
        out[1] = seg[1];
        out[2] = seg[2];
        out[3] = seg[3];
    }
    return true;
}

bool Net::isIPv4Inner(const std::string& ip)
{
    /*
     * 内网(私有)IP:
     *     回环:  127.0.0.1/8     (127.0.0.0   - 127.255.255.255)
     *     私有A: 10.0.0.0/8      (10.0.0.0    - 10.255.255.255)
     *     私有B: 172.16.0.0/12   (172.16.0.0  - 172.31.255.255)
     *     私有C: 192.168.0.0/16  (192.168.0.0 - 192.168.255.255)
     */
    uint8_t a[4] = {0};
    if (!isIPv4(ip, a)) /* 格式校验 + 取四段值 */
    {
        return false;
    }
    if (127 == a[0]) /* 127.0.0.0/8 */
    {
        return true;
    }
    if (10 == a[0]) /* 10.0.0.0/8 */
    {
        return true;
    }
    if (172 == a[0] && a[1] >= 16 && a[1] <= 31) /* 172.16.0.0/12 */
    {
        return true;
    }
    if (192 == a[0] && 168 == a[1]) /* 192.168.0.0/16 */
    {
        return true;
    }
    return false;
}

Net::IPv4Info Net::calcIPv4Info(const std::string& ip, const std::string& netmask)
{
    IPv4Info info;
    /* 校验IP并取四段值 */
    uint8_t a[4];
    if (!isIPv4(ip, a))
    {
        return info;
    }
    /* 校验子网掩码并取四段值 */
    uint8_t m[4];
    if (!isIPv4(netmask, m))
    {
        return info;
    }
    /* 校验掩码合法性: 必须是连续的1后跟连续的0, 把四段拼成32位无符号数, 取反后应形如：000...0111...1 */
    unsigned int mask32 = ((unsigned int)m[0] << 24) | ((unsigned int)m[1] << 16) | ((unsigned int)m[2] << 8) | (unsigned int)m[3];
    unsigned int inv = ~mask32;
    if ((inv & (inv + 1)) != 0) /* 取反后不是连续0后连续1, 则非法 */
    {
        return info;
    }
    /* 求前缀长度(0~32) */
    int prefixLen = 0;
    unsigned int tmp = mask32;
    while (tmp & 0x80000000u)
    {
        ++prefixLen;
        tmp <<= 1;
    }
    info.ip = ip;
    info.netmask = netmask;
    /* 网络地址: ip & netmask */
    uint8_t n[4] = {0};
    for (int i = 0; i < 4; ++i)
    {
        n[i] = a[i] & m[i];
    }
    char nBuf[16] = {0};
    snprintf(nBuf, sizeof(nBuf), "%u.%u.%u.%u", (unsigned)n[0], (unsigned)n[1], (unsigned)n[2], (unsigned)n[3]);
    info.network = nBuf;
    /* 主机地址: ip & ~netmask */
    uint8_t h[4] = {0};
    for (int i = 0; i < 4; ++i)
    {
        h[i] = a[i] & (uint8_t)~m[i];
    }
    char hBuf[16] = {0};
    snprintf(hBuf, sizeof(hBuf), "%u.%u.%u.%u", (unsigned)h[0], (unsigned)h[1], (unsigned)h[2], (unsigned)h[3]);
    info.host = hBuf;
    /* 广播地址: (ip & netmask) | ~netmask */
    uint8_t b[4] = {0};
    for (int i = 0; i < 4; ++i)
    {
        b[i] = (a[i] & m[i]) | (uint8_t)~m[i];
    }
    char bBuf[16] = {0};
    snprintf(bBuf, sizeof(bBuf), "%u.%u.%u.%u", (unsigned)b[0], (unsigned)b[1], (unsigned)b[2], (unsigned)b[3]);
    info.broadcast = bBuf;
    /* 主机数: 2^(32-prefixLen) - 2; /31: 点对点链路(RFC 3021), 传统算法给0; /32: 单主机, 无网络/广播概念, 传统算法给0 */
    if (prefixLen >= 31)
    {
        info.hostCount = 0;
    }
    else
    {
        info.hostCount = (1ULL << (32 - prefixLen)) - 2ULL;
    }
    return info;
}

Net::IPv4Info Net::calcIPv4Info(const std::string& ipWithPrefix)
{
    IPv4Info info;
    /* 拆分IP与prefix */
    size_t slash = ipWithPrefix.find('/');
    auto ipPart = (std::string::npos == slash) ? ipWithPrefix : ipWithPrefix.substr(0, slash);
    /* 解析prefix, 无'/'时按0处理 */
    int prefix = 0;
    if (std::string::npos != slash)
    {
        auto prefixPart = ipWithPrefix.substr(slash + 1);
        if (prefixPart.empty()) /* "192.168.4.23/" 这种, prefix 为空, 非法 */
        {
            return info;
        }
        int digits = 0;
        for (char ch : prefixPart)
        {
            if (ch < '0' || ch > '9')
            {
                return info;
            }
            if (++digits > 2) /* 0~32 最多 2 位 */
            {
                return info;
            }
            prefix = prefix * 10 + (ch - '0');
            if (prefix > 32)
            {
                return info;
            }
        }
    }
    /* 由prefix生成netmask字符串 */
    uint32_t mask32 = (0 == prefix) ? 0u : (0xFFFFFFFFu << (32 - prefix));
    char maskBuf[16] = {0};
    snprintf(maskBuf, sizeof(maskBuf), "%u.%u.%u.%u", (unsigned)((mask32 >> 24) & 0xFF), (unsigned)((mask32 >> 16) & 0xFF),
             (unsigned)((mask32 >> 8) & 0xFF), (unsigned)(mask32 & 0xFF));
    return calcIPv4Info(ipPart, maskBuf);
}

std::vector<Net::IfaceInfo> Net::getAllInterfaces()
{
    std::vector<IfaceInfo> ifaceList;
#ifdef _WIN32
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO(); /* 存储本机网卡信息 */
    if (!pIpAdapterInfo)
    {
        return ifaceList;
    }
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    bool renew = false;
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)(new BYTE[stSize]); /* 重新申请内存空间用来存储所有网卡信息 */
        if (!pIpAdapterInfo)
        {
            return ifaceList;
        }
        nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
        renew = true;
    }
    if (ERROR_SUCCESS == nRel)
    {
        while (pIpAdapterInfo) /* 遍历所有网卡 */
        {
            IfaceInfo iface;
            /* 网卡名 */
            iface.name = pIpAdapterInfo->AdapterName;
            /* MAC地址 */
            for (UINT i = 0; i < pIpAdapterInfo->AddressLength; ++i)
            {
                char hex[4] = {0};
                sprintf_s(hex, sizeof(hex), "%02x", pIpAdapterInfo->Address[i]);
                iface.mac.emplace_back(hex);
            }
            /* 类型 */
            iface.realType = pIpAdapterInfo->Type;
            switch (iface.realType)
            {
            case MIB_IF_TYPE_OTHER:
                iface.type = IfaceInfo::Type::other;
                break;
            case MIB_IF_TYPE_ETHERNET:
                iface.type = IfaceInfo::Type::ethernet;
                break;
            case MIB_IF_TYPE_TOKENRING:
                iface.type = IfaceInfo::Type::tokenring;
                break;
            case MIB_IF_TYPE_FDDI:
                iface.type = IfaceInfo::Type::fddi;
                break;
            case MIB_IF_TYPE_PPP:
                iface.type = IfaceInfo::Type::ppp;
                break;
            case MIB_IF_TYPE_LOOPBACK:
                iface.type = IfaceInfo::Type::loopback;
                break;
            case MIB_IF_TYPE_SLIP:
                iface.type = IfaceInfo::Type::slip;
                break;
            default:
                iface.type = IfaceInfo::Type::other;
                break;
            }
            /* 描述 */
            iface.desc = pIpAdapterInfo->Description;
            /* IPv4地址列表(可能网卡有多IP, 因此通过循环去判断) */
            IP_ADDR_STRING* pIpAddrString = &(pIpAdapterInfo->IpAddressList);
            do
            {
                IfaceInfo::IPv4Mask im;
                im.ipv4 = pIpAddrString->IpAddress.String;
                im.netmask = pIpAddrString->IpMask.String;
                iface.ipv4List.emplace_back(im);
                pIpAddrString = pIpAddrString->Next;
            } while (pIpAddrString);
            /* 保存并遍历下一个 */
            ifaceList.emplace_back(iface);
            pIpAdapterInfo = pIpAdapterInfo->Next;
        }
    }
    if (pIpAdapterInfo)
    {
        if (renew)
        {
            delete[] pIpAdapterInfo;
        }
        else
        {
            delete pIpAdapterInfo;
        }
    }
#else
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd >= 0)
    {
        struct ifaddrs* ifList = NULL;
        if (getifaddrs(&ifList) >= 0)
        {
            /* step1. 收集所有唯一接口名(含DOWN), 同时从AF_PACKET获取链路层信息 */
            std::vector<std::string> ifNames;
            std::map<std::string, IfaceInfo> pktMap; /* 从AF_PACKET解析的网卡类型和MAC */
            for (struct ifaddrs* ifa = ifList; NULL != ifa; ifa = ifa->ifa_next)
            {
                if (!ifa->ifa_name)
                {
                    continue;
                }
                bool alreadyExist = false;
                for (const auto& name : ifNames)
                {
                    if (name == ifa->ifa_name)
                    {
                        alreadyExist = true;
                        break;
                    }
                }
                if (!alreadyExist)
                {
                    ifNames.push_back(ifa->ifa_name);
                }
                /* 从AF_PACKET获取网卡类型和MAC地址(支持DOWN状态) */
                if (ifa->ifa_addr && AF_PACKET == ifa->ifa_addr->sa_family)
                {
                    struct sockaddr_ll* sll = (struct sockaddr_ll*)ifa->ifa_addr;
                    IfaceInfo& iface = pktMap[ifa->ifa_name];
                    iface.name = ifa->ifa_name;
                    /* 网卡类型 */
                    iface.realType = sll->sll_hatype;
                    switch (iface.realType)
                    {
                    case ARPHRD_ETHER:
                        iface.type = IfaceInfo::Type::ethernet;
                        break;
                    case ARPHRD_PRONET:
                        iface.type = IfaceInfo::Type::tokenring;
                        break;
                    case ARPHRD_FDDI:
                        iface.type = IfaceInfo::Type::fddi;
                        break;
                    case ARPHRD_PPP:
                        iface.type = IfaceInfo::Type::ppp;
                        break;
                    case ARPHRD_LOOPBACK:
                        iface.type = IfaceInfo::Type::loopback;
                        break;
                    case ARPHRD_SLIP:
                        iface.type = IfaceInfo::Type::slip;
                        break;
                    default:
                        iface.type = IfaceInfo::Type::other;
                        break;
                    }
                    /* MAC地址 */
                    iface.mac.clear();
                    for (int i = 0; i < sll->sll_halen && i < 6; ++i)
                    {
                        char hex[4] = {0};
                        snprintf(hex, sizeof(hex), "%02x", sll->sll_addr[i]);
                        iface.mac.emplace_back(hex);
                    }
                }
            }
            freeifaddrs(ifList);
            /* step2. 逐个接口获取详细信息 */
            for (const auto& name : ifNames)
            {
                IfaceInfo iface;
                struct ifreq ifreq;
                memset(&ifreq, 0, sizeof(ifreq));
                strcpy(ifreq.ifr_name, name.c_str());
                /* 网卡名 */
                iface.name = name;
                /* 网卡类型, MAC地址 */
                if (!ioctl(fd, SIOCGIFHWADDR, &ifreq))
                {
                    /* 网卡类型 */
                    iface.realType = ifreq.ifr_hwaddr.sa_family;
                    switch (iface.realType)
                    {
                    case ARPHRD_ETHER:
                        iface.type = IfaceInfo::Type::ethernet;
                        break;
                    case ARPHRD_PRONET:
                        iface.type = IfaceInfo::Type::tokenring;
                        break;
                    case ARPHRD_FDDI:
                        iface.type = IfaceInfo::Type::fddi;
                        break;
                    case ARPHRD_PPP:
                        iface.type = IfaceInfo::Type::ppp;
                        break;
                    case ARPHRD_LOOPBACK:
                        iface.type = IfaceInfo::Type::loopback;
                        break;
                    case ARPHRD_SLIP:
                        iface.type = IfaceInfo::Type::slip;
                        break;
                    default:
                        iface.type = IfaceInfo::Type::other;
                        break;
                    }
                    /* MAC地址 */
                    for (int i = 0; i < 6; ++i)
                    {
                        char hex[4] = {0};
                        snprintf(hex, sizeof(hex), "%02x", (unsigned char)ifreq.ifr_hwaddr.sa_data[i]);
                        iface.mac.emplace_back(hex);
                    }
                }
                else if (pktMap.end() != pktMap.find(name)) /* ioctl失败时, 使用从AF_PACKET获取的信息(支持DOWN状态) */
                {
                    iface.realType = pktMap[name].realType;
                    iface.type = pktMap[name].type;
                    iface.mac = pktMap[name].mac;
                }
                /* 接口状态 */
                if (!ioctl(fd, SIOCGIFFLAGS, &ifreq))
                {
                    iface.isUp = 0 != (ifreq.ifr_flags & IFF_UP);
                }
                else
                {
                    iface.isUp = false;
                }
                /* IPv4地址 */
                if (!ioctl(fd, SIOCGIFADDR, &ifreq))
                {
                    char ipv4[32] = {0};
                    snprintf(ipv4, sizeof(ipv4), "%s", (char*)inet_ntoa(((struct sockaddr_in*)&(ifreq.ifr_addr))->sin_addr));
                    iface.ipv4 = ipv4;
                }
                /* 子网掩码 */
                if (!ioctl(fd, SIOCGIFNETMASK, &ifreq))
                {
                    char netmask[32] = {0};
                    snprintf(netmask, sizeof(netmask), "%s", (char*)inet_ntoa(((struct sockaddr_in*)&(ifreq.ifr_netmask))->sin_addr));
                    iface.netmask = netmask;
                }
                /* 广播地址 */
                if (!ioctl(fd, SIOCGIFBRDADDR, &ifreq))
                {
                    char broadcast[32] = {0};
                    snprintf(broadcast, sizeof(broadcast), "%s", (char*)inet_ntoa(((struct sockaddr_in*)&(ifreq.ifr_broadaddr))->sin_addr));
                    iface.broadcast = broadcast;
                }
                /* 保存到列表 */
                ifaceList.emplace_back(iface);
            }
        }
        close(fd);
    }
#endif
    return ifaceList;
}
} // namespace utility
