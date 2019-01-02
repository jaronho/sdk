#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <string>
#include <vector>
#include <map>
#include <mutex>

#define GLOBAL_MODULE_COMMON            /* 启用 Common 模块 */
#define GLOBAL_MODULE_INI               /* 启用 Ini 模块 */
#define GLOBAL_MODULE_XML               /* 启用 Xml 模块 */
#define GLOBAL_MODULE_LOG               /* 启用 Log 模块 */
#define GLOBAL_MODULE_HTTP_CLIENT       /* 启用 Htttp Client 模块 */
#define GLOBAL_MODULE_HTTP_SERVER       /* 启用 Http Server 模块 */

/*********************************************************************
***************************** Device 接口 ****************************
**********************************************************************/
#ifdef __linux
#include <ifaddrs.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#endif

typedef struct network_dev_st {
    char name[IFNAMSIZ];
    char mac[32];
    char ipv4[INET_ADDRSTRLEN];
    char ipv6[INET6_ADDRSTRLEN];
} network_dev_st;

/* 获取网络设备 */
extern std::vector<network_dev_st> devGetNetwork(void);

#ifdef GLOBAL_MODULE_COMMON
/*********************************************************************
***************************** Common 接口 ****************************
**********************************************************************/
#include "common/Common.h"
/* 初始应用程序目录 */
extern void initAppDirectory(const std::string& dir);

/* 获取应用程序目录 */
extern std::string getAppDirectory(void);
#endif

#ifdef GLOBAL_MODULE_INI
/*********************************************************************
****************************** INI 接口 ******************************
**********************************************************************/
#include "inifile/IniFile.h"

/* 获取ini整型 */
extern int iniGetInt(const std::string& key);

/* 设置ini整型 */
extern void iniSetInt(const std::string& key, int value);

/* 获取ini浮点型 */
extern float iniGetFloat(const std::string& key);

/* 设置ini浮点型 */
extern void iniSetFloat(const std::string& key, float value);

/* 获取ini字符串 */
extern std::string iniGetString(const std::string& key);

/* 设置ini字符串 */
extern void iniSetString(const std::string& key, const std::string& value);

/* 保存ini */
extern void iniSave(void);
#endif

#ifdef GLOBAL_MODULE_XML
/*********************************************************************
****************************** XML 接口 ******************************
**********************************************************************/
#include "shareprefs/SharePrefs.h"

/* 获取xml整型 */
extern int xmlGetInt(const std::string& key);

/* 设置xml整型 */
extern void xmlSetInt(const std::string& key, int value);

/* 获取xml浮点型 */
extern float xmlGetFloat(const std::string& key);

/* 设置xml浮点型 */
extern void xmlSetFloat(const std::string& key, float value);

/* 获取xml字符串 */
extern std::string xmlGetString(const std::string& key);

/* 设置xml字符串 */
extern void xmlSetString(const std::string& key, const std::string& value);

/* 保存xml */
extern void xmlSave(void);
#endif

#ifdef GLOBAL_MODULE_LOG
/*********************************************************************
****************************** 日志接口 ******************************
**********************************************************************/
#include "logfile/logfilewrapper.h"

/* 日志记录 */
extern void logRecord(const std::string& str);

/* 错误记录 */
extern void errRecord(const std::string& str);
#endif

#ifdef GLOBAL_MODULE_HTTP_CLIENT
/*********************************************************************
*************************** HTTP CLIENT 接口 *************************
**********************************************************************/
#include "httpclient/HttpClient.h"

/* http请求GET */
extern void httpGet(const std::string& url, HTTP_REQUEST_CALLBACK callback);

/* http请求POST */
extern void httpPost(const std::string& url, const std::string& data, HTTP_REQUEST_CALLBACK callback);

/* http请求POST表单 */
extern void httpPostForm(const std::string& url, const std::map<std::string, std::string>* contents, const std::map<std::string, std::string>* files, HTTP_REQUEST_CALLBACK callback);
#endif

#ifdef GLOBAL_MODULE_HTTP_SERVER
/*********************************************************************
*************************** HTTP SERVER 接口 *************************
**********************************************************************/
#include "httpserver/HttpServer.h"

/* 设置server过滤回调 */
extern void serverFilterCB(HTTP_FILTER_CALLBACK callback);

/* 设置server错误回调 */
extern void serverErrorCB(HTTP_ERROR_CALLBACK callback);

/* 设置server路由,支持GET/POST */
extern void serverRouter(const std::string& uri, HTTP_ROUTER_CALLBACK callback);

/* 设置server路由,只支持GET */
extern void serverRouterGet(const std::string& uri, HTTP_ROUTER_CALLBACK callback);

/* 设置server路由,只支持POST */
extern void serverRouterPost(const std::string& uri, HTTP_ROUTER_CALLBACK callback);

/* server运行 */
extern void serverRun(unsigned int port, bool printReceive, bool printError, bool printFilter);

/* 获取字段字符串值 */
extern std::string serverFieldString(const std::map<std::string, HttpField*>& body, const std::string& name, const std::string& defaultValue);

/* 获取字段整型值 */
extern int serverFieldInt(const std::map<std::string, HttpField*>& body, const std::string& name, int defaultValue);
#endif

#endif
