#pragma once
#include <functional>
#include <map>

#include "utility/net/net.h"

namespace toolkit
{
/**
 * @brief 网络配置
 */
class NetConfig final
{
public:
    /**
     * @brief 网桥信息
     */
    struct BridgeInfo
    {
        /**
         * @brief 是否有效
         * @return true-有效, false-无效
         */
        bool isValid() const;

        /**
         * @brief 重置
         */
        void reset();

        /**
         * @brief 判断网络接口是否有差异(顺序无关)
         * @param portList 网络接口列表
         * @param newList [输出]网络接口列表中新增加的接口
         * @param notList [输出]网络接口列表中不存在的接口
         * @return true-有差异, false-无差异
         */
        bool diffPorts(const std::vector<std::string>& portList, std::vector<std::string>* newList = nullptr,
                       std::vector<std::string>* notList = nullptr) const;

        bool operator==(const BridgeInfo& other) const;

        bool operator!=(const BridgeInfo& other) const;

        std::string name; /* 名称 */
        std::vector<std::string> ports; /* 网络接口 */
    };

    /**
     * @brief 判断指定网卡名是否存在
     * @param name 网卡名
     * @return true-存在, false-不存在
     */
    static bool isNetcardExist(const std::string& name);

    /**
     * @brief 判断指定网卡名是否已启动
     * @param name 网卡名
     * @return true-已启动, false-未启动
     */
    static bool isNetcardUp(const std::string& name);

    /**
     * @brief 获取网桥列表 
     * @return 网桥列表
     */
    static std::vector<BridgeInfo> getBridgeInfos();

    /**
     * @brief 获取以太网卡列表(过滤掉网桥)
     * @return 网卡列表
     */
    static std::vector<utility::Net::IfaceInfo> getEthernetCards();

    /**
     * @brief 配置以太网卡名(成功后需要重启系统或网络才能生效)
     * @param macNameMap 要配置的网卡, key: MAC地址, value: 网卡名
     * @return true-有配置操作, false-无配置操作
     */
    static bool configEthernetCardName(const std::map<std::string, std::string>& macNameMap);

    /**
     * @brief 配置网桥(不会自动启动网桥)
     * @param name 网桥名称
     * @param ports 网络接口
     * @param recfg 网桥存在时是否重配, true-重新配置, false-否
     * @return true-有配置操作, false-无配置操作
     */
    static bool configBridge(const std::string& name, const std::vector<std::string>& ports, bool recfg = false);

    /**
     * @brief 检测ping是否可通(阻塞)
     * @param src 本机要使用的网络接口(当本机有多个网卡时, 可以指定使用哪一个), 为空表示默认, 例如: 网卡名(enp1s0) 或 IP(192.168.3.123)
     * @param dest 目标地址, 可以为IP或域名
     * @param timeout 超时时间(单位: 秒), 小等于0表示使用默认超时
     * @param detail [输出]检测结果
     * @return true-可通, false-不通
     */
    static bool checkPing(const std::string& src, const std::string& dest, int timeout = 1, std::string* result = nullptr);
};
} // namespace toolkit
