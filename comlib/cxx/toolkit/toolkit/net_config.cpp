#include "net_config.h"

#include <algorithm>

#include "utility/strtool/strtool.h"
#include "utility/system/system.h"

namespace toolkit
{
bool NetConfig::BridgeInfo::isValid() const
{
    return !name.empty();
}

void NetConfig::BridgeInfo::reset()
{
    name.clear();
    ports.clear();
}

bool NetConfig::BridgeInfo::diffPorts(const std::vector<std::string>& portList, std::vector<std::string>* newList,
                                      std::vector<std::string>* notList) const
{
    std::vector<std::string> newPortList;
    std::vector<std::string> notPortList;
    /* 获取新增加的接口 */
    for (const auto& other : portList)
    {
        bool existFlag = false;
        for (const auto& port : ports)
        {
            if (other == port)
            {
                existFlag = true;
                break;
            }
        }
        if (!existFlag)
        {
            newPortList.emplace_back(other);
        }
    }
    /* 获取不存在的接口 */
    for (const auto& port : ports)
    {
        bool existFlag = false;
        for (const auto& other : portList)
        {
            if (port == other)
            {
                existFlag = true;
                break;
            }
        }
        if (!existFlag)
        {
            notPortList.emplace_back(port);
        }
    }
    /* 差异判断 */
    if (newList)
    {
        *newList = newPortList;
    }
    if (notList)
    {
        *notList = notPortList;
    }
    if (newPortList.empty() && notPortList.empty()) /* 无差异 */
    {
        return false;
    }
    return true;
}

bool NetConfig::BridgeInfo::operator==(const NetConfig::BridgeInfo& other) const
{
    if (other.name == name && !diffPorts(other.ports))
    {
        return true;
    }
    return false;
}

bool NetConfig::BridgeInfo::operator!=(const NetConfig::BridgeInfo& other) const
{
    if (other.name != name || diffPorts(other.ports))
    {
        return true;
    }
    return false;
}

bool NetConfig::isNetcardExist(const std::string& name)
{
    if (name.empty())
    {
        return false;
    }
    auto interfaceList = utility::Net::getAllInterfaces();
    for (const auto& iface : interfaceList)
    {
        if (name == iface.name)
        {
            return true;
        }
    }
    return false;
}

bool NetConfig::isNetcardUp(const std::string& name)
{
    if (name.empty())
    {
        return false;
    }
#ifdef _WIN32
    return false;
#else
    std::vector<std::string> outVec;
    auto command = "ethtool " + name + " | grep 'Link detected:'";
    utility::System::runCmd(command, nullptr, &outVec, true);
    if (1 == outVec.size() && utility::StrTool::contains(outVec[0], "yes", false))
    {
        return true;
    }
    return false;
#endif
}

std::vector<NetConfig::BridgeInfo> NetConfig::getBridgeInfos()
{
    std::vector<BridgeInfo> bridgeList;
#ifdef _WIN32
#else
    std::vector<std::string> outVec;
    utility::System::runCmd("brctl show", nullptr, &outVec, true);
    BridgeInfo bi;
    for (auto line : outVec)
    {
        /* 把非可显ASCII字符替换为空格 */
        for (size_t i = 0; i < line.size(); ++i)
        {
            char ch = line[i];
            if (ch < 32 || 127 == ch) /* 非可显字符 */
            {
                line[i] = ' ';
            }
        }
        /* 去除左右空格, 去除多余的空格 */
        utility::StrTool::trimLeftRight(line, ' ');
        utility::StrTool::trimDuplicate(line, ' ');
        /* 截取行各段信息 */
        auto vec = utility::StrTool::split(line, " ");
        if (1 == vec.size()) /* 只有1段, 可以视为网桥接口 */
        {
            if (bi.isValid())
            {
                bi.ports.emplace_back(vec[0]);
            }
        }
        else if (vec.size() > 1) /* 大于1段, 视为找到新网桥 */
        {
            /* 保存上次找到的网桥信息 */
            if (bi.isValid())
            {
                bridgeList.emplace_back(bi);
            }
            /* 设置新的网桥信息 */
            bi.reset();
            bi.name = vec[0];
            if (4 == vec.size())
            {
                bi.ports.emplace_back(vec[vec.size() - 1]);
            }
        }
    }
    if (bi.isValid())
    {
        bridgeList.emplace_back(bi);
    }
#endif
    return bridgeList;
}

std::vector<utility::Net::IfaceInfo> NetConfig::getEthernetCards()
{
    /* 获取网桥, 网卡 */
    auto bridgeList = getBridgeInfos();
    std::vector<utility::Net::IfaceInfo> netcardList;
    auto interfaceList = utility::Net::getAllInterfaces();
    for (const auto& iface : interfaceList)
    {
        /* 判断是否以太网 */
        if (utility::Net::IfaceInfo::Type::ethernet != iface.type) /* 非以太网 */
        {
            continue;
        }
        /* 判断是否网桥 */
        bool bridgeFlag = false;
        for (const auto& bi : bridgeList)
        {
            if (iface.name == bi.name)
            {
                bridgeFlag = true;
                break;
            }
        }
        if (bridgeFlag)
        {
            continue;
        }
        netcardList.emplace_back(iface);
    }
    /* 排序(网卡地址小的排在前面) */
    std::sort(netcardList.begin(), netcardList.end(), [](utility::Net::IfaceInfo a, utility::Net::IfaceInfo b) {
        return utility::StrTool::toLower(utility::StrTool::join(a.mac, ":"))
               < utility::StrTool::toLower(utility::StrTool::join(b.mac, ":"));
    });
    return netcardList;
}

bool NetConfig::configEthernetCardName(const std::map<std::string, std::string>& macNameMap)
{
    struct ModifyInfo
    {
        utility::Net::IfaceInfo iface; /* 原始的信息 */
        std::string name; /* 新的网卡名 */
    };
    if (macNameMap.empty())
    {
        return false;
    }
#ifdef _WIN32
    return false;
#else
    auto interfaceList = getEthernetCards();
    /* 设置网卡新名称的持久化字符串, 使用命令暂时重命名网卡 */
    static const std::string TMP_PREFIX = "tmp_";
    std::vector<ModifyInfo> modifyList;
    for (const auto& kv : macNameMap)
    {
        const auto& mac = kv.first;
        const auto& name = kv.second;
        for (const auto& iface : interfaceList)
        {
            auto macFmt1 = utility::StrTool::join(iface.mac, ":"), macFmt2 = utility::StrTool::join(iface.mac, "-");
            if (utility::StrTool::equal(mac, macFmt1, false) || utility::StrTool::equal(mac, macFmt2, false))
            {
                if (!name.empty() && name != iface.name) /* 网卡名有变更才进行命令操作 */
                {
                    ModifyInfo mi;
                    mi.iface = iface;
                    mi.name = name;
                    modifyList.emplace_back(mi);
                    /* 使用命令重命名(暂时重命名为临时名称, 避免名称冲突), 不然要使配置文件生效的话需要重启 */
                    auto command = "ip link set " + iface.name + " down && ip link set " + iface.name + " name " + TMP_PREFIX + name;
                    utility::System::runCmd(command);
                }
                break;
            }
        }
    }
    if (modifyList.empty())
    {
        return false;
    }
    /* 使用命令操作 */
    for (const auto& mi : modifyList)
    {
        /* step1: 修改网卡名(命名为正式名称) */
        std::string command = "ip link set " + TMP_PREFIX + mi.name + " name " + mi.name;
        /* step2: 启动网卡 */
        command += " && ip link set " + mi.name + " up";
        /* step3: 同步IP地址, 子网掩码, 广播地址 */
        if (mi.iface.ipv4.empty())
        {
            command += " && ip addr flush dev " + mi.name;
        }
        else
        {
            command += " && ifconfig " + mi.name + " " + mi.iface.ipv4;
            if (!mi.iface.netmask.empty())
            {
                command += " netmask " + mi.iface.netmask;
            }
            if (!mi.iface.broadcast.empty())
            {
                command += " broadcast " + mi.iface.broadcast;
            }
        }
        utility::System::runCmd(command);
    }
    return true;
#endif
}

bool NetConfig::configBridge(const std::string& name, const std::vector<std::string>& ports, bool recfg)
{
    if (name.empty())
    {
        return false;
    }
#ifdef _WIN32
    return false;
#else
    bool existBridge = false;
    std::vector<std::string> addList;
    /* step1: 判断当前网桥 */
    auto bridgeList = getBridgeInfos();
    for (const auto& bi : bridgeList)
    {
        if (name == bi.name) /* 存在网桥 */
        {
            existBridge = true;
            std::vector<std::string> delList;
            if (!bi.diffPorts(ports, &addList, &delList)) /* 网桥端口无差异 */
            {
                if (recfg)
                {
                    addList = delList = ports;
                }
                else
                {
                    return false;
                }
            }
            /* 删除网络接口 */
            for (const auto& port : delList)
            {
                utility::System::runCmd("brctl delif " + bi.name + " " + port);
            }
        }
        else /* 其他网桥 */
        {
            for (const auto& port1 : bi.ports) /* 判断网络接口是否被当前网桥占用 */
            {
                for (const auto& port2 : ports)
                {
                    if (port1 == port2) /* 被占用, 删除网络接口 */
                    {
                        utility::System::runCmd("brctl delif " + bi.name + " " + port1);
                        break;
                    }
                }
            }
        }
    }
    /* step2: 添加网桥/网络接口 */
    if (!existBridge)
    {
        utility::System::runCmd("brctl addbr " + name);
        addList = ports;
    }
    for (const auto& port : addList)
    {
        utility::System::runCmd("brctl addif " + name + " " + port);
    }
    return true;
#endif
}

bool NetConfig::checkPing(const std::string& src, const std::string& dest, int timeout, std::string* result)
{
    if (result)
    {
        result->clear();
    }
    if (dest.empty())
    {
        return false;
    }
    std::string command;
    command += "ping";
    /* 设置次数 */
#ifdef _WIN32
    command += " -n 1";
#else
    command += " -c 1";
#endif
    /* 设置超时(秒) */
    if (timeout > 0)
    {
        command += " -w " + std::to_string(timeout);
    }
    /* 设置本机要使用的网络接口 */
    if (!src.empty())
    {
#ifdef _WIN32
        command += " -S " + src;
#else
        command += " -I " + src;
#endif
    }
    /* 设置目标地址 */
    command += " " + dest;
    /* 执行和解析 */
    std::string outStr; /* 这里需要读取输出, 否则Linux下会失败 */
    if (0 != utility::System::runCmd(command, &outStr))
    {
        if (result)
        {
            *result = outStr;
        }
        return false;
    }
    if (result)
    {
        *result = outStr;
    }
    if (std::string::npos != outStr.rfind("100%") || std::string::npos == outStr.rfind("0%")) /* 数据包丢失 */
    {
        return false;
    }
    return true;
}
} // namespace toolkit
