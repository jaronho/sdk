#include "Global.h"

#ifdef GLOBAL_MODULE_COMMON
/*********************************************************************
***************************** Common 接口 ****************************
**********************************************************************/
std::string utf8ToString(const std::string& str) {
    int nwLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* pwBuf = new wchar_t[nwLen + 1];    /* 一定要加1，不然会出现尾巴 */
    memset(pwBuf, 0, nwLen * 2 + 2);
    ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = ::WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    char* pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);
    ::WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
    std::string retStr = pBuf;
    delete[]pBuf;
    delete[]pwBuf;
    pBuf = NULL;
    pwBuf = NULL;
    return retStr;
}

std::string stringToUTF8(const std::string& str) {
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    wchar_t* pwBuf = new wchar_t[nwLen + 1];    /* 一定要加1，不然会出现尾巴 */
    ZeroMemory(pwBuf, nwLen * 2 + 2);
    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    char* pBuf = new char[nLen + 1];
    ZeroMemory(pBuf, nLen + 1);
    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
    std::string retStr(pBuf);
    delete[]pwBuf;
    delete[]pBuf;
    pwBuf = NULL;
    pBuf = NULL;
    return retStr;
}

std::string utf8string(const std::string& str) {
    if (str.empty()) {
        return str;
    }
    if (Common::isStringUTF8(str.c_str())) {
        return str;
    }
    return stringToUTF8(str);
}
#endif

#ifdef GLOBAL_MODULE_INI
/*********************************************************************
****************************** INI 接口 ******************************
**********************************************************************/
#define INI_FILENAME        "config.ini"
#define INI_SECTION         "CONFIG"
IniFile s_ini;
bool s_iniOpened = false;

bool openIni(void) {
    if (s_iniOpened) {
        return true;
    }
    if (0 == s_ini.open(INI_FILENAME)) {
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
bool s_xmlOpened = false;

bool openXml(void) {
    if (s_xmlOpened) {
        return true;
    }
    if (SharePrefs::open(XML_FILENAME)) {
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
#define LOG_FILE_SIZE       (1024 * 1024 * 4)
logfilewrapper_st* s_logWrapper = NULL;
std::mutex s_logWrapperMutex;

void logRecord(const std::string& str) {
    printf("%s\n", str.c_str());
    s_logWrapperMutex.lock();
    if (NULL == s_logWrapper) {
        s_logWrapper = logfilewrapper_init(LOG_FILENAME, LOG_EXTNAME, LOG_FILE_SIZE, false);
    }
    logfilewrapper_record(s_logWrapper, NULL, 1, str.c_str());
    s_logWrapperMutex.unlock();
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
    HttpClient::getInstance()->post(url, data.empty() ? NULL : data.c_str(), callback);
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

std::string serverField(const std::map<std::string, HttpField*>& body, const std::string& name, const std::string& defaultValue) {
    std::map<std::string, HttpField*>::const_iterator iter;
    if (body.end() != (iter = body.find(name))) {
        return iter->second->getContent();
    }
    return defaultValue;
}
#endif
