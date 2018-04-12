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
#include <map>
#include "../libevent/include/event.h"
#include "../libevent/include/evhttp.h"
#include "MultipartFormData.h"

class HttpField;

// http错误回调
#define HTTP_ERROR_CALLBACK std::function<std::string(const std::string& method, \
                                                      const std::string& host, \
                                                      unsigned short port, \
                                                      const std::map<std::string, std::string>& headers, \
                                                      const std::map<std::string, HttpField*>& body, \
                                                      unsigned int errorCode, \
                                                      const std::string& errorBuf, \
                                                      std::map<std::string, std::string>& responseHeaders)>

// http路由回调
#define HTTP_ROUTER_CALLBACK std::function<std::string(const std::string& method, \
                                                       const std::string& host, \
                                                       unsigned short port, \
                                                       const std::map<std::string, std::string>& headers, \
                                                       const std::map<std::string, HttpField*>& body, \
                                                       std::map<std::string, std::string>& responseHeaders)>

// http路由对象
class HttpRouter {
public:
    HttpRouter() : support_get(true), support_post(true) {}

public:
    bool support_get;                       // 是否支持get
    bool support_post;                      // 是否支持post
    HTTP_ROUTER_CALLBACK callback;          // 回调函数
};

// http字段对象
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
    //------------------- used when form data is file  -------------------
    std::string getFilename(void);
    void setFilename(const std::string& filename);
    std::string getFileContentType(void);
    void setFileContentType(const std::string& fileContentType);
    //--------------------------------------------------------------------

private:
    std::string mName;
    unsigned int mType;
    char* mContent;
    size_t mContentLength;
    std::string mFilename;
    std::string mFileContentType;
};

// http服务端
class HttpServer {
public:
    static HttpServer* getInstance(void);   // 获取单例
    static void destroyInstance(void);      // 删除单例
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
                            std::map<std::string, std::string>& responseHeaders);       // 处理错误
    std::string handleRouter(char major,
                             char minor,
                             const std::string& method,
                             const std::string& host,
                             unsigned short port,
                             const std::map<std::string, std::string>& headers,
                             const std::map<std::string, HttpField*>& body,
                             const std::string& uri,
                             std::map<std::string, std::string>& responseHeaders);      // 处理路由
    void setErrorCallback(HTTP_ERROR_CALLBACK errorCallback);                           // 设置错误回调
    void addRouter(const std::string& uri, HttpRouter* router);                         // 添加路由
    void addRouter(const std::string& uri, HTTP_ROUTER_CALLBACK callback);              // 添加路由(支持get和post)
    void addRouterGet(const std::string& uri, HTTP_ROUTER_CALLBACK callback);           // 添加路由(只支持get)
    void addRouterPost(const std::string& uri, HTTP_ROUTER_CALLBACK callback);          // 添加路由(只支持post)
    void run(const std::string& ip, unsigned int port, bool printReceive = false, bool printError = true);    // 运行

private:
    std::string getErrorResponse(unsigned int errorCode, const std::string& errorBuf);  // 获取错误响应
    void printReceive(char major,
                      char minor,
                      const std::string& method,
                      const std::string& host,
                      unsigned short port,
                      const std::map<std::string, std::string>& headers,
                      const std::map<std::string, HttpField*>& body,
                      const std::string& uri);                                          // 打印接收到的信息

private:
    bool mIsRunning;                                // 是否运行中
    bool mIsPrintReceive;                           // 是否打印接收信息
    bool mIsPrintError;                             // 是否打印错误信息
    HTTP_ERROR_CALLBACK mErrorCallback;             // 错误回调
    std::map<std::string, HttpRouter*> mRouterMap;  // 路由映射表
};

#endif  //_HTTP_SERVER_H_
