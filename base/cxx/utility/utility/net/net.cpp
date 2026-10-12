#include "net.h"

#include <map>
#include <string.h>
#include <vector>
#ifdef _WIN32
#include <WinSock2.h>
/* 需要在WinSock2.h后包含 */
#include <Iphlpapi.h>
#include <Winreg.h>
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Advapi32.lib")
#else
#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
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

bool Net::canAppendIPv4(const std::string& ip, char c)
{
    /* 仅允许数字和点 */
    if ('.' != c && (c < '0' || c > '9'))
    {
        return false;
    }
    if (ip.length() >= 15) /* IPv4最大长度15, 例如: 255.255.255.255 */
    {
        return false;
    }
    auto s = ip;
    s += c;
    if ('.' == s[0] || std::string::npos != s.find("..")) /* 首字符不能是点, 且不允许连续的点 */
    {
        return false;
    }
    int dots = 0; /* 点的数量: 4段IP最多3个点 */
    for (auto ch : s)
    {
        if ('.' == ch)
        {
            dots++;
        }
    }
    if (dots > 3)
    {
        return false;
    }
    /* 按点分段校验: 每段不超过3位, 无前导零, 数值不超过255 */
    size_t start = 0;
    for (size_t i = 0; i <= s.length(); ++i)
    {
        if (i != s.length() && '.' != s[i])
        {
            continue;
        }
        auto seg = s.substr(start, i - start);
        if (seg.empty()) /* 末尾空段表示刚输入点号, 等待继续输入, 属于合法中间状态 */
        {
            return (i == s.length());
        }
        if (seg.length() > 3)
        {
            return false;
        }
        if (seg.length() > 1 && '0' == seg[0]) /* 不允许前导零, 例如: 01 */
        {
            return false;
        }
        int num = 0;
        for (auto d : seg)
        {
            num = num * 10 + (d - '0');
        }
        if (num > 255)
        {
            return false;
        }
        start = i + 1;
    }
    return true;
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
    /* 解析prefix, 无'/'时按'/32'处理(单主机) */
    int prefix = 32;
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

#ifdef _WIN32
/**
 * @brief 读取网卡的设备实例ID(PnP实例ID)
 * @param adapterName 网卡名称(适配器GUID, 例如: {a5b0c1d2-...})
 * @return 设备实例ID, 例如: PCI\VEN_8086&DEV_15B8\3&11583659&0&FF, 失败返回空串
 */
static std::string getPnpInstanceId(const std::string& adapterName)
{
    /* 注册表路径: HKLM\SYSTEM\CurrentControlSet\Control\Network\{网卡类GUID}\{适配器GUID}\Connection */
    static const char* kNetClassGuid = "{4d36e972-e325-11ce-bfc1-08002be10318}"; /* 网卡设备类GUID */
    if (adapterName.empty()) /* 少数系统上适配器名为空, 无法定位注册表键 */
    {
        return "";
    }
    auto keyPath = std::string("SYSTEM\\CurrentControlSet\\Control\\Network\\") + kNetClassGuid + "\\" + adapterName + "\\Connection";
    /* 说明: 使用RegOpenKeyExA和RegQueryValueExA(自Windows 2000起可用), RegGetValueA需要Vista及以上系统 */
    HKEY key = NULL;
    if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &key))
    {
        return "";
    }
    std::string instanceId;
    DWORD valueSize = 0; /* 先查询取值大小, 再按实际大小取值, 避免固定缓冲不足 */
    if (ERROR_SUCCESS == RegQueryValueExA(key, "PnpInstanceID", nullptr, nullptr, nullptr, &valueSize) && valueSize > 1)
    {
        std::vector<char> buf(valueSize + 1, 0);
        if (ERROR_SUCCESS == RegQueryValueExA(key, "PnpInstanceID", nullptr, nullptr, (LPBYTE)buf.data(), &valueSize))
        {
            instanceId = buf.data();
        }
    }
    RegCloseKey(key);
    return instanceId;
}
#endif

/**
 * @brief 判断网卡物理形态(所属总线)
 * @param name 网卡名称
 * @return 物理形态
 */
