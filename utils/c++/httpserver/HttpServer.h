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

// http路由回调
#define HTTP_ROUTER_CALLBACK std::function<const char*(const std::map<std::string, std::string>&)>

// http路由对象
class HttpRouter {
public:
	HttpRouter() : support_get(true), support_post(true) {}

public:
	bool support_get;						// 是否支持get
	bool support_post;						// 是否支持post
	HTTP_ROUTER_CALLBACK callback;          // 回调函数
};

// http服务端
class HttpServer {
public:
    static HttpServer* getInstance(void);	// 获取单例
    static void destroyInstance(void);		// 删除单例
	void addHeader(const std::string& name, const std::string& value);			// 添加头部
    void addRouter(const std::string& uri, HttpRouter* router);					// 添加路由
	void addRouter(const std::string& uri, HTTP_ROUTER_CALLBACK callback);		// 添加路由(支持get和post)
	void addRouterGet(const std::string& uri, HTTP_ROUTER_CALLBACK callback);	// 添加路由(只支持get)
	void addRouterPost(const std::string& uri, HTTP_ROUTER_CALLBACK callback);	// 添加路由(只支持post)
	const char* handleRouter(const char* method, const char* uri, const std::map<std::string, std::string>& datas);	// 处理路由
    void run(const std::string& ip, unsigned int port, bool pcri = true);		// 运行

private:
	bool mIsRunning;	// 是否运行中
	std::map<std::string, std::string> mHeaderMap;		// 头部映射表
    std::map<std::string, HttpRouter*> mRouterMap;      // 路由映射表
};

#endif //_HTTP_SERVER_H_
