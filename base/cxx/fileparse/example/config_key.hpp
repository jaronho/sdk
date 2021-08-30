#pragma once
#include <string>

#include "../fileparse/ini/ini_helper.h"

const std::string CONFIG_FILENAME = "config.ini";
/* 整型键值对(只读) */
#define CONFIG_KV_IR(keyVar, keyName, val) MAKE_INI_KV(CONFIG_FILENAME, keyVar, keyName, std::to_string(val), "", "", true)
/* 整型键值对(读写) */
#define CONFIG_KV_IW(keyVar, keyName, val) MAKE_INI_KV(CONFIG_FILENAME, keyVar, keyName, std::to_string(val), "", "", false)
/* 字符串键值对(只读) */
#define CONFIG_KV_SR(keyVar, keyName, val) MAKE_INI_KV(CONFIG_FILENAME, keyVar, keyName, val, "", "", true)
/* 字符串键值对(读写) */
#define CONFIG_KV_SW(keyVar, keyName, val) MAKE_INI_KV(CONFIG_FILENAME, keyVar, keyName, val, "", "", false)

namespace config_key
{
CONFIG_KV_SW(APPNAME, "/appname", "测试"); /* 应用程序名称 */
CONFIG_KV_SR(VERSION, "/version", "1.0.0"); /* 应用程序版本 */

CONFIG_KV_SW(SERVER_IP, "/server/ip", "192.168.5.2"); /* 服务器地址 */
CONFIG_KV_IW(SERVER_PORT, "/server/port", 8009); /* 服务器端口 */

CONFIG_KV_SW(ADMIN_ACCOUNT, "/admin/account", "root"); /* 管理员账户 */
CONFIG_KV_SW(ADMIN_PASSWORD, "/admin/password", "123456"); /* 管理员密码 */
} // namespace config_key
