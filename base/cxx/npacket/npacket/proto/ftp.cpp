#include "ftp.h"

#include <math.h>
#include <unordered_map>

namespace npacket
{
/**
 * @brief 命令列表(https://www.rfc-editor.org/rfc/rfc5797), key-命令, value-是否需要参数(暂时不用)
 */
const std::unordered_map<std::string, int> CMD_MAP = {
    {"ABOR", 0}, /* 放弃 */
    {"ACCT", 1}, /* 账户 */
    {"ADAT", 1}, /* 认证/安全数据 */
    {"ALLO", 1}, /* 分配 */
    {"APPE", 1}, /* 追加(创建) */
    {"AUTH", 1}, /* 认证/安全机制 */
    {"AUTH+", 1}, /* 认证/安全机制 */
    {"CCC", 1}, /* 清除命令通道 */
    {"CDUP", 0}, /* 回到上层目录 */
    {"CONF", 1}, /* 受保护的机密命令 */
    {"CWD", 1}, /* 改变工作目录 */
    {"DELE", 1}, /* 删除文件 */
    {"ENC", 1}, /* 受保护的私有命令 */
    {"EPRT", 1}, /* 扩展端口 */
    {"EPSV", 1}, /* 扩展被动模式 */
    {"FEAT", 0}, /* 功能协商 */
    {"HELP", 1}, /* 帮助 */
    {"LANG", 1}, /* 语言(适用于服务器消息) */
    {"LIST", 1}, /* 列表 */
    {"LPRT", 1}, /* 数据端口{FOOBAR} */
    {"LPSV", 1}, /* 被动模式{FOOBAR} */
    {"MDTM", 1}, /* 文件修改时间 */
    {"MIC", 1}, /* 完全受保护的命令 */
    {"MKD", 1}, /* 创建目录 */
    {"MLSD", 0}, /* 列出目录(用于机器) */
    {"MLST", 0}, /* 列表单个对象 */
    {"MODE", 1}, /* 传输模式 */
    {"NLST", 1}, /* 名字列表 */
    {"NOOP", 0}, /* 空操作 */
    {"OPTS", 1}, /* 选项 */
    {"PASS", 1}, /* 密码 */
    {"PASV", 0}, /* 被动模式 */
    {"PBSZ", 1}, /* 保护缓冲区大小 */
    {"PBSZ+", 1}, /* 保护缓冲区大小 */
    {"PORT", 1}, /* 数据端口 */
    {"PROT", 1}, /* 数据通道保护级别 */
    {"PROT+", 1}, /* 数据通道保护级别 */
    {"PWD", 0}, /* 打印工作目录 */
    {"QUIT", 0}, /* 退出登录 */
    {"REIN", 0}, /* 重新启动 */
    {"REST", 1}, /* 重新启动 */
    {"REST+", 1}, /* 重新启动(适用于STREAM模式) */
    {"RETR", 1}, /* 下载文件 */
    {"RMD", 1}, /* 删除目录 */
    {"RNFR", 1}, /* 重命名 */
    {"RNTO", 1}, /* 重命名为 */
    {"SITE", 1}, /* 站点参数 */
    {"SIZE", 1}, /* 文件大小 */
    {"SMNT", 1}, /* 结构加载 */
    {"STAT", 1}, /* 状态 */
    {"STOR", 1}, /* 上传文件 */
    {"STOU", 0}, /* 唯一保存 */
    {"STRU", 1}, /* 文件结构 */
    {"SYST", 0}, /* 系统 */
    {"TYPE", 1}, /* 表示类型 */
    {"USER", 1}, /* 用户名 */
    {"XCUP", 0}, /* 回到上层目录 */
    {"XCWD", 1}, /* 改变工作目录 */
    {"XMKD", 1}, /* 创建目录 */
    {"XPWD", 1}, /* 打印工作目录 */
    {"XRMD", 1}, /* 删除目录 */
    {"-N/A-", 0} /* 虚拟文件存储 */
};

/**
 * @brief 代码列表, key-代码, value-参数(暂时不用)
 */
const std::unordered_map<std::string, int> CODE_MAP = {
    {"110", 0}, /* 重新启动标记回复, 格式必须是"MARK yyyy = mmmm", yyyy是用户进程数据流标记, mmmm是服务器的等效标记(注意"="前后空格) */
    {"120", 0}, /* 服务在N分钟内准备就绪 */
    {"125", 0}, /* 数据连接已打开, 开始传送数据 */
    {"150", 0}, /* 文件状态正常, 正在打开数据连接 */
    {"200", 0}, /* 命令执行成功 */
    {"202", 0}, /* 命令未执行, 本站点多余 */
    {"211", 0}, /* 系统状态或系统帮助回复 */
    {"212", 0}, /* 目录状态 */
    {"213", 0}, /* 文件状态 */
    {"214", 0}, /* 帮助信息, 关于如何使用服务器或特定非标准命令的含义, 此回复仅对人类用户有用 */
    {"215", 0}, /* NAME系统类型, 其中NAME是Assigned Numbers文档中列表中的正式系统名称 */
    {"220", 0}, /* 为新用户准备好服务 */
    {"221", 0}, /* 服务关闭控制连接, 注销(如果合适的话) */
    {"225", 0}, /* 数据连接打开, 没有正在进行的传输 */
    {"226", 0}, /* 正在关闭数据连接, 请求的文件操作成功(例如: 文件传输或文件中止) */
    {"227", 0}, /* 进入被动模式(IP地址, IP端口) */
    {"230", 0}, /* 用户已登录, 继续 */
    {"250", 0}, /* 请求文件操作完毕 */
    {"257", 0}, /* 创建"PATHNAME"路径名 */
    {"331", 0}, /* 用户名正确，需要密码 */
    {"332", 0}, /* 需要帐号登录 */
    {"350", 0}, /* 请求的文件操作等待进一步信息 */
    {"421", 0}, /* 服务不可用, 关闭控制连接, 如果服务知道它必须关闭, 这可能是对任何命令的应答 */
    {"425", 0}, /* 无法打开数据连接 */
    {"426", 0}, /* 连接关闭, 传输中止 */
    {"450", 0}, /* 请求的文件操作未执行, 文件不可用(例如: 文件忙) */
    {"451", 0}, /* 请求的操作中止: 处理中发生本地错误 */
    {"452", 0}, /* 请求的操作未被执行: 系统中存储空间不足 */
    {"500", 0}, /* 语法错误, 命令无法识别, 这可能包括命令行过长等错误 */
    {"501", 0}, /* 参数或参数中的语法错误 */
    {"502", 0}, /* 命令未执行 */
    {"503", 0}, /* 命令顺序错误 */
    {"504", 0}, /* 无效命令参数, 此参数下的命令未实现  */
    {"530", 0}, /* 未登录: 账号或密码错误 */
    {"532", 0}, /* 存储文件需要帐号 */
    {"550", 0}, /* 请求的操作未被执行, 文件不可用(例如: 找不到文件, 无法访问) */
    {"551", 0}, /* 请求的操作中止: 页类型未知 */
    {"552", 0}, /* 请求的文件操作中止, 超出存储分配(用于当前目录或数据集) */
    {"553", 0} /* 请求的操作未被执行, 文件名不合法 */
};

/**
 * @brief IPv4字符串转4字节二进制数组
 */
bool str2IPv4(const std::string& str, uint8_t out[4])
{
    int a, b, c, d;
    if (std::sscanf(str.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
    {
        return false;
    }
    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255)
    {
        return false;
    }
    out[0] = static_cast<uint8_t>(a);
    out[1] = static_cast<uint8_t>(b);
    out[2] = static_cast<uint8_t>(c);
    out[3] = static_cast<uint8_t>(d);
    return true;
}

/**
 * @brief IPv6字符串转16字节二进制数组, 支持格式: 2001:0db8:85a3:0000:0000:8a2e:0370:7334, 压缩格式::
 */
bool str2IPv6(const std::string& str, uint8_t out[16])
{
    memset(out, 0, 16);
    uint16_t segments[8] = {0};
    int segIdx = 0;
    int zeroCompressPos = -1;
    size_t pos = 0;
    size_t len = str.size();
    for (size_t i = 0; i + 1 < len; ++i)
    {
        if (':' == str[i] && ':' == str[i + 1])
        {
            zeroCompressPos = segIdx;
            break;
        }
    }
    while (pos < len && segIdx < 8)
    {
        if (':' == str[pos])
        {
            pos++;
            continue;
        }
        char buf[5] = {0};
        int bufIdx = 0;
        while (pos < len && ':' != str[pos] && bufIdx < 4)
        {
            buf[bufIdx++] = str[pos++];
        }
        uint16_t val = 0;
        std::sscanf(buf, "%hx", &val);
        segments[segIdx++] = val;
        if (pos < len && ':' == str[pos])
        {
            pos++;
        }
    }
    if (-1 != zeroCompressPos)
    {
        int moveCnt = segIdx - zeroCompressPos;
        for (int i = 7; i >= zeroCompressPos + moveCnt; --i)
        {
            segments[i] = segments[i - moveCnt];
        }
        for (int i = zeroCompressPos; i < 8 - moveCnt; ++i)
        {
            segments[i] = 0;
        }
    }
    for (int i = 0; i < 8; ++i)
    {
        out[i * 2] = (uint8_t)((segments[i] >> 8) & 0xFF);
        out[i * 2 + 1] = (uint8_t)(segments[i] & 0xFF);
    }
    return true;
}

FtpParser::FtpParser(uint32_t dcTimeout) : m_dataConnectTimeout(dcTimeout > 0 ? dcTimeout : 15) {}

uint32_t FtpParser::getProtocol() const noexcept
{
    return ApplicationProtocol::FTP;
}

ParseResult FtpParser::parse(const ProtocolData& pd, uint32_t& consumeLen)
{
    consumeLen = 0;
    recyleDataConnect(pd.ntp);
    if (!pd.header || !pd.header->parent || TransportProtocol::TCP != pd.header->getProtocol()
        || (NetworkProtocol::IPv4 != pd.header->parent->getProtocol() && NetworkProtocol::IPv6 != pd.header->parent->getProtocol()))
    {
        return ParseResult::FAILURE;
    }
    if (parseData(pd.ntp, pd.totalLen, pd.header, pd.payload, pd.payloadLen))
    {
        consumeLen = pd.payloadLen;
        return ParseResult::SUCCESS;
    }
    if (pd.payloadLen >= 5 && '\r' == pd.payload[pd.payloadLen - 2]
        && '\n' == pd.payload[pd.payloadLen - 1]) /* FTP控制包最小5个字节且都以'\r\n'结尾 */
    {
        if (parseRequest(pd.ntp, pd.totalLen, pd.header, pd.payload, pd.payloadLen))
        {
            consumeLen = pd.payloadLen;
            return ParseResult::SUCCESS;
        }
        else if (parseResponse(pd.ntp, pd.totalLen, pd.header, pd.payload, pd.payloadLen))
        {
            consumeLen = pd.payloadLen;
            return ParseResult::SUCCESS;
        }
    }
    return ParseResult::FAILURE;
}

void FtpParser::setRequestCallback(const CTRL_PKT_CALLBACK& callback)
{
    m_requestCb = callback;
}

void FtpParser::setResponseCallback(const CTRL_PKT_CALLBACK& callback)
{
    m_responseCb = callback;
}

void FtpParser::setDataCallback(const DATA_PKT_CALLBACK& callback)
{
    m_dataCb = callback;
}

bool FtpParser::parseRequest(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                             const uint8_t* payload, uint32_t payloadLen)
{
    auto ch0 = payload[0], ch1 = payload[1], ch2 = payload[2];
    if ((ch0 >= 'A' && ch0 <= 'Z') && (ch1 >= 'A' && ch1 <= 'Z') && (ch2 >= 'A' && ch2 <= 'Z')) /* 疑似请求包 */
    {
        std::string cmd, arg;
        bool cmdFlag = true;
        /* 解析命令和参数 */
        for (uint32_t i = 0; i < payloadLen - 2; ++i)
        {
            auto ch = payload[i];
            if ('\r' == ch || '\n' == ch)
            {
                return false;
            }
            if (' ' == ch)
            {
                if (cmdFlag)
                {
                    cmdFlag = false;
                }
                else
                {
                    arg.push_back(ch);
                }
            }
            else
            {
                if (cmdFlag)
                {
                    cmd.push_back(ch);
                }
                else
                {
                    arg.push_back(ch);
                }
            }
        }
        if (CMD_MAP.end() == CMD_MAP.find(cmd)) /* 未匹配到命令 */
        {
            return false;
        }
        if (m_requestCb)
        {
            m_requestCb(ntp, totalLen, header, cmd, arg);
        }
        if ("PORT" == cmd) /* 主动模式, 解析客户端数据端口(服务端数据端口=服务端控制端口-1) */
        {
            std::string ip;
            uint16_t port;
            if (parseDataPort(arg, ip, port))
            {
                handleDataPort(ntp, header, DataMode::ACTIVE, ip, port);
                return true;
            }
            return false;
        }
        return true;
    }
    return false;
}

bool FtpParser::parseResponse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                              const uint8_t* payload, uint32_t payloadLen)
{
    if (payloadLen < 7)
    {
        return false;
    }
    auto ch0 = payload[0], ch1 = payload[1], ch2 = payload[2], ch3 = payload[3];
    if ((ch0 >= '1' && ch0 <= '5') && (ch1 >= '0' && ch1 <= '5') && (ch2 >= '0' && ch2 <= '7')
        && (' ' == ch3 || '-' == ch3)) /* 疑似应答包 */
    {
        std::string code, arg;
        bool codeFlag = true;
        /* 解析代码和参数 */
        for (uint32_t i = 0; i < payloadLen - 2; ++i)
        {
            auto ch = payload[i];
            if (('\r' == ch && '\n' != payload[i + 1]) || ('\n' == ch))
            {
                return false;
            }
            if (' ' == ch || '-' == ch)
            {
                if (codeFlag)
                {
                    codeFlag = false;
                }
                else
                {
                    arg.push_back(ch);
                }
            }
            else
            {
                if (codeFlag)
                {
                    code.push_back(ch);
                }
                else
                {
                    arg.push_back(ch);
                }
            }
        }
        if (CODE_MAP.end() == CODE_MAP.find(code)) /* 未匹配到代码 */
        {
            return false;
        }
        if (m_responseCb)
        {
            m_responseCb(ntp, totalLen, header, code, arg);
        }
        if ("227" == code) /* 被动模式, 解析服务端数据端口 */
        {
            auto bp = arg.find("(");
            auto ep = arg.rfind(")");
            if (std::string::npos != bp && std::string::npos != ep && bp < ep)
            {
                std::string ip;
                uint16_t port;
                if (parseDataPort(arg.substr(bp + 1, ep - 1 - bp), ip, port))
                {
                    handleDataPort(ntp, header, DataMode::PASSIVE, ip, port);
                    return true;
                }
            }
            return false;
        }
        return true;
    }
    return false;
}

bool FtpParser::parseDataPort(const std::string& ip_port, std::string& ip, uint16_t& port)
{
    ip.clear();
    port = 0;
    std::string value;
    int index = 0;
    for (size_t i = 0; i < ip_port.size(); ++i)
    {
        const auto& ch = ip_port[i];
        if ((ch >= '0' && ch <= '9') || (',' == ch && i > 0 && i < ip_port.size() - 1))
        {
            if (',' != ch)
            {
                value.push_back(ch);
                if (i < ip_port.size() - 1)
                {
                    continue;
                }
            }
            if (value.size() > 0)
            {
                auto num = std::atoi(value.c_str());
                if (num >= 0 && num <= 255)
                {
                    ++index;
                    if (index <= 4)
                    {
                        ip += ((1 == index) ? value : ("." + value));
                    }
                    else if (index <= 6)
                    {
                        port += ((5 == index) ? (num * 256) : num);
                    }
                    value.clear();
                    continue;
                }
            }
        }
        return false;
    }
    if (6 != index)
    {
        return false;
    }
    return true;
}

void FtpParser::handleDataPort(const std::chrono::steady_clock::time_point& ntp, const ProtocolHeader* header, const DataMode& mode,
                               const std::string& ip, uint16_t port)
{
    IpPortKey key;
    uint8_t ip4[4] = {0};
    uint8_t ip6[16] = {0};
    if (str2IPv4(ip, ip4))
    {
        key = IpPortKey::createIpv4(ip4, port);
    }
    else if (str2IPv6(ip, ip6))
    {
        key = IpPortKey::createIpv6(ip6, port);
    }
    else
    {
        return;
    }
    if (header && m_dataConnectList.end() == m_dataConnectList.find(key))
    {
        std::string srcMac, dstMac, srcAddr, dstAddr;
        if (header->parent)
        {
            if (header->parent->parent)
            {
                auto ethHeader = (const EthernetIIHeader*)(header->parent->parent);
                ethHeader->srcMacStr(srcMac);
                ethHeader->dstMacStr(dstMac);
            }
            if (NetworkProtocol::IPv4 == header->parent->getProtocol())
            {
                auto ipv4Header = (const Ipv4Header*)(header->parent);
                ipv4Header->srcAddrStr(srcAddr);
                ipv4Header->dstAddrStr(dstAddr);
            }
            else if (NetworkProtocol::IPv6 == header->parent->getProtocol())
            {
                auto ipv6Header = (const Ipv6Header*)(header->parent);
                ipv6Header->srcAddrStr(srcAddr);
                ipv6Header->dstAddrStr(dstAddr);
            }
        }
        auto tcpHeader = (const TcpHeader*)(header);
        auto dci = std::make_shared<DataConnectInfo>();
        if (DataMode::ACTIVE == mode) /* 主动模式 */
        {
            dci->ctrl.clientMac = srcMac;
            dci->ctrl.clientIp = srcAddr;
            dci->ctrl.clientPort = tcpHeader->srcPort;
            dci->ctrl.serverMac = dstMac;
            dci->ctrl.serverIp = dstAddr;
            dci->ctrl.serverPort = tcpHeader->dstPort;
        }
        else /* 被动模式 */
        {
            dci->ctrl.clientMac = dstMac;
            dci->ctrl.clientIp = dstAddr;
            dci->ctrl.clientPort = tcpHeader->dstPort;
            dci->ctrl.serverMac = srcMac;
            dci->ctrl.serverIp = srcAddr;
            dci->ctrl.serverPort = tcpHeader->srcPort;
        }
        dci->ctrl.mode = mode;
        dci->ip = ip;
        dci->port = port;
        dci->status = DataConnectStatus::READY;
        dci->tp = ntp;
        m_dataConnectList.insert(std::make_pair(key, dci));
    }
}

void FtpParser::recyleDataConnect(const std::chrono::steady_clock::time_point& ntp)
{
    size_t interval = ceil((float)m_dataConnectTimeout / 10); /* 使用(超时时间/10)作为清理间隔 */
    if (interval < 1)
    {
        interval = 1;
    }
    if (std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_lastRecyleTime).count() <= interval)
    {
        return;
    }
    m_lastRecyleTime = ntp;
    for (auto iter = m_dataConnectList.begin(); m_dataConnectList.end() != iter;)
    {
        if (std::chrono::duration_cast<std::chrono::seconds>(ntp - iter->second->tp).count() >= m_dataConnectTimeout) /* 超时则回收 */
        {
            auto ctrl = iter->second->ctrl;
            iter = m_dataConnectList.erase(iter);
            if (m_dataCb)
            {
                m_dataCb(ntp, 0, nullptr, ctrl, DataFlag::ABNORMAL, nullptr, 0);
            }
        }
        else
        {
            iter++;
        }
    }
}

