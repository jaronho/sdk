#include "Global.h"

static std::string s_app_directory;

/*********************************************************************
***************************** Device 接口 ****************************
**********************************************************************/
std::vector<network_dev_st> devGetNetwork(void) {
    std::vector<network_dev_st> networkDevList;
#ifdef __linux
    struct ifaddrs* ifa_head = NULL;
    struct ifaddrs* ifa_curr = NULL;
    getifaddrs(&ifa_head);
    for (ifa_curr = ifa_head; ifa_curr; ifa_curr = ifa_curr->ifa_next) {
        if (!ifa_curr->ifa_addr) {
            continue;
        }
        int index = -1;
        for (size_t i = 0; i < networkDevList.size(); ++i) {
            if (0 == strcmp(ifa_curr->ifa_name, networkDevList[i].name)) {
                index = i;
                break;
            }
        }
        if (index < 0) {
            index = 0;
            network_dev_st networkDev;
            strcpy(networkDev.name, ifa_curr->ifa_name);
            /* MAC Address */
            struct ifreq req;
            req.ifr_addr.sa_family = AF_INET;
            strcpy((char*)req.ifr_name, ifa_curr->ifa_name);
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd) {
                ioctl(fd, SIOCGIFHWADDR, &req);
                close(fd);
                unsigned char* mac = (unsigned char*)req.ifr_hwaddr.sa_data;
                snprintf(networkDev.mac, sizeof(networkDev.mac), "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }
            networkDevList.push_back(networkDev);
        }
        if (AF_INET == ifa_curr->ifa_addr->sa_family) { /* IPv4 Address */
            void* tmp_addr = &((struct sockaddr_in*)ifa_curr->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmp_addr, networkDevList[index].ipv4, INET_ADDRSTRLEN);
        } else if (AF_INET6 == ifa_curr->ifa_addr->sa_family) { /* IPv6 Address */
            void* tmp_addr = &((struct sockaddr_in6*)ifa_curr->ifa_addr)->sin6_addr;
            inet_ntop(AF_INET6, tmp_addr, networkDevList[index].ipv6, INET6_ADDRSTRLEN);
        }
    }
    /* remove trailing comma */
    if (ifa_head) {
        freeifaddrs(ifa_head);
    }
#endif
    return networkDevList;
}

#ifdef GLOBAL_MODULE_COMMON
/*********************************************************************
***************************** Common 接口 ****************************
**********************************************************************/
void initAppDirectory(const std::string& dir) {
    if (s_app_directory.empty() && dir.size() > 0) {
        std::string exeFullPath = Common::replaceString(dir, "\\", "/");
        s_app_directory = exeFullPath.substr(0, exeFullPath.find_last_of('/')) + "/";
    }
}

std::string getAppDirectory(void) {
    return s_app_directory;
}
#endif

#ifdef GLOBAL_MODULE_INI
/*********************************************************************
****************************** INI 接口 ******************************
**********************************************************************/
#define INI_FILENAME        "config.ini"
#define INI_SECTION         "CONFIG"
static IniFile s_ini;
static bool s_iniOpened = false;

bool openIni(void) {
    if (s_iniOpened) {
        return true;
    }
    if (0 == s_ini.open(s_app_directory + INI_FILENAME)) {
        s_iniOpened = true;
        return true;
    }
    return false;
}

int iniGetInt(const std::string& key) {
    if (!openIni()) {
        return 0;
    }
    return s_ini.getInt(INI_SECTION, key);
}

void iniSetInt(const std::string& key, int value) {
    if (!openIni()) {
        return;
    }
    s_ini.setInt(INI_SECTION, key, value);
}

float iniGetFloat(const std::string& key) {
    if (!openIni()) {
        return 0;
    }
    return s_ini.getFloat(INI_SECTION, key);
}

void iniSetFloat(const std::string& key, float value) {
    if (!openIni()) {
        return;
    }
    s_ini.setFloat(INI_SECTION, key, value);
}

std::string iniGetString(const std::string& key) {
    if (!openIni()) {
        return "";
    }
    return s_ini.getString(INI_SECTION, key);
}

void iniSetString(const std::string& key, const std::string& value) {
    if (!openIni()) {
        return;
    }
    s_ini.setString(INI_SECTION, key, value);
}

void iniSave(void) {
    if (s_iniOpened) {
        s_ini.save();
    }
}
#endif

#ifdef GLOBAL_MODULE_XML
/*********************************************************************
****************************** XML 接口 ******************************
**********************************************************************/
#define XML_FILENAME        "config.xml"
static bool s_xmlOpened = false;

bool openXml(void) {
    if (s_xmlOpened) {
        return true;
    }
    if (SharePrefs::open(s_app_directory + XML_FILENAME)) {
        s_xmlOpened = true;
        return true;
    }
    return false;
}

int xmlGetInt(const std::string& key) {
    if (!openXml()) {
        return 0;
    }
    return SharePrefs::getInt(key);
}

void xmlSetInt(const std::string& key, int value) {
    if (!openXml()) {
        return;
    }
    SharePrefs::setInt(key, value);
}

float xmlGetFloat(const std::string& key) {
    if (!openXml()) {
        return 0;
    }
    return SharePrefs::getFloat(key);
}

void xmlSetFloat(const std::string& key, float value) {
    if (!openXml()) {
        return;
    }
    SharePrefs::setFloat(key, value);
}

std::string xmlGetString(const std::string& key) {
    if (!openXml()) {
        return "";
    }
    return SharePrefs::getString(key);
}

void xmlSetString(const std::string& key, const std::string& value) {
    if (!openXml()) {
        return;
    }
    SharePrefs::setString(key, value);
}

void xmlSave(void) {
    if (s_xmlOpened) {
        SharePrefs::save();
    }
}
#endif

#ifdef GLOBAL_MODULE_LOG
/*********************************************************************
****************************** 日志接口 ******************************
**********************************************************************/
#define LOG_FILENAME        "client"
#define LOG_EXTNAME         ".log"
#define LOG_FILE_SIZE       (1024 * 1024 * 10)
static logfilewrapper_st* s_logWrapper = nullptr;
static std::mutex s_logWrapperMutex;

void logRecord(const std::string& str) {
    printf("%s\n", str.c_str());
    s_logWrapperMutex.lock();
    if (!s_logWrapper) {
        s_logWrapper = logfilewrapper_init((s_app_directory + LOG_FILENAME).c_str(), LOG_EXTNAME, LOG_FILE_SIZE, false);
    }
    logfilewrapper_record(s_logWrapper, nullptr, 1, str.c_str());
    s_logWrapperMutex.unlock();
}

#define ERR_FILENAME        "client"
#define ERR_EXTNAME         ".err"
#define ERR_FILE_SIZE       (1024 * 1024 * 10)
static logfilewrapper_st* s_errWrapper = nullptr;
static std::mutex s_errWrapperMutex;

void errRecord(const std::string& str) {
    printf("%s\n", str.c_str());
    s_errWrapperMutex.lock();
    if (!s_errWrapper) {
        s_errWrapper = logfilewrapper_init((s_app_directory + ERR_FILENAME).c_str(), ERR_EXTNAME, ERR_FILE_SIZE, false);
    }
    logfilewrapper_record(s_errWrapper, nullptr, 1, str.c_str());
    s_errWrapperMutex.unlock();
}
#endif

#ifdef GLOBAL_MODULE_HTTP_CLIENT
/*********************************************************************
*************************** HTTP CLIENT 接口 *************************
**********************************************************************/
void httpGet(const std::string& url, HTTP_REQUEST_CALLBACK callback) {
    HttpClient::getInstance()->get(url, callback);
}

void httpPost(const std::string& url, const std::string& data, HTTP_REQUEST_CALLBACK callback) {
    HttpClient::getInstance()->post(url, data.empty() ? nullptr : data.c_str(), callback);
}

void httpPostForm(const std::string& url, const std::map<std::string, std::string>* contents, const std::map<std::string, std::string>* files, HTTP_REQUEST_CALLBACK callback) {
    HttpClient::getInstance()->postForm(url, contents, files, callback);
}
#endif

#ifdef GLOBAL_MODULE_HTTP_SERVER
/*********************************************************************
*************************** HTTP SERVER 接口 *************************
**********************************************************************/
void serverFilterCB(HTTP_FILTER_CALLBACK callback) {
    HttpServer::getInstance()->setFilterCallback(callback);
}

void serverErrorCB(HTTP_ERROR_CALLBACK callback) {
    HttpServer::getInstance()->setErrorCallback(callback);
}

void serverRouter(const std::string& uri, HTTP_ROUTER_CALLBACK callback) {
    HttpServer::getInstance()->addRouter(uri, callback);
}

void serverRouterGet(const std::string& uri, HTTP_ROUTER_CALLBACK callback) {
    HttpServer::getInstance()->addRouterGet(uri, callback);
}

void serverRouterPost(const std::string& uri, HTTP_ROUTER_CALLBACK callback) {
    HttpServer::getInstance()->addRouterPost(uri, callback);
}

void serverRun(unsigned int port, bool printReceive, bool printError, bool printFilter) {
    HttpServer::getInstance()->run("127.0.0.1", port, printReceive, printError, printFilter);
}

std::string serverFieldString(const std::map<std::string, HttpField*>& body, const std::string& name, const std::string& defaultValue) {
    std::map<std::string, HttpField*>::const_iterator iter;
    if (body.end() != (iter = body.find(name))) {
        return iter->second->getContent();
    }
    return defaultValue;
}

int serverFieldInt(const std::map<std::string, HttpField*>& body, const std::string& name, int defaultValue) {
    std::map<std::string, HttpField*>::const_iterator iter;
    if (body.end() != (iter = body.find(name))) {
        return atoi(iter->second->getContent());
    }
    return defaultValue;
}
#endif