static Net::IfaceInfo::BusType getIfaceBusType(const std::string& name)
{
#ifdef _WIN32
    /* 设备实例ID示例:
     * PCIe网卡: PCI\VEN_8086&DEV_15B8\3&11583659&0&FF
     * USB网卡:  USB\VID_0B95&PID_1790\000001
     * SOC网卡:  ACPI\VEN_...(Windows无SOC总线概念, 按ACPI枚举近似判定)
     * 虚拟网卡: ROOT\..., SW\..., SWD\..., VMBUS\...(Hyper-V, 虚拟机合成网卡)
     */
    auto instanceId = getPnpInstanceId(name);
    if (instanceId.empty())
    {
        return Net::IfaceInfo::BusType::unknown;
    }
    auto enumerator = instanceId.substr(0, instanceId.find('\\')); /* 实例ID首段即枚举器名称 */
    if ("USB" == enumerator)
    {
        return Net::IfaceInfo::BusType::usb;
    }
    if ("PCI" == enumerator)
    {
        return Net::IfaceInfo::BusType::pcie;
    }
    if ("ACPI" == enumerator)
    {
        return Net::IfaceInfo::BusType::soc;
    }
    if ("ROOT" == enumerator || "SW" == enumerator || "SWD" == enumerator || "VMBUS" == enumerator)
    {
        return Net::IfaceInfo::BusType::virtual_;
    }
    return Net::IfaceInfo::BusType::unknown;
#else
    /* 设备路径示例(通过/sys/class/net/<名称>的符号链接目标判断):
     * PCIe网卡: /sys/devices/platform/fe180000.pcie/pci0003:30/0003:30:00.0/0003:31:00.0/net/enp1s0
     * SOC网卡:  /sys/devices/platform/PHYT0004:01/net/enp2s0
     * USB网卡:  /sys/devices/pci0000:00/0000:00:02.0/0000:03:00.0/usb1/1-3/1-3.1/1-3.1:1.0/net/enp5s0
     * 虚拟网卡: /sys/devices/virtual/net/dummy0
     */
    char buf[1024] = {0};
    auto len = readlink(("/sys/class/net/" + name).c_str(), buf, sizeof(buf) - 1);
    if (len < 0)
    {
        return Net::IfaceInfo::BusType::unknown;
    }
    std::string devPath(buf, len);
    std::transform(devPath.begin(), devPath.end(), devPath.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    if (std::string::npos != devPath.find("/virtual/")) /* 虚拟网卡 */
    {
        return Net::IfaceInfo::BusType::virtual_;
    }
    if (std::string::npos != devPath.find("/usb")) /* USB网卡, 其父控制器常同时含pci, 因此要先判断 */
    {
        return Net::IfaceInfo::BusType::usb;
    }
    if (std::string::npos != devPath.find("pci")) /* PCIe网卡(含BDF段, 例如: 0000:13:00.0) */
    {
        return Net::IfaceInfo::BusType::pcie;
    }
    if (std::string::npos != devPath.find("/platform/")) /* SOC板载网卡(含/platform/soc/, /platform/PHYT0004:01) */
    {
        return Net::IfaceInfo::BusType::soc;
    }
    return Net::IfaceInfo::BusType::unknown;
#endif
}

#ifndef _WIN32
/**
 * @brief 判断网卡是否为无线网卡
 * @param name 网卡名称
 * @return true-无线网卡, false-非无线网卡或判定失败
 * @note station模式的无线网卡, 内核上报的硬件类型仍是ARPHRD_ETHER(与以太网一致), 只能通过sysfs区分:
 *       /sys/class/net/<名称>/wireless(无线扩展)或/sys/class/net/<名称>/phy80211(mac80211设备)
 */
static bool isWirelessIface(const std::string& name)
{
    auto basePath = "/sys/class/net/" + name + "/";
    struct stat st = {0};
    return (0 == stat((basePath + "wireless").c_str(), &st) || 0 == stat((basePath + "phy80211").c_str(), &st));
}
#endif

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
            iface.rawType = pIpAdapterInfo->Type;
            switch (iface.rawType)
            {
            case MIB_IF_TYPE_OTHER:
                iface.linkType = IfaceInfo::LinkType::other;
                break;
            case MIB_IF_TYPE_ETHERNET:
                iface.linkType = IfaceInfo::LinkType::ethernet;
                break;
            case MIB_IF_TYPE_TOKENRING:
                iface.linkType = IfaceInfo::LinkType::tokenring;
                break;
            case MIB_IF_TYPE_FDDI:
                iface.linkType = IfaceInfo::LinkType::fddi;
                break;
            case MIB_IF_TYPE_PPP:
                iface.linkType = IfaceInfo::LinkType::ppp;
                break;
            case MIB_IF_TYPE_LOOPBACK:
                iface.linkType = IfaceInfo::LinkType::loopback;
                break;
            case MIB_IF_TYPE_SLIP:
                iface.linkType = IfaceInfo::LinkType::slip;
                break;
            case IF_TYPE_IEEE80211: /* 无线网卡(WiFi), 例如: 71 */
                iface.linkType = IfaceInfo::LinkType::wireless;
                break;
            case IF_TYPE_PROP_VIRTUAL: /* 专有虚拟网卡(TAP/TUN), 例如: 53 */
                iface.linkType = IfaceInfo::LinkType::virtual_;
                break;
            case IF_TYPE_TUNNEL: /* 隧道网卡, 例如: 131 */
                iface.linkType = IfaceInfo::LinkType::tunnel;
                break;
            case IF_TYPE_WWANPP: /* 蜂窝网网卡(PPP方式), 例如: 243 */
            case IF_TYPE_WWANPP2: /* 蜂窝网网卡(PPP2方式), 例如: 244 */
                iface.linkType = IfaceInfo::LinkType::wwan;
                break;
            default:
                iface.linkType = IfaceInfo::LinkType::other;
                break;
            }
            /* 网卡形态 */
            iface.busType = getIfaceBusType(iface.name);
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
                    iface.rawType = sll->sll_hatype;
                    switch (iface.rawType)
                    {
                    case ARPHRD_ETHER:
                        iface.linkType = IfaceInfo::LinkType::ethernet;
                        break;
                    case ARPHRD_PRONET:
                        iface.linkType = IfaceInfo::LinkType::tokenring;
                        break;
                    case ARPHRD_FDDI:
                        iface.linkType = IfaceInfo::LinkType::fddi;
                        break;
                    case ARPHRD_PPP:
                        iface.linkType = IfaceInfo::LinkType::ppp;
                        break;
                    case ARPHRD_LOOPBACK:
                        iface.linkType = IfaceInfo::LinkType::loopback;
                        break;
                    case ARPHRD_SLIP:
                        iface.linkType = IfaceInfo::LinkType::slip;
                        break;
                    case ARPHRD_IEEE80211: /* 无线网卡处于monitor模式时上报的类型 */
                        iface.linkType = IfaceInfo::LinkType::wireless;
                        break;
                    case ARPHRD_TUNNEL: /* IPv4隧道设备(tunl0), 例如: 768 */
                    case ARPHRD_TUNNEL6: /* IPv6隧道设备(ip6tnl0), 例如: 769 */
                    case ARPHRD_SIT: /* IPv6 over IPv4隧道设备(sit0), 例如: 776 */
                    case ARPHRD_IPGRE: /* GRE隧道设备(gre0), 例如: 778 */
                    case ARPHRD_NONE: /* 无链路层头部(TUN设备), 例如: 65534 */
                        iface.linkType = IfaceInfo::LinkType::tunnel;
                        break;
                    default:
                        iface.linkType = IfaceInfo::LinkType::other;
                        break;
                    }
                    if (isWirelessIface(iface.name)) /* 补充判据: station模式无线网卡上报的类型仍是ARPHRD_ETHER */
                    {
                        iface.linkType = IfaceInfo::LinkType::wireless;
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
                /* 网卡形态(每个网卡只查询一次, 与ioctl是否成功无关) */
                iface.busType = getIfaceBusType(name);
                /* 网卡类型, MAC地址 */
                if (!ioctl(fd, SIOCGIFHWADDR, &ifreq))
                {
                    /* 网卡类型 */
                    iface.rawType = ifreq.ifr_hwaddr.sa_family;
                    switch (iface.rawType)
                    {
                    case ARPHRD_ETHER:
                        iface.linkType = IfaceInfo::LinkType::ethernet;
                        break;
                    case ARPHRD_PRONET:
                        iface.linkType = IfaceInfo::LinkType::tokenring;
                        break;
                    case ARPHRD_FDDI:
                        iface.linkType = IfaceInfo::LinkType::fddi;
                        break;
                    case ARPHRD_PPP:
                        iface.linkType = IfaceInfo::LinkType::ppp;
                        break;
                    case ARPHRD_LOOPBACK:
                        iface.linkType = IfaceInfo::LinkType::loopback;
                        break;
                    case ARPHRD_SLIP:
                        iface.linkType = IfaceInfo::LinkType::slip;
                        break;
                    case ARPHRD_IEEE80211: /* 无线网卡处于monitor模式时上报的类型 */
                        iface.linkType = IfaceInfo::LinkType::wireless;
                        break;
                    case ARPHRD_TUNNEL: /* IPv4隧道设备(tunl0), 例如: 768 */
                    case ARPHRD_TUNNEL6: /* IPv6隧道设备(ip6tnl0), 例如: 769 */
                    case ARPHRD_SIT: /* IPv6 over IPv4隧道设备(sit0), 例如: 776 */
                    case ARPHRD_IPGRE: /* GRE隧道设备(gre0), 例如: 778 */
                    case ARPHRD_NONE: /* 无链路层头部(TUN设备), 例如: 65534 */
                        iface.linkType = IfaceInfo::LinkType::tunnel;
                        break;
                    default:
                        iface.linkType = IfaceInfo::LinkType::other;
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
                    iface.rawType = pktMap[name].rawType;
                    iface.linkType = pktMap[name].linkType;
                    iface.mac = pktMap[name].mac;
                }
                if (isWirelessIface(name)) /* 补充判据: station模式无线网卡上报的类型仍是ARPHRD_ETHER */
                {
                    iface.linkType = IfaceInfo::LinkType::wireless;
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
