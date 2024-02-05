#include "ftp_handler.h"

#include <stdio.h>

#include "filter.h"
#include "print.h"

/**
 * @brief 打印FTP控制请求
 */
void printFtpCtrlReq(const std::string& flag, const std::string& param)
{
    printf("            ----- FTP [request] -----\n");
    printf("            %s: %s\n", (flag[0] >= 'A' && flag[0] <= 'Z') ? "cmd" : "code", flag.c_str());
    if (param.empty())
    {
        return;
    }
    printf("            param: %s\n", param.c_str());
}

/**
 * @brief 打印FTP控制应答
 */
void printFtpCtrlResp(const std::string& flag, const std::string& param)
{
    printf("            ----- FTP [response] -----\n");
    printf("            %s: %s\n", (flag[0] >= 'A' && flag[0] <= 'Z') ? "cmd" : "code", flag.c_str());
    if (param.empty())
    {
        return;
    }
    printf("            param: %s\n", param.c_str());
}

/**
 * @brief 打印FTP数据
 */
void printFtpData(const npacket::FtpParser::CtrlInfo& ctrl, const npacket::FtpParser::DataFlag& flag, const uint8_t* data, uint32_t dataLen)
{
    std::string modeDesc = npacket::FtpParser::DataMode::active == ctrl.mode ? "PORT" : "PASV";
    if (npacket::FtpParser::DataFlag::ready == flag)
    {
        printf("            ----- FTP-DATA [%s][start][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), ctrl.clientIp.c_str(),
               ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
    }
    else if (npacket::FtpParser::DataFlag::body == flag)
    {
        printf("            ----- FTP-DATA [%s][%d][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), dataLen,
               ctrl.clientIp.c_str(), ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
        printf("%s\n", std::string(data, data + dataLen).c_str());
    }
    else if (npacket::FtpParser::DataFlag::finish == flag)
    {
        printf("            ----- FTP-DATA [%s][finish][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), ctrl.clientIp.c_str(),
               ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
    }
    else if (npacket::FtpParser::DataFlag::abnormal == flag)
    {
        printf("            ----- FTP-DATA [%s][timeout][client: %s:%d][server: %s:%d] -----\n", modeDesc.c_str(), ctrl.clientIp.c_str(),
               ctrl.clientPort, ctrl.serverIp.c_str(), ctrl.serverPort);
    }
}

void handleApplicationFtpCtrlReq(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                 const std::shared_ptr<npacket::ProtocolHeader>& header, const std::string& flag, const std::string& param)
{
    if (Filter::getInstance().showFtp())
    {
        printTransportHeader(header);
        printFtpCtrlReq(flag, param);
    }
}

void handleApplicationFtpCtrlResp(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                  const std::shared_ptr<npacket::ProtocolHeader>& header, const std::string& flag, const std::string& param)
{
    if (Filter::getInstance().showFtp())
    {
        printTransportHeader(header);
        printFtpCtrlResp(flag, param);
    }
}

void handleApplicationFtpData(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                              const std::shared_ptr<npacket::ProtocolHeader>& header, const npacket::FtpParser::CtrlInfo& ctrl,
                              const npacket::FtpParser::DataFlag& flag, const uint8_t* data, uint32_t dataLen)
{
    if (Filter::getInstance().showFtpData())
    {
        printTransportHeader(header);
        printFtpData(ctrl, flag, data, dataLen);
    }
}
