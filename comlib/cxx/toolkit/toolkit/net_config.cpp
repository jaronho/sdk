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

bool NetConfig::BridgeInfo::checkSamePorts(const std::vector<std::string>& portList) const
{
    if (portList.size() != ports.size())
    {
        return false;
    }
    /* 判断网络接口是否一致(不需要按顺序) */
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
            return false;
        }
    }
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
            return false;
        }
    }
    return true;
}

bool NetConfig::BridgeInfo::operator==(const NetConfig::BridgeInfo& other) const
{
    if (other.name == name && checkSamePorts(other.ports))
    {
        return true;
    }
    return false;
}

bool NetConfig::BridgeInfo::operator!=(const NetConfig::BridgeInfo& other) const
{
    if (other.name != name || !checkSamePorts(other.ports))
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
    utility::System::runCmd(command, nullptr, &outVec);
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
    utility::System::runCmd("brctl show", nullptr, &outVec);
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
    std::vector<std::vector<utility::Net::IfaceInfo>> groupList;
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
        /* 查找厂商编组并插入 */
        bool findFlag = false;
        auto vendorId = utility::StrTool::join(iface.mac, ":", 3); /* 网卡厂商ID */
        for (auto iter = groupList.begin(); groupList.end() != iter; ++iter)
        {
            auto tmpInfo = (*iter)[0];
            auto tmpVendorId = utility::StrTool::join(tmpInfo.mac, ":", 3); /* 获取当前组的厂商ID */
            if (0 == vendorId.compare(tmpVendorId)) /* 厂商ID一致, 找到编组 */
            {
                iter->emplace_back(iface);
                /* 排序(网卡名小的排在前面) */
                std::sort(iter->begin(), iter->end(), [](utility::Net::IfaceInfo a, utility::Net::IfaceInfo b) { return a.name < b.name; });
                findFlag = true;
                break;
            }
        }
        if (findFlag)
        {
            continue;
        }
        /* 没有找到, 创建新编组 */
        std::vector<utility::Net::IfaceInfo> group;
        group.emplace_back(iface);
        groupList.emplace_back(group);
    }
    /* 排序(编组数量多的排在前面) */
    std::sort(groupList.begin(), groupList.end(),
              [](std::vector<utility::Net::IfaceInfo> a, std::vector<utility::Net::IfaceInfo> b) { return a.size() > b.size(); });
    /* 组成列表 */
    std::vector<utility::Net::IfaceInfo> netcardList;
    for (auto iter = groupList.begin(); groupList.end() != iter; ++iter)
    {
        for (auto it = iter->begin(); iter->end() != it; ++it)
        {
            netcardList.emplace_back(*it);
        }
    }
    return netcardList;
}

bool NetConfig::configEthernetCardName(const std::map<std::string, std::string>& macNameMap, int waitUp)
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
    /* 写配置文件 */
    auto content = utility::StrTool::join(lineList, "\n");
    utility::FileInfo fi("/etc/udev/rules.d/70-persistent-net.rules"); /* 注意: 该文件修改完需要重启系统或网络才能生效 */
    fi.write(content.c_str(), content.size());
    /* 使用命令操作 */
    for (size_t i = 0; i < modifyList.size(); ++i)
    {
        const auto& mi = modifyList[i];
        /* step1: 修改网卡名(命名为正式名称) */
        std::string command = "ip link set " + TMP_PREFIX + mi.name + " name " + mi.name;
        /* step2: 同步IP地址, 子网掩码, 广播地址 */
        if (!mi.iface.ipv4.empty())
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
        /* step3: 启动网卡 */
        command += " && ip link set " + mi.name + " up";
        utility::System::runCmd(command);
    }
    /* 延迟等待网卡启动完毕 */
    utility::System::waitForTime(waitUp, [&]() {
        for (size_t i = 0; i < modifyList.size(); ++i)
        {
            if (!isNetcardUp(modifyList[i].name)) /* 有网卡未启动 */
            {
                return false;
            }
        }
        return true;
    });
    return true;
#endif
}

