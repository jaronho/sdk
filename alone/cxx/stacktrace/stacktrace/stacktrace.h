#pragma once

#include <string>
#include <vector>

namespace stacktrace
{
/**
 * @brief 获取堆栈信息
 * @return 堆栈信息
 */
std::vector<std::string> getStacktrace();

#ifndef _WIN32
/**
 * @brief 获取地址详细信息
 * @return 详细信息
 */
std::string getAddressDetail(const std::string& procName, const std::string& address);
#endif
} // namespace stacktrace
