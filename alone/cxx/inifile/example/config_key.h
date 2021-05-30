#pragma once
#include <string>

#include "inifile/ini_helper.h"

const std::string CONFIG_FILENAME = "config.ini";
#define CONFIG_KV_S(keyVar, keyName, val) MAKE_INI_KV(CONFIG_FILENAME, keyVar, keyName, val, "", "")
#define CONFIG_KV_I(keyVar, keyName, val) MAKE_INI_KV(CONFIG_FILENAME, keyVar, keyName, std::to_string(val), "", "")

namespace config_key
{
CONFIG_KV_S(APPNAME, "/appname", "测试") /* 应用程序名称 */

CONFIG_KV_S(SERVER_IP, "/server/ip", "192.168.5.2") /* 服务器地址 */
CONFIG_KV_I(SERVER_PORT, "/server/port", 8009) /* 服务器端口 */

CONFIG_KV_S(ADMIN_ACCOUNT, "/admin/account", "root") /* 管理员账户 */
CONFIG_KV_S(ADMIN_PASSWORD, "/admin/password", "123456") /* 管理员密码 */
} // namespace config_key