bool FtpParser::parseData(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                          const uint8_t* payload, uint32_t payloadLen)
{
    if (!header || !header->parent)
    {
        return false;
    }
    IpPortKey srcKey, dstKey;
    auto tcpHeader = (const TcpHeader*)(header);
    if (NetworkProtocol::IPv4 == header->parent->getProtocol())
    {
        auto ipv4Header = (const Ipv4Header*)(header->parent);
        srcKey = IpPortKey::createIpv4(ipv4Header->srcAddr, tcpHeader->srcPort);
        dstKey = IpPortKey::createIpv4(ipv4Header->dstAddr, tcpHeader->dstPort);
    }
    else if (NetworkProtocol::IPv6 == header->parent->getProtocol())
    {
        auto ipv6Header = (const Ipv6Header*)(header->parent);
        uint8_t srcBytes[16], dstBytes[16];
        ipv6Header->srcAddrBytes(srcBytes);
        ipv6Header->dstAddrBytes(dstBytes);
        srcKey = IpPortKey::createIpv6(srcBytes, tcpHeader->srcPort);
        dstKey = IpPortKey::createIpv6(dstBytes, tcpHeader->dstPort);
    }
    auto iter = m_dataConnectList.find(srcKey);
    if (m_dataConnectList.end() == iter)
    {
        iter = m_dataConnectList.find(dstKey);
        if (m_dataConnectList.end() == iter)
        {
            return false;
        }
    }
    iter->second->tp = ntp;
    auto ctrl = iter->second->ctrl;
    if (0 == payloadLen)
    {
        if (1 == tcpHeader->flagAck && DataConnectStatus::READY == iter->second->status) /* 数据连接建立 */
        {
            iter->second->status = DataConnectStatus::CREATED;
            if (m_dataCb)
            {
                m_dataCb(ntp, totalLen, header, ctrl, DataFlag::READY, nullptr, 0);
            }
        }
        else if (1 == tcpHeader->flagFin && DataConnectStatus::CREATED == iter->second->status) /* 数据连接断开 */
        {
            m_dataConnectList.erase(iter);
            if (m_dataCb)
            {
                m_dataCb(ntp, totalLen, header, ctrl, DataFlag::FINISH, nullptr, 0);
            }
        }
    }
    else if (DataConnectStatus::CREATED == iter->second->status) /* 数据连接已连接 */
    {
        if (m_dataCb)
        {
            m_dataCb(ntp, totalLen, header, ctrl, DataFlag::BODY, payload, payloadLen);
        }
    }
    return true;
}
} // namespace npacket
