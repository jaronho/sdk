#pragma once
#include "npacket/proto/iec103.h"

/**
 * @brief 处理应用层IEC103固定帧
 */
void handleApplicationIec103FixedFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                       const std::shared_ptr<npacket::ProtocolHeader>& header,
                                       const std::shared_ptr<npacket::iec103::FixedFrame>& frame);

/**
 * @brief 处理应用层IEC103可变帧
 */
void handleApplicationIec103VariableFrame(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                          const std::shared_ptr<npacket::ProtocolHeader>& header,
                                          const std::shared_ptr<npacket::iec103::VariableFrame>& frame);
