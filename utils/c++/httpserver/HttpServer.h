/**********************************************************************
* Author:	jaron.ho
* Date:		2018-04-10
* Brief:	http server
**********************************************************************/
#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>
#if defined(WIN32) || defined(_WIN32) ||  defined(WIN64) || defined(_WIN64)
#include "../libevent/platform-win32/include/event.h"
#include "../libevent/platform-win32/include/evhttp.h"
#else
#include "../libevent/platform-linux/include/event.h"
#include "../libevent/platform-linux/include/evhttp.h"
#endif

class HttpField;

/* http过滤回调,返回值:0.通过,非0.被过滤 */
#define HTTP_FILTER_CALLBACK std::function<int(char major, \
                                               char minor, \
                                               const std::string& method, \
                                               const std::string& host, \
                                               unsigned short port, \
                                               const std::string& uri)>

/* http错误回调,返回值:无 */
#define HTTP_ERROR_CALLBACK std::function<void(const std::string& method, \
                                               const std::string& host, \
                                               unsigned short port, \
                                               const std::map<std::string, std::string>& headers, \
                                               const std::map<std::string, HttpField*>& body, \
                                               const std::string& uri, \
                                               unsigned int errorCode, \
                                               const std::string& errorBuf, \
                                               std::map<std::string, std::string>& responseHeaders)>

/* http路由回调,返回值:响应字符串,格式自定义,一般是json格式 */
#define HTTP_ROUTER_CALLBACK std::function<std::string(const std::string& method, \
                                                       const std::string& host, \
                                                       unsigned short port, \
                                                       const std::map<std::string, std::string>& headers, \
                                                       const std::map<std::string, HttpField*>& body, \
                                                       const std::string& uri, \
                                                       std::map<std::string, std::string>& responseHeaders)>

/* http路由对象 */
class HttpRouter {
public:
    HttpRouter() : support_get(true), support_post(true) {}

public:
    bool support_get;                       /* 是否支持get */
    bool support_post;                      /* 是否支持post */
    HTTP_ROUTER_CALLBACK callback;          /* 回调函数 */
};

/* http字段对象 */
class HttpField {
public:
    static const unsigned int TYPE_TEXT = 1;
    static const unsigned int TYPE_FILE = 2;

public:
    HttpField(void);
    ~HttpField(void);

    std::string getName(void);
    void setName(const std::string& name);
    unsigned int getType(void);
    void setType(unsigned int type);
    const char* getContent(void);
    void setContent(const char* content, size_t length);
    size_t getContentLength(void);
    /*------------------- used when form data is file  -------------------*/
    std::string getFilename(void);
    void setFilename(const std::string& filename);
    std::string getFileContentType(void);
    void setFileContentType(const std::string& fileContentType);
    /*--------------------------------------------------------------------*/

private:
    std::string mName;
    unsigned int mType;
    char* mContent;
    size_t mContentLength;
    std::string mFilename;
    std::string mFileContentType;
};

