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
#include <sys/statfs.h>
#endif

typedef struct network_dev_st {
    char name[IFNAMSIZ];            /* 网络设备名 */
    char mac[32];                   /* MAC地址 */
    char ipv4[INET_ADDRSTRLEN];     /* IPv4地址 */
    char ipv6[INET6_ADDRSTRLEN];    /* IPv6地址 */
} network_dev_st;

typedef struct memory_dev_st {
    unsigned long total;            /* 总内存大小 */
    unsigned long free;             /* 空闲内存大小 */
    unsigned long available;        /* 可用的内存大小 */
} memory_dev_st;

typedef struct disk_dev_st {
    unsigned long long total;       /* 磁盘总大小(单位:字节B) */
    unsigned long long free;        /* 空闲磁盘大小(单位:字节B) */
    unsigned long long available;   /* 非超级用户可用的空闲磁盘大小(单位:字节B) */
} disk_dev_st;

typedef struct cpu_dev_st { /* 1 jiffies = 0.01秒 */
    unsigned long user;             /* 从系统启动开始累计到当前时刻,处于用户态的运行时间,不包含nice值为负的进程(单位:jiffies) */
    unsigned long nice;             /* 从系统启动开始累计到当前时刻,nice值为负的进程所占用的CPU时间(单位:jiffies) */
    unsigned long system;           /* 从系统启动开始累计到当前时刻,处于核心态的运行时间(单位:jiffies) */
    unsigned long idle;             /* 从系统启动开始累计到当前时刻,除IO等待时间以外的其它等待时间(单位:jiffies) */
    unsigned long iowait;           /* 从系统启动开始累计到当前时刻,IO等待时间(单位:jiffies) */
    unsigned long irq;              /* 从系统启动开始累计到当前时刻,硬中断时间(单位:jiffies) */
    unsigned long softirq;          /* 从系统启动开始累计到当前时刻,软中断时间(单位:jiffies) */
} cpu_dev_st;

/* 获取网络设备 */
extern std::vector<network_dev_st> devGetNetwork(void);

/* 获取内存情况 */
extern memory_dev_st devGetMemory(void);

/* 获取内存使用率 */
extern double devGetMemoryOccupy(void);

/* 获取磁盘情况 */
extern disk_dev_st devGetDisk(const char* path);

/* 获取磁盘使用率 */
extern double devGetDiskOccupy(void);

/* 获取CPU情况 */
extern cpu_dev_st devGetCPU(void);

/* 获取CPU使用率 */
extern double devGetCpuOccupy(void);

#ifdef GLOBAL_MODULE_COMMON
/*********************************************************************
***************************** Common 接口 ****************************
**********************************************************************/
#include "common/Common.h"
/* 初始应用程序目录 */
extern void initAppDirectory(const std::string& dir);

/* 获取应用程序目录 */
extern std::string getAppDirectory(void);

/* 获取应用程序名称 */
extern std::string getAppName(void);
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

/* http异步监听(每帧循环调用) */
extern void httpAsyncListen(void);

/* http请求GET */
extern void httpGet(const std::string& sslCaFilename, const std::string& url, const std::map<std::string, std::string>* headers, HTTP_REQUEST_CALLBACK callback, void* param, int connecttimeout, int timeout, bool async);

/* http请求POST */
extern void httpPost(const std::string& sslCaFilename, const std::string& url, const std::map<std::string, std::string>* headers, const unsigned char* data, unsigned int dataLength, HTTP_REQUEST_CALLBACK callback, void* param, int connecttimeout, int timeout, bool async);

/* http请求POST表单 */
extern void httpPostForm(const std::string& sslCaFilename, const std::string& url, const std::map<std::string, std::string>* headers, const std::map<std::string, std::string>* contents, const std::map<std::string, std::string>* filenames, HTTP_REQUEST_CALLBACK callback, void* param, int connecttimeout, int timeout, bool async);
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
