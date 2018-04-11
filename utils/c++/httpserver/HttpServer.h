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

// http路由回调
#define HTTP_ROUTER_CALLBACK std::function<std::string(const std::map<std::string, std::string>& headers, \
													   const std::map<std::string, HttpField*>& body)>

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
	void addHeader(const std::string& name, const std::string& value);					// 添加头部
    void addRouter(const std::string& uri, HttpRouter* router);							// 添加路由
	void addRouter(const std::string& uri, HTTP_ROUTER_CALLBACK callback);				// 添加路由(支持get和post)
	void addRouterGet(const std::string& uri, HTTP_ROUTER_CALLBACK callback);			// 添加路由(只支持get)
	void addRouterPost(const std::string& uri, HTTP_ROUTER_CALLBACK callback);			// 添加路由(只支持post)
	std::string handleRouter(const std::string& method, 
							 const std::string& uri,
							 const std::map<std::string, std::string>& headers,
							 const std::map<std::string, HttpField*>& body);			// 处理路由
    void run(const std::string& ip, unsigned int port, unsigned int printLevel = 0);	// 运行

private:
	bool mIsRunning;	// 是否运行中
	std::map<std::string, std::string> mHeaderMap;		// 头部映射表
    std::map<std::string, HttpRouter*> mRouterMap;      // 路由映射表
};

#endif //_HTTP_SERVER_H_
