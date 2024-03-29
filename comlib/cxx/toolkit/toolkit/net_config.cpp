#include "net_config.h"

#include <algorithm>

#include "utility/filesystem/file_info.h"
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
    for (size_t i = 0; i < portList.size(); ++i)
    {
        bool existFlag = false;
        for (size_t j = 0; j < ports.size(); ++j)
        {
            if (portList[i] == ports[j])
            {
                existFlag = true;
                break;
            }
        }
        if (!existFlag)
        {
            newPortList.emplace_back(portList[i]);
        }
    }
    /* 获取不存在的接口 */
    for (size_t i = 0; i < ports.size(); ++i)
    {
        bool existFlag = false;
        for (size_t j = 0; j < portList.size(); ++j)
        {
            if (ports[i] == portList[j])
            {
                existFlag = true;
                break;
            }
        }
        if (!existFlag)
        {
            notPortList.emplace_back(ports[i]);
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
    for (size_t i = 0; i < interfaceList.size(); ++i)
    {
        if (0 == name.compare(interfaceList[i].name))
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
    for (size_t i = 1, len = outVec.size(); i < len; ++i)
    {
        auto line = outVec[i];
        /* 把非可显ASCII字符替换为空格 */
        for (size_t k = 0; k < line.size(); ++k)
        {
            char ch = line[k];
            if (ch < 32 || 127 == ch) /* 非可显字符 */
            {
                line[k] = ' ';
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
    for (size_t i = 0; i < interfaceList.size(); ++i)
    {
        const auto& iface = interfaceList[i];
        /* 判断是否以太网 */
        if (utility::Net::IfaceInfo::Type::ethernet != iface.type) /* 非以太网 */
        {
            continue;
        }
        /* 判断是否网桥 */
        bool bridgeFlag = false;
        for (size_t j = 0; j < bridgeList.size(); ++j)
        {
            if (0 == iface.name.compare(bridgeList[j].name))
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
    std::vector<std::string> lineList;
    std::vector<ModifyInfo> modifyList;
    for (auto iter = macNameMap.begin(); macNameMap.end() != iter; ++iter)
    {
        const auto& mac = iter->first;
        const auto& name = iter->second;
        for (size_t j = 0; j < interfaceList.size(); ++j)
        {
            const auto& iface = interfaceList[j];
            auto macFmt1 = utility::StrTool::toLower(utility::StrTool::join(iface.mac, ":"));
            auto macFmt2 = utility::StrTool::toLower(utility::StrTool::join(iface.mac, "-"));
            if (utility::StrTool::equal(mac, macFmt1, false) || utility::StrTool::equal(mac, macFmt2, false))
            {
                /* 设置配置行字符串 */
                auto line = "SUBSYSTEM==\"net\", ACTION==\"add\", ATTR{address}==\"" + macFmt1 + "\", NAME=\"" + name + "\"";
                lineList.emplace_back(line);
                if (0 != iface.name.compare(name)) /* 网卡名有变更才进行命令操作 */
                {
                    ModifyInfo mi;
                    mi.iface = iface;
                    mi.name = name;
                    modifyList.emplace_back(mi);
                    /* 使用命令重命名(暂时重命名为临时名称, 避免名称冲突), 不然要使配置文件生效的话需要重启 */
                    auto command = "ip link set " + iface.name + " down && ip link set " + iface.name + " name " + TMP_PREFIX + name;
                    utility::System::runCmd(command);
                }
            }
        }
    }
    if (modifyList.empty())
    {
        return false;
    }
    /* 写配置文件, 注意: 该文件修改完需要重启系统或网络才能生效 */
    utility::FileInfo("/etc/udev/rules.d/70-persistent-net.rules").write(utility::StrTool::join(lineList, "\n"));
    /* 使用命令操作 */
    for (size_t i = 0; i < modifyList.size(); ++i)
    {
        const auto& mi = modifyList[i];
        /* step1: 修改网卡名(命名为正式名称) */
        std::string command = "ip link set " + TMP_PREFIX + mi.name + " name " + mi.name;
        /* step2: 启动网卡 */
        command += " && ip link set " + mi.name + " up";
        /* step3: 同步IP地址, 子网掩码, 广播地址 */
        if (mi.iface.ipv4.empty())
        {
            command += " && ip addr flush dev  " + mi.name;
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
    for (size_t i = 0; i < bridgeList.size(); ++i)
    {
        const auto& bridgeInfo = bridgeList[i];
        if (0 == name.compare(bridgeInfo.name)) /* 存在网桥 */
        {
            existBridge = true;
            std::vector<std::string> delList;
            if (!bridgeInfo.diffPorts(ports, &addList, &delList)) /* 网桥端口无差异 */
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
            for (size_t j = 0; j < delList.size(); ++j)
            {
                utility::System::runCmd("brctl delif " + bridgeInfo.name + " " + delList[j]);
            }
        }
        else /* 其他网桥 */
        {
            for (size_t j = 0; j < bridgeInfo.ports.size(); ++j) /* 判断网络接口是否被当前网桥占用 */
            {
                for (size_t k = 0; k < ports.size(); ++k)
                {
                    if (0 == bridgeInfo.ports[j].compare(ports[k])) /* 被占用, 删除网络接口 */
                    {
                        utility::System::runCmd("brctl delif " + bridgeInfo.name + " " + bridgeInfo.ports[j]);
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
    for (size_t i = 0; i < addList.size(); ++i)
    {
        utility::System::runCmd("brctl addif " + name + " " + addList[i]);
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
