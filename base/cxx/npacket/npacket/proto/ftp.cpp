#include "ftp.h"

#include <unordered_map>

namespace npacket
{
/**
 * @brief 命令列表, key-命令, value-是否需要参数(暂时不用)
 */
const std::unordered_map<std::string, int> CMD_MAP = {
    {"ABOR", 0}, /* 放弃 */
    {"ACCT", 1}, /* 账户 */
    {"ADAT", 1}, /* 认证/安全数据 */
    {"ALGS", 1}, /* FTP64 ALG状态 */
    {"ALLO", 1}, /* 分配 */
    {"APPE", 1}, /* 追加 */
    {"AUTH", 1}, /* 认证/安全机制 */
    {"AUTH+", 1}, /* 认证/安全机制 */
    {"CDUP", 0}, /* 回到上层目录 */
    {"CONF", 1}, /* 受保护的机密命令 */
    {"CWD", 1}, /* 进入目录 */
    {"DELE", 1}, /* 删除文件 */
    {"ENC", 1}, /* 受保护的私有命令 */
    {"FEAT", 0}, /* 功能协商 */
    {"HELP", 1}, /* 帮助 */
    {"LIST", 1}, /* 列表 */
    {"MDTM", 1}, /* 文件修改时间 */
    {"MIC", 1}, /* 完全受保护的命令 */
    {"MKD", 1}, /* 创建目录 */
    {"MODE", 1}, /* 传输模式 */
    {"NLST", 1}, /* 名字列表 */
    {"NOOP", 0}, /* 空操作 */
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
    {"USER", 1} /* 账户 */
};

/**
 * @brief 代码列表, key-代码, value-参数(暂时不用)
 */
const std::unordered_map<std::string, int> CODE_MAP = {};

uint32_t FtpParser::getProtocol() const
{
    return ApplicationProtocol::FTP;
}

bool FtpParser::parse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader, const uint8_t* payload,
                      uint32_t payloadLen)
{
    if (TransportProtocol::TCP != transportHeader->getProtocol())
    {
        return false;
    }
    if (payloadLen < 5) /* FTP包最小5个字节 */
    {
        return false;
    }
    if ('\r' != payload[payloadLen - 2] || '\n' != payload[payloadLen - 1]) /* FTP包都以'\r\n'结尾 */
    {
        return false;
    }
    if (parseRequest(totalLen, transportHeader, payload, payloadLen))
    {
        return true;
    }
    else if (parseResponse(totalLen, transportHeader, payload, payloadLen))
    {
        return true;
    }
    return false;
}

void FtpParser::setRequestCallback(const REQUEST_CALLBACK& callback)
{
    m_requestCb = callback;
}

bool FtpParser::parseRequest(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader, const uint8_t* payload,
                             uint32_t payloadLen)
{
    auto ch0 = payload[0], ch1 = payload[1], ch2 = payload[2];
    if ((ch0 >= 'A' && ch0 <= 'Z') && (ch1 >= 'A' && ch1 <= 'Z') && (ch2 >= 'A' && ch2 <= 'Z')) /* 疑似请求包 */
    {
        std::string cmd, param;
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
                    param.push_back(ch);
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
                    param.push_back(ch);
                }
            }
        }
        if (CMD_MAP.end() == CMD_MAP.find(cmd)) /* 未匹配到命令 */
        {
            return false;
        }
        if (m_requestCb)
        {
            m_requestCb(totalLen, transportHeader, cmd, param);
        }
        return true;
    }
    return false;
}

bool FtpParser::parseResponse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader, const uint8_t* payload,
                              uint32_t payloadLen)
{
    if (payloadLen < 7)
    {
        return false;
    }
    auto ch0 = payload[0], ch1 = payload[1], ch2 = payload[2], ch3 = payload[3];
    if ((ch0 >= '1' && ch0 <= '5') && (ch1 >= '0' && ch1 <= '5') && (ch2 >= '0' && ch2 <= '7')
        && (' ' == ch3 || '-' == ch3)) /* 疑似应答包 */
    {
    }
    return false;
}
} // namespace npacket
