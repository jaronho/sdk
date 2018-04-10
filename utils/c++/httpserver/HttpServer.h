/**********************************************************************
* Author:	jaron.ho
* Date:		2018-04-10
* Brief:	http server
**********************************************************************/
#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <string>
#include <map>
#include <functional>
#include "../libevent/include/event.h"
#include "../libevent/include/evhttp.h"

// http路由
#define HTTP_ROUTER std::function<const char*(void* data)>

// http服务端
class HttpServer {
public:
    static HttpServer* getInstance(void);	// 获取单例
    static void destroyInstance(void);		// 删除单例
	void addHeader(const std::string& name, const std::string& value);	// 添加头部
    void addRouter(const std::string& uri, HTTP_ROUTER router);     // 添加路由
	const char* handleRouter(const char* uri, void* data);	// 处理路由
    void run(const std::string& ip, unsigned int port, bool pcri = true);     // 运行

private:
	bool mIsRunning;	// 是否运行中
	std::map<std::string, std::string> mHeaderMap;		// 头部映射表
    std::map<std::string, HTTP_ROUTER> mRouterMap;      // 路由映射表
};

#endif //_HTTP_SERVER_H_