bool NetConfig::configBridge(const std::string& name, const std::vector<std::string>& ports)
{
    if (name.empty())
    {
        return false;
    }
#ifdef _WIN32
    return false;
#else
    bool existBridge = false;
    /* step1: 判断当前网桥 */
    auto bridgeList = getBridgeInfos();
    for (size_t i = 0; i < bridgeList.size(); ++i)
    {
        const auto& bridgeInfo = bridgeList[i];
        if (0 == name.compare(bridgeInfo.name)) /* 存在网桥 */
        {
            if (bridgeInfo.checkSamePorts(ports)) /* 网桥端口无变化 */
            {
                return false;
            }
            existBridge = true;
            /* 删除网络接口 */
            for (size_t j = 0; j < bridgeInfo.ports.size(); ++j)
            {
                utility::System::runCmd("brctl delif " + bridgeInfo.name + " " + bridgeInfo.ports[j]);
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
    }
    for (size_t i = 0; i < ports.size(); ++i)
    {
        utility::System::runCmd("brctl addif " + name + " " + ports[i]);
    }
    /* step3: 启动网络接口 */
    for (size_t i = 0; i < ports.size(); ++i)
    {
        utility::System::runCmd("ip addr flush dev " + ports[i] + " && ip link set " + ports[i] + " up");
    }
    /* step4: 启动网桥, 使用ifdown/ifup使配置文件生效 */
    utility::System::runCmd("ip addr flush dev " + name + " && ifdown " + name + " && ifup " + name);
    return true;
#endif
}

bool NetConfig::deleteBridge(const std::string& name)
{
    if (name.empty())
    {
        return false;
    }
#ifdef _WIN32
    return false;
#else
    auto bridgeList = getBridgeInfos();
    for (size_t i = 0; i < bridgeList.size(); ++i)
    {
        const auto& bridgeInfo = bridgeList[i];
        if (0 == name.compare(bridgeInfo.name)) /* 存在网桥 */
        {
            /* 删除网络接口 */
            for (size_t j = 0; j < bridgeInfo.ports.size(); ++j)
            {
                utility::System::runCmd("brctl delif " + bridgeInfo.name + " " + bridgeInfo.ports[j]);
            }
            /* 停止网桥 */
            utility::System::runCmd("ip link set " + bridgeInfo.name + " down");
            /* 删除网桥 */
            utility::System::runCmd("brctl delbr " + bridgeInfo.name);
            return true;
        }
    }
    return false;
#endif
}

bool NetConfig::enableBridge(const std::string& name)
{
    if (name.empty())
    {
        return false;
    }
#ifdef _WIN32
    return false;
#else
    auto bridgeList = getBridgeInfos();
    for (const auto& bridgeInfo : bridgeList)
    {
        if (0 == name.compare(bridgeInfo.name)) /* 存在网桥 */
        {
            /* step1. 启动网络接口 */
            for (const auto& port : bridgeInfo.ports)
            {
                utility::System::runCmd("ip addr flush dev " + port + " && ip link set " + port + " down && ip link set " + port + " up");
            }
            /* step2. 启动网桥, 使用ifdown/ifup使配置文件生效 */
            utility::System::runCmd("ip addr flush dev " + name + " && ifdown " + name + " && ifup " + name);
            return true;
        }
    }
    return false;
#endif
}

bool NetConfig::checkPing(const std::string& src, const std::string& dest, int timeout)
{
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
    std::vector<std::string> outVec; /* 这里需要读取输出, 否则Linux下会失败 */
    if (0 != utility::System::runCmd(command, nullptr, &outVec))
    {
        return false;
    }
    /* 去除空行 */
    for (auto iter = outVec.begin(); outVec.end() != iter;)
    {
        if (iter->empty())
        {
            outVec.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
#ifdef _WIN32
    if (6 == outVec.size()) /* Windows平台下可通时, 返回6行 */
    {
        return true;
    }
#else
    if (5 == outVec.size()) /* Linux平台下可通时, 返回5行 */
    {
        return true;
    }
#endif
    return false;
}
} // namespace toolkit
