/**********************************************************************
* Author:	jaron.ho
* Date:		2014-06-04
* Brief:	http 客户端
**********************************************************************/
#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include <string>
#include <algorithm>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../libcurl/CURLEx.h"

// http回调
#define HTTP_CALLBACK std::function<void(bool success, int curlcode, int responsecode, \
                                         const std::string& errorbuffer, \
                                         const std::string& responseheader, \
                                         const std::string& responsebody)>

// http对象
class HttpObject : public CurlRequest {
public:
    HttpObject() : requesttype("post"), syncresponse(true), success(false), curlcode(-1), responsecode(-1) {}

public:
    // 请求
    std::string tag;						// 标签,用以标识每个请求
    std::string requesttype;				// 请求类型(不区分大小写):"GET","POST","POST_FORM","PUT","DELETE"
    bool syncresponse;						// 同步响应
    HTTP_CALLBACK callback;                 // 回调函数

    // 响应
    bool success;							// 是否请求成功
    int curlcode;							// 请求返回的curl码
    int responsecode;						// http响应码,[200, 404]
    std::string errorbuffer;				// 错误描述,当responsecode!=200时,此值描述错误信息
    std::vector<char> responseheader;		// 响应头部,可以通过std::string str(responseheader->begin(), responseheader->end());返回字符串
    std::vector<char> responsebody;			// 响应数据,可以通过std::string str(responsebody->begin(), responsebody->end());返回字符串
};

// http客户端
class HttpClient {
public:
    static HttpClient* getInstance(void);	// 获取单例
    static void destroyInstance(void);		// 删除单例
    void send(HttpObject* obj);				// 发送http请求
    void receive(void);						// 每帧循环接收(用于异步响应)
    void get(const std::string& url, HTTP_CALLBACK callback = 0);   // http get 请求(同步响应)
    void post(const std::string& url, const char* data, HTTP_CALLBACK callback = 0);    // http post 请求(同步响应)
    void postForm(const std::string& url,
                  const std::map<std::string, std::string>* contents = NULL,
                  const std::map<std::string, std::string>* files = NULL,
                  HTTP_CALLBACK callback = 0);    // http post 请求(同步响应)
};

#endif //_HTTP_CLIENT_H_

/*
************************************************** sample_01

int main() {
    auto callback = [](bool success, int curlcode, int responsecode, const std::string& errorbuffer, const std::string& responseheader, const std::string& responsebody)->void {
        printf("==================================================\n");
        printf("success = %s\n", success ? "true" : "false");
        printf("curlcode = %d\n", curlcode);
        printf("responsecode = %d\n", responsecode);
        printf("errorbuffer = %s\n", errorbuffer.c_str());
        printf("responseheader = \n%s\n", responseheader.c_str());
        printf("responsebody = \n%s\n", responsebody.c_str());
    };
    HttpClient::getInstance()->get("http://www.baidu.com", callback);
    while (true) {}
    return 0;
}
*/
