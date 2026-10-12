#pragma once
#include <string>
#include <vector>

namespace utility
{
class Net final
{
public:
    /**
     * @brief IPv4信息
     */
    struct IPv4Info
    {
        std::string ip; /* IP地址, 例如: 192.168.3.10 */
        std::string netmask; /* 子网掩码, 例如: 255.255.255.0 */
        std::string network; /* 网络地址, 例如: 192.168.3.0 */
        std::string host; /* 主机地址, 例如: 0.0.0.10 */
        std::string broadcast; /* 广播地址, 例如: 192.168.3.255 */
        uint64_t hostCount = 0; /* 主机数, 例如: 254 */
    };

    /**
     * @brief 网络接口(网卡)信息
     */
    struct IfaceInfo
    {
        /**
         * @brief 网卡链路层类型, 各平台的原始取值不同(Linux为ARPHRD_*, Windows为MIB_IF_TYPE_*或IF_TYPE_*), 具体取值见rawType
         */
        enum class LinkType
        {
            other, /* 其它类型(原始类型未覆盖时的兜底值) */
            ethernet, /* 以太网(Linux: ARPHRD_ETHER, Windows: MIB_IF_TYPE_ETHERNET=6) */
            tokenring, /* 令牌环网(Linux: ARPHRD_PRONET, Windows: MIB_IF_TYPE_TOKENRING=9) */
            fddi, /* 光纤分布式数据接口(Linux: ARPHRD_FDDI, Windows: MIB_IF_TYPE_FDDI=15) */
            ppp, /* 点对点协议(Linux: ARPHRD_PPP, Windows: MIB_IF_TYPE_PPP=23) */
            loopback, /* 本地回环(Linux: ARPHRD_LOOPBACK, Windows: MIB_IF_TYPE_LOOPBACK=24) */
            slip, /* 串行线路IP(Linux: ARPHRD_SLIP, Windows: MIB_IF_TYPE_SLIP=28) */
            wireless, /* 无线网卡(WiFi)(Linux: ARPHRD_IEEE80211(仅monitor模式), Windows: IF_TYPE_IEEE80211=71) */
            virtualNic, /* 专有虚拟网卡(TAP/TUN等软件模拟的网卡)(Windows: IF_TYPE_PROP_VIRTUAL=53, Linux下TAP上报为以太网) */
            tunnel, /* 隧道网卡(无链路层头部的封装接口)(Linux: ARPHRD_NONE, Windows: IF_TYPE_TUNNEL=131) */
            wwan /* 蜂窝网网卡(4G/5G拨号接口)(Windows: IF_TYPE_WWANPP=243, IF_TYPE_WWANPP2=244) */
        };

        /**
         * @brief 网卡硬件形态(按所属总线或设备来源归类, 非严格的总线类型), virt为无实体设备的虚拟网卡, soc为芯片内置网卡(实际挂在platform总线), 二者按来源归并
         */
        enum class BusType
        {
            unknown, /* 未知(判定失败) */
            virt, /* 虚拟网卡(无实体设备) */
            usb, /* USB网卡(含板载USB转网口) */
            pcie, /* PCIe网卡(扩展卡或x86板载) */
            soc /* SOC板载网卡 */
        };

        std::string linkTypeStr() const
        {
            switch (linkType)
            {
            case LinkType::ethernet:
                return "ethernet";
            case LinkType::tokenring:
                return "tokenring";
            case LinkType::fddi:
                return "fddi";
            case LinkType::ppp:
                return "ppp";
            case LinkType::loopback:
                return "loopback";
            case LinkType::slip:
                return "slip";
            case LinkType::wireless:
                return "wireless";
            case LinkType::virtualNic:
                return "virtualNic";
            case LinkType::tunnel:
                return "tunnel";
            case LinkType::wwan:
                return "wwan";
            default:
                return std::to_string(rawType);
            }
        }

        /**
         * @brief 获取网卡硬件形态的字符串描述
         * @return 字符串描述, 例如: pcie
         */
        std::string busTypeStr() const
        {
            switch (busType)
            {
            case BusType::virt:
                return "virtual";
            case BusType::usb:
                return "usb";
            case BusType::pcie:
                return "pcie";
            case BusType::soc:
                return "soc";
            default:
                return "unknown";
            }
        }

#ifdef _WIN32
        struct IPv4Mask
        {
            std::string ipv4; /* IPv4地址 */
            std::string netmask; /* 子网掩码 */
        };
#endif

        std::string name; /* 名称 */
        int rawType = 0; /* 链路层类型(平台原始值: Linux为ARPHRD_*, Windows为MIB_IF_TYPE_*) */
        LinkType linkType = LinkType::other; /* 链路层类型(转换后的类型值) */
        BusType busType = BusType::unknown; /* 网卡硬件形态(Windows下SOC为近似判定) */
        std::vector<std::string> mac; /* MAC地址 */
#ifdef _WIN32
        std::string desc; /* 描述 */
        std::vector<IPv4Mask> ipv4List; /* IPv4列表 */
#else
        bool isUp = false; /* 网卡是否启动: true=UP, false=DOWN */
        std::string ipv4; /* IPv4地址 */
        std::string netmask; /* 子网掩码 */
        std::string broadcast; /* 广播的地址 */
#endif
    };

public:
    /**
     * @brief 判断IP地址是否为IPv4格式
     * @param ip IP地址, 例如: 192.168.3.10
     * @param out [输出]IP地址各段值, 例如: {192, 168, 3, 10}
     * @return true-是, false-否
     */
    static bool isIPv4(const std::string& ip, uint8_t out[4] = nullptr);

    /**
     * @brief 判断IP地址是否为内网IPv4
     * @param ip IP地址, 例如: 192.168.3.10
     * @return true-是, false-否
     */
    static bool isIPv4Inner(const std::string& ip);

    /**
     * @brief 校验在现有IP地址取值后追加字符c, 结果是否仍为合法的IPv4前缀
     * @param ip 当前已有取值
     * @param c 待追加的字符(数字或点)
     * @return true-合法, false-非法
     */
    static bool canAppendIPv4(const std::string& ip, char c);

    /**
     * @brief 根据IPv4地址和子网掩码计算IPv4信息(说明: 网关无法自动计算, 必须是人为配置)
     * @param ip IPv4地址, 例如: 192.168.3.10
     * @param netmask 子网掩码, 例如: 255.255.255.0
     * @return IPv4信息
     */
    static IPv4Info calcIPv4Info(const std::string& ip, const std::string& netmask);

    /**
     * @brief 根据IPv4地址计算IPv4信息(说明: 网关无法自动计算, 必须是人为配置)
     * @param ip IPv4地址(说明: 子网掩码自动计算), 例如: 192.168.3.10/16, 192.168.3.10等价于192.168.3.10/32
     * @return IPv4信息
     */
    static IPv4Info calcIPv4Info(const std::string& ipWithPrefix);

    /**
     * @brief 获取本机所有网络接口(网卡)
     * @return 网络接口列表
     */
    static std::vector<IfaceInfo> getAllInterfaces();
};
} // namespace utility
