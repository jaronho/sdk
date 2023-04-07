#pragma once
#include "npacket/proto/ftp.h"

/**
 * @brief 处理应用层FTP控制请求
 */
void handleApplicationFtpCtrlReq(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                 const std::shared_ptr<npacket::ProtocolHeader>& header, const std::string& flag, const std::string& param);

/**
 * @brief 处理应用层FTP控制应答
 */
void handleApplicationFtpCtrlResp(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                  const std::shared_ptr<npacket::ProtocolHeader>& header, const std::string& flag,
                                  const std::string& param);

/**
 * @brief 处理应用层FTP数据
 */
void handleApplicationFtpData(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                              const std::shared_ptr<npacket::ProtocolHeader>& header, const npacket::FtpParser::CtrlInfo& ctrl,
                              const npacket::FtpParser::DataFlag& flag, const uint8_t* data, uint32_t dataLen);
