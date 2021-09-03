#include "net.h"

#include <string.h>
#ifdef _WIN32
#include <WinSock2.h>
/* 需要在WinSock2.h后包含 */
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace utilitiy
{
bool Net::isIPv4(const std::string& ip)
{
    if (ip.empty())
    {
        return false;
    }
    int a1 = 0, a2 = 0, a3 = 0, a4 = 0;
    if (4 == sscanf(ip.c_str(), "%d.%d.%d.%d", &a1, &a2, &a3, &a4))
    {
        if ((a1 >= 0 && a1 <= 255) && (a2 >= 0 && a2 <= 255) && (a3 >= 0 && a3 <= 255) && (a4 >= 0 && a4 <= 255))
        {
            return true;
        }
    }
    return false;
}

bool Net::isIPv4Inner(const std::string& ip)
{
    /*
     * 内网(私有)IP:
     *     127.0.0.1
     *     A: 10.0.0.0    - 10.255.255.255
     *     B: 172.16.0.0  - 172.31.255.255
     *     C: 192.168.0.0 - 192.168.255.255
     */
    /* A: 10.0.0.0    - 10.255.255.255 */
    static const auto A_BEGIN = (unsigned long long)10 * (256 * 256 * 256);
    static const auto A_END = (unsigned long long)10 * (256 * 256 * 256) + 255 * (256 * 256) + 255 * (256) + 255;
    /* B: 172.16.0.0  - 172.31.255.255 */
    static const auto B_BEGIN = (unsigned long long)172 * (256 * 256 * 256) + 16 * (256 * 256);
    static const auto B_END = (unsigned long long)172 * (256 * 256 * 256) + 31 * (256 * 256) + 255 * (256) + 255;
    /* C: 192.168.0.0 - 192.168.255.255 */
    static const auto C_BEGIN = (unsigned long long)192 * (256 * 256 * 256) + 168 * (256 * 256);
    static const auto C_END = (unsigned long long)192 * (256 * 256 * 256) + 168 * (256 * 256) + 255 * (256) + 255;
    if (ip.empty())
    {
        return false;
    }
    if (0 == ip.compare("127.0.0.1"))
    {
        return true;
    }
    unsigned long long a1 = 0, a2 = 0, a3 = 0, a4 = 0;
    if (4 != sscanf(ip.c_str(), "%lld.%lld.%lld.%lld", &a1, &a2, &a3, &a4))
    {
        return false;
    }
    if ((a1 < 0 || a1 > 255) || (a2 < 0 || a2 > 255) || (a3 < 0 || a3 > 255) || (a4 < 0 || a4 > 255))
    {
        return false;
    }
    auto ipNum = a1 * (256 * 256 * 256) + a2 * (256 * 256) + a3 * (256) + a4;
    if ((ipNum >= A_BEGIN && ipNum <= A_END) || (ipNum >= B_BEGIN && ipNum <= B_END) || (ipNum >= C_BEGIN && ipNum <= C_END))
    {
        return true;
    }
    return false;
}

IPv4Info Net::calcIPv4Info(const std::string& ip, const std::string& netmask)
{
    IPv4Info info;
    int a1 = 0, a2 = 0, a3 = 0, a4 = 0;
    if (4 != sscanf(ip.c_str(), "%d.%d.%d.%d", &a1, &a2, &a3, &a4))
    {
        return info;
    }
    if ((a1 < 0 || a1 > 255) || (a2 < 0 || a2 > 255) || (a3 < 0 || a3 > 255) || (a4 < 0 || a4 > 255))
    {
        return info;
    }
    int m1 = 0, m2 = 0, m3 = 0, m4 = 0;
    if (4 != sscanf(netmask.c_str(), "%d.%d.%d.%d", &m1, &m2, &m3, &m4))
    {
        return info;
    }
    if ((m1 < 0 || m1 > 255) || (m2 < 0 || m2 > 255) || (m3 < 0 || m3 > 255) || (m4 < 0 || m4 > 255))
    {
        return info;
    }
    info.ip = ip;
    info.netmask = netmask;
    /* 计算网络地址: ip & netmask */
    unsigned char n1 = a1 & m1, n2 = a2 & m2, n3 = a3 & m3, n4 = a4 & m4;
    char network[16] = {0};
#ifdef _WIN32
    sprintf_s(network, "%d.%d.%d.%d", n1, n2, n3, n4);
#else
    sprintf(network, "%d.%d.%d.%d", n1, n2, n3, n4);
#endif
    info.network = network;
    /* 计算主机地址: ip & ~netmask */
    unsigned char h1 = a1 & (~m1), h2 = a2 & (~m2), h3 = a3 & (~m3), h4 = a4 & (~m4);
    char host[16] = {0};
#ifdef _WIN32
    sprintf_s(host, "%d.%d.%d.%d", h1, h2, h3, h4);
#else
    sprintf(host, "%d.%d.%d.%d", h1, h2, h3, h4);
#endif
    info.host = host;
    /* 计算广播地址: (ip & netmask) | ~netmask */
    unsigned char b1 = n1 | ~m1, b2 = n2 | ~m2, b3 = n3 | ~m3, b4 = n4 | ~m4;
    char broadcast[16] = {0};
#ifdef _WIN32
    sprintf_s(broadcast, "%d.%d.%d.%d", b1, b2, b3, b4);
#else
    sprintf(broadcast, "%d.%d.%d.%d", b1, b2, b3, b4);
#endif
    info.broadcast = broadcast;
    /* 计算默认网关: 网络地址 + 主机地址(除最后一位其他都置1) */
    unsigned char g1 = n1 + h1, g2 = n2 + h2, g3 = n3 + h3, g4 = n4 + (h4 | 0xFE);
    char defaultGateway[16] = {0};
#ifdef _WIN32
    sprintf_s(defaultGateway, "%d.%d.%d.%d", g1, g2, g3, g4);
#else
    sprintf(defaultGateway, "%d.%d.%d.%d", g1, g2, g3, g4);
#endif
    info.defaultGateway = defaultGateway;
    /* 计算主机数: 255.255.255.255 - 子网掩码 - 2 */
    static const unsigned long long MAX_COUNT = (unsigned long long)256 * 256 * 256 * 256;
    auto sCount = (unsigned long long)m1 * (256 * 256 * 256) + (unsigned long long)m2 * (256 * 256) + (unsigned long long)m3 * (256) + m4;
    info.hostCount = MAX_COUNT - sCount - 2;
    return info;
}

std::vector<NetInterface> Net::getNetInterfaces()
{
    std::vector<NetInterface> interfaceList;
#ifdef _WIN32
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO(); /* 存储本机网卡信息 */
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    bool renew = false;
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)(new BYTE[stSize]); /* 重新申请内存空间用来存储所有网卡信息 */
        nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
        renew = true;
    }
    if (ERROR_SUCCESS == nRel)
    {
        while (pIpAdapterInfo) /* 遍历所有网卡 */
        {
            NetInterface nc;
            /* 网卡名 */
            nc.name = pIpAdapterInfo->AdapterName;
            /* MAC地址 */
            for (UINT i = 0; i < pIpAdapterInfo->AddressLength; ++i)
            {
                char hex[4] = {0};
                sprintf_s(hex, sizeof(hex), "%02x", pIpAdapterInfo->Address[i]);
                nc.mac.emplace_back(hex);
            }
            /* 类型 */
            switch (pIpAdapterInfo->Type)
            {
            case MIB_IF_TYPE_OTHER:
                nc.type = NetInterface::Type::OTHER;
                break;
            case MIB_IF_TYPE_ETHERNET:
                nc.type = NetInterface::Type::ETHERNET;
                break;
            case MIB_IF_TYPE_TOKENRING:
                nc.type = NetInterface::Type::TOKENRING;
                break;
            case MIB_IF_TYPE_FDDI:
                nc.type = NetInterface::Type::FDDI;
                break;
            case MIB_IF_TYPE_PPP:
                nc.type = NetInterface::Type::PPP;
                break;
            case MIB_IF_TYPE_LOOPBACK:
                nc.type = NetInterface::Type::LOOPBACK;
                break;
            case MIB_IF_TYPE_SLIP:
                nc.type = NetInterface::Type::SLIP;
                break;
            default:
                nc.type = NetInterface::Type::OTHER;
                break;
            }
            /* 描述 */
            nc.desc = pIpAdapterInfo->Description;
            /* IPv4地址列表(可能网卡有多IP, 因此通过循环去判断) */
            IP_ADDR_STRING* pIpAddrString = &(pIpAdapterInfo->IpAddressList);
            do
            {
                NetInterface::IPv4AndMask im;
                im.ipv4 = pIpAddrString->IpAddress.String;
                im.netmask = pIpAddrString->IpMask.String;
                nc.ipv4List.emplace_back(im);
                pIpAddrString = pIpAddrString->Next;
            } while (pIpAddrString);
            /* 保存并遍历下一个 */
            interfaceList.emplace_back(nc);
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
            for (struct ifaddrs* ifa = ifList; NULL != ifa; ifa = ifa->ifa_next)
            {
                bool alreadyExist = false;
                for (size_t i = 0; i < interfaceList.size(); ++i)
                {
                    if (0 == interfaceList[i].name.compare(ifa->ifa_name))
                    {
                        alreadyExist = true;
                        break;
                    }
                }
                if (alreadyExist)
                {
                    continue;
                }
                NetInterface nc;
                struct ifreq ifreq;
                /* 网卡名 */
                strcpy(ifreq.ifr_name, ifa->ifa_name);
                nc.name = ifa->ifa_name;
                /* 网卡类型,MAC地址 */
                if (!ioctl(fd, SIOCGIFHWADDR, &ifreq))
                {
                    /* 网卡类型 */
                    switch (ifreq.ifr_hwaddr.sa_family)
                    {
                    case ARPHRD_ETHER:
                        nc.type = NetInterface::Type::ETHERNET;
                        break;
                    case ARPHRD_PRONET:
                        nc.type = NetInterface::Type::TOKENRING;
                        break;
                    case ARPHRD_FDDI:
                        nc.type = NetInterface::Type::FDDI;
                        break;
                    case ARPHRD_PPP:
                        nc.type = NetInterface::Type::PPP;
                        break;
                    case ARPHRD_LOOPBACK:
                        nc.type = NetInterface::Type::LOOPBACK;
                        break;
                    case ARPHRD_SLIP:
                        nc.type = NetInterface::Type::SLIP;
                        break;
                    default:
                        nc.type = NetInterface::Type::OTHER;
                        break;
                    }
                    /* MAC地址 */
                    for (int i = 0; i < 6; ++i)
                    {
                        char hex[4] = {0};
                        snprintf(hex, sizeof(hex), "%02x", (unsigned char)ifreq.ifr_hwaddr.sa_data[i]);
                        nc.mac.emplace_back(hex);
                    }
                }
                /* IPv4地址 */
                if (!ioctl(fd, SIOCGIFADDR, &ifreq))
                {
                    char ipv4[32] = {0};
                    snprintf(ipv4, sizeof(ipv4), "%s", (char*)inet_ntoa(((struct sockaddr_in*)&(ifreq.ifr_addr))->sin_addr));
                    nc.ipv4 = ipv4;
                }
                /* 子网掩码 */
                if (!ioctl(fd, SIOCGIFNETMASK, &ifreq))
                {
                    char netmask[32] = {0};
                    snprintf(netmask, sizeof(netmask), "%s", (char*)inet_ntoa(((struct sockaddr_in*)&(ifreq.ifr_netmask))->sin_addr));
                    nc.netmask = netmask;
                }
                /* 广播地址 */
                if (!ioctl(fd, SIOCGIFBRDADDR, &ifreq))
                {
                    char broadcast[32] = {0};
                    snprintf(broadcast, sizeof(broadcast), "%s", (char*)inet_ntoa(((struct sockaddr_in*)&(ifreq.ifr_broadaddr))->sin_addr));
                    nc.broadcast = broadcast;
                }
                /* 保存 */
                interfaceList.emplace_back(nc);
            }
            freeifaddrs(ifList);
        }
        close(fd);
    }
#endif
    return interfaceList;
}
} // namespace utilitiy
