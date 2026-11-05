#include "net_config.h"

#include <algorithm>
#include <cerrno>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

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

bool NetConfig::isLinkDetected(const std::string& name)
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
        if (utility::Net::IfaceInfo::LinkType::ethernet != iface.linkType) /* 非以太网 */
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

int NetConfig::checkTcp(const std::string& src, const std::string& dest, int port, int timeout)
{
    if (dest.empty() || port <= 0 || port > 65535)
    {
        return -1;
    }
    if (timeout <= 0)
    {
        timeout = 1;
    }
    /* 平台差异定义 */
#ifdef _WIN32
    using SocketHandle = SOCKET;
    const SocketHandle INVALID_HANDLE = INVALID_SOCKET;
    const int ERR_REFUSED = WSAECONNREFUSED;
    const int ERR_TIMEDOUT = WSAETIMEDOUT;
    const int ERR_INTR = WSAEINTR;
    auto closeSocket = [](SocketHandle fd) { closesocket(fd); };
    auto lastError = []() { return (int)WSAGetLastError(); };
    auto isInProgress = [](int err) { return WSAEWOULDBLOCK == err || WSAEINPROGRESS == err; };
#else
    using SocketHandle = int;
    const SocketHandle INVALID_HANDLE = -1;
    const int ERR_REFUSED = ECONNREFUSED;
    const int ERR_TIMEDOUT = ETIMEDOUT;
    const int ERR_INTR = EINTR;
    auto closeSocket = [](SocketHandle fd) { close(fd); };
    auto lastError = []() { return errno; };
    auto isInProgress = [](int err) { return EINPROGRESS == err; };
#endif
#ifdef _WIN32
    WSADATA wsaData;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        return -4;
    }
#endif
    int probeResult = -4;
    bool timedOut = false;
    int errCode = 0;
    /* 解析目标地址 */
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; /* 同时支持IPv4和IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* addrList = nullptr;
    if (0 == getaddrinfo(dest.c_str(), std::to_string(port).c_str(), &hints, &addrList) && nullptr != addrList)
    {
        for (addrinfo* ai = addrList; nullptr != ai; ai = ai->ai_next)
        {
            SocketHandle fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            if (INVALID_HANDLE == fd)
            {
                errCode = lastError();
                continue;
            }
            /* 绑定本机要使用的网络接口 */
            if (!src.empty())
            {
                bool bound = false;
                addrinfo srcHints;
                memset(&srcHints, 0, sizeof(srcHints));
                srcHints.ai_family = ai->ai_family;
                srcHints.ai_socktype = SOCK_STREAM;
                addrinfo* srcAddr = nullptr;
                if (0 == getaddrinfo(src.c_str(), nullptr, &srcHints, &srcAddr) && nullptr != srcAddr)
                {
                    bound = (0 == bind(fd, srcAddr->ai_addr, srcAddr->ai_addrlen));
                    freeaddrinfo(srcAddr);
                }
#if !defined(_WIN32) && defined(SO_BINDTODEVICE)
                if (!bound) /* 解析为IP失败时, 尝试按网卡名绑定 */
                {
                    bound = (0 == setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, src.c_str(), (socklen_t)src.size()));
                }
#endif
                if (!bound) /* 指定的网络接口无效, 不能静默走默认网卡, 否则结果误导 */
                {
                    closeSocket(fd);
                    freeaddrinfo(addrList);
#ifdef _WIN32
                    WSACleanup();
#endif
                    return -1;
                }
            }
            /* 设置非阻塞模式, 用于实现连接超时 */
#ifdef _WIN32
            u_long nonBlock = 1;
            if (0 != ioctlsocket(fd, FIONBIO, &nonBlock))
            {
                errCode = lastError();
                closeSocket(fd);
                continue;
            }
#else
            int flags = fcntl(fd, F_GETFL, 0);
            if (flags < 0 || 0 != fcntl(fd, F_SETFL, flags | O_NONBLOCK)) /* 设置失败会退化为阻塞连接, 不能继续 */
            {
                errCode = lastError();
                closeSocket(fd);
                continue;
            }
#endif
            /* 发起非阻塞连接 */
            errCode = 0;
            bool connected = (0 == connect(fd, ai->ai_addr, ai->ai_addrlen));
            if (!connected)
            {
                errCode = lastError();
                if (isInProgress(errCode)) /* 连接进行中, 等待结果 */
                {
                    timeval tv;
                    tv.tv_sec = timeout;
                    tv.tv_usec = 0;
                    int ret = 0;
                    do
                    {
                        fd_set writeSet;
                        fd_set errSet;
                        FD_ZERO(&writeSet);
                        FD_ZERO(&errSet);
                        FD_SET(fd, &writeSet);
                        FD_SET(fd, &errSet);
                        ret = select((int)fd + 1, nullptr, &writeSet, &errSet, &tv);
                    } while (ret < 0 && ERR_INTR == lastError()); /* 被信号中断时重试 */
                    if (ret > 0) /* 连接有结果, 获取实际错误码 */
                    {
                        int soErr = 0;
                        socklen_t len = sizeof(soErr);
                        if (0 != getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&soErr, &len))
                        {
                            errCode = lastError(); /* 获取失败时不能当作连接成功 */
                        }
                        else
                        {
                            errCode = soErr;
                        }
                    }
                    else if (0 == ret) /* 等待超时 */
                    {
                        timedOut = true;
                        errCode = ERR_TIMEDOUT;
                    }
                    else
                    {
                        errCode = lastError();
                    }
                }
            }
            closeSocket(fd);
            if (0 == errCode) /* 连接成功 */
            {
                probeResult = 0;
                break;
            }
            if (ERR_REFUSED == errCode) /* 目标主机返回了RST响应, 说明主机可达, 只是该端口未开放 */
            {
                probeResult = 1;
                break;
            }
            /* 其他错误(如超时), 继续尝试下一个地址 */
        }
        freeaddrinfo(addrList);
    }
    else
    {
        probeResult = -2;
    }
    if (-4 == probeResult && timedOut) /* 至少有一次等待超时, 统一归类为超时 */
    {
        probeResult = -3;
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return probeResult;
}

} // namespace toolkit
