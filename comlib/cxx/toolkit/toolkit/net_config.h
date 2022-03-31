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
         * @brief 判断网络接口是否一致(顺序无关)
         * @param portList 网络接口列表
         * @return true-一致, false-不一致
         */
        bool checkSamePorts(const std::vector<std::string>& portList) const;

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
     * @brief 获取以太网卡列表(过滤掉网桥), 并对列表进行排序(相同厂商多的网卡排在前面) 
     * @return 网卡列表
     */
    static std::vector<utility::Net::IfaceInfo> getEthernetCards();

    /**
     * @brief 修改以太网卡名(修改成功后需要重启才能生效)
     * @param macNameMap 要修改名称的网卡, key: MAC地址, value: 网卡名
     * @return true-有修改, false-无修改
     */
    static bool modifyEthernetCardName(const std::map<std::string, std::string>& macNameMap);

    /**
     * @brief 添加/修改网桥
     * @param name 网桥名称
     * @param ports 网络接口
     * @return true-有添加/修改, false-无添加/修改
     */
    static bool modifyBridge(const std::string& name, const std::vector<std::string>& ports);

    /**
     * @brief 删除网桥
     * @param name 网桥名称
     * @return true-有网桥, false-无网桥
     */
    static bool deleteBridge(const std::string& name);
};
} // namespace toolkit
