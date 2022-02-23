#pragma once
#include <string>
#include <vector>

namespace utilitiy
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
        std::string defaultGateway; /* 默认网关, 例如: 192.168.3.254 */
        int hostCount; /* 主机数, 例如: 254 */
    };

    /**
     * @brief 网络接口(网卡)信息
     */
    struct IfaceInfo
    {
        enum class Type
        {
            other,
            ethernet,
            tokenring,
            fddi,
            ppp,
            loopback,
            slip
        };

        std::string typeStr() const
        {
            switch (type)
            {
            case Type::ethernet:
                return "ethernet";
            case Type::tokenring:
                return "tokenring";
            case Type::fddi:
                return "fddi";
            case Type::ppp:
                return "ppp";
            case Type::loopback:
                return "loopback";
            case Type::slip:
                return "slip";
            }
            return std::to_string(realType);
        }

#ifdef _WIN32
        struct IPv4Mask
        {
            std::string ipv4; /* IPv4地址 */
            std::string netmask; /* 子网掩码 */
        };
#endif

        std::string name; /* 名称 */
        int realType; /* 网卡类型(实际的类型值, 不同平台可能不一样) */
        Type type; /* 网卡类型(转换后的类型值) */
        std::vector<std::string> mac; /* MAC地址 */
#ifdef _WIN32
        std::string desc; /* 描述 */
        std::vector<IPv4Mask> ipv4List; /* IPv4列表 */
#else
        std::string ipv4; /* IPv4地址 */
        std::string netmask; /* 子网掩码 */
        std::string broadcast; /* 广播的地址 */
#endif
    };

public:
    /**
     * @brief 判断IP地址是否为IPv4格式
     * @param ip IP地址, 例如: 192.168.3.10
     * @return true-是, false-否
     */
    static bool isIPv4(const std::string& ip);

    /**
     * @brief 判断IP地址是否为内网IPv4
     * @param ip IP地址, 例如: 192.168.3.10
     * @return true-是, false-否
     */
    static bool isIPv4Inner(const std::string& ip);

    /**
     * @brief 根据IPv4地址和子网掩码计算IPv4信息
     * @param ip IPv4地址, 例如: 192.168.3.10
     * @param netmask 子网掩码, 例如: 255.255.255.0
     * @return IPv4信息
     */
    static IPv4Info calcIPv4Info(const std::string& ip, const std::string& netmask);

    /**
     * @brief 获取本机所有网络接口(网卡)
     * @return 网络接口列表
     */
    static std::vector<IfaceInfo> getAllInterfaces();
};
} // namespace utilitiy