/* http服务端 */
class HttpServer {
public:
    static HttpServer* getInstance(void);               /* 获取单例 */
    static std::string nowdate(void);                   /* 当前日期 */
    static std::vector<std::string> localhosts(void);   /* 获取本机IP地址 */

public:
    int handleFilter(char major,
                     char minor,
                     const char* method,
                     const char* host,
                     unsigned short port,
                     const char* uri);                                                  /* 处理过滤 */
    std::string handleError(char major,
                            char minor,
                            const std::string& method,
                            const std::string& host,
                            unsigned short port,
                            const std::map<std::string, std::string>& headers,
                            const std::map<std::string, HttpField*>& body,
                            const std::string& uri,
                            unsigned int errorCode,
                            const std::string& errorBuf,
                            std::map<std::string, std::string>& responseHeaders);       /* 处理错误 */
    std::string handleRouter(char major,
                             char minor,
                             const std::string& method,
                             const std::string& host,
                             unsigned short port,
                             const std::map<std::string, std::string>& headers,
                             const std::map<std::string, HttpField*>& body,
                             const std::string& uri,
                             std::map<std::string, std::string>& responseHeaders);      /* 处理路由 */
    void setFilterCallback(HTTP_FILTER_CALLBACK filterCallback);                        /* 设置过滤回调 */
    void setErrorCallback(HTTP_ERROR_CALLBACK errorCallback);                           /* 设置错误回调 */
    void addRouter(const std::string& uri, HttpRouter* router);                         /* 添加路由 */
    void addRouter(const std::string& uri, HTTP_ROUTER_CALLBACK callback);              /* 添加路由(支持get和post) */
    void addRouterGet(const std::string& uri, HTTP_ROUTER_CALLBACK callback);           /* 添加路由(只支持get) */
    void addRouterPost(const std::string& uri, HTTP_ROUTER_CALLBACK callback);          /* 添加路由(只支持post) */
    void run(const std::string& ip, unsigned int port, bool printReceive = false, bool printError = true, bool printFilter = true);    /* 运行 */

private:
    std::string getErrorResponse(unsigned int errorCode, const std::string& errorBuf);  /* 获取错误响应 */
    void printReceive(char major,
                      char minor,
                      const std::string& method,
                      const std::string& host,
                      unsigned short port,
                      const std::map<std::string, std::string>& headers,
                      const std::map<std::string, HttpField*>& body,
                      const std::string& uri);                                          /* 打印接收到的信息 */

private:
    bool mIsRunning;                                /* 是否运行中 */
    bool mIsPrintReceive;                           /* 是否打印接收信息 */
    bool mIsPrintError;                             /* 是否打印错误信息 */
    bool mIsPrintFilter;                            /* 是否打印过滤信息 */
    HTTP_FILTER_CALLBACK mFilterCallback;           /* 过滤回调 */
    HTTP_ERROR_CALLBACK mErrorCallback;             /* 错误回调 */
    std::map<std::string, HttpRouter*> mRouterMap;  /* 路由映射表 */
};

#endif  /* _HTTP_SERVER_H_ */

/*
************************************************** sample_01

auto handleFilter = [](char major,
                       char minor,
                       const std::string& method,
                       const std::string& host,
                       unsigned short port,
                       const std::string& uri)->int {
    if (0 != strcmp("POST", method)) {
        return 1;
    }
    if (0 != strcmp("/post", uri)) {
        return 2;
    }
    return 0;
};

auto handleError = [](const std::string& method,
                      const std::string& host,
                      unsigned short port,
                      const std::map<std::string, std::string>& headers,
                      const std::map<std::string, HttpField*>& body,
                      const std::string& uri,
                      unsigned int errorCode,
                      const std::string& errorBuf,
                      std::map<std::string, std::string>& responseHeaders)->void {
    printf("Error: code => %d, msg => %s\n", errorCode, errorBuf.c_str());
};

auto handlePost = [](const std::string& method,
                     const std::string& host,
                     unsigned short port,
                     const std::map<std::string, std::string>& headers,
                     const std::map<std::string, HttpField*>& body,
                     const std::string& uri,
                     std::map<std::string, std::string>& responseHeaders)->std::string {
    std::string response = "{";
    std::map<std::string, HttpField*>::const_iterator iter = body.begin();
    for (; body.end() != iter; ++iter) {
        if (body.begin() != iter) {
            response += ",";
        }
        response += "\"" + iter->first + "\":";
        if (HttpField::TYPE_TEXT == iter->second->getType()) {
            response += iter->second->getContent();
        }
    }
    response += "}";
    return response;
};

int main() {
    HttpServer::getInstance()->setFilterCallback(handleFilter);
    HttpServer::getInstance()->setErrorCallback(handleError);
    HttpServer::getInstance()->addRouterPost("/post", handlePost);
    HttpServer::getInstance()->run("127.0.0.1", 5001, true, true);
    system("pause");
    return 0;
}
*/
