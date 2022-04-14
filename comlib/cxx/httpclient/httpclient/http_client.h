#pragma once

#include <list>
#include <mutex>
#include <string>

#include "curlex/curlex.h"
#include "threading/thread_proxy.hpp"

namespace http
{
/**
 * @brief 响应回调
 * @param resp 响应数据
 */
using ResponseCallback = std::function<void(curlex::Response& resp)>;

/**
 * @brief HTTP客户端模块
 */
class HttpClient
{
public:
    /**
     * @brief 启动模块
     * @param threadCount 用于执行网络事件的线程个数
     */
    static void start(size_t threadCount = 4);

    /**
     * @brief 停止模块
     */
    static void stop();

    /**
     * @brief 运行单次(用于监听响应回调, 在主逻辑线程中循环调用, 一般是在主线程)
     */
    static void runOnce();

    /**
     * @brief 创建简单请求对象
     * @param url 资源地址
     * @return 请求对象
     */
    static curlex::SimpleRequestPtr makeSimpleRequest(const std::string& url);

    /**
     * @brief 创建SSL(双向验证)请求对象
     * @param certFile 证书文件, 例如: client.crt
     * @param privateKeyFile 私钥文件, 例如: client.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: qq123456
     * @param url 资源地址
     * @return 请求对象
     */
    static curlex::Ssl2WayRequestPtr makeSsl2WayRequest(const std::string& certFile, const std::string& privateKeyFile,
                                                        const std::string& privateKeyFilePwd, const std::string& url);

    /**
     * @brief 创建带用户名密码请求对象
     * @param username 用户名
     * @param password 密码
     * @param url 资源地址
     * @return 请求对象
     */
    static curlex::UserpwdRequestPtr makeUserpwdRequest(const std::string& username, const std::string& password, const std::string& url);

    /**
     * @brief 创建字节流数据对象
     * @param bytes 字节流
     * @param count 字节数
     * @return 数据对象
     */
    static curlex::RawRequestDataPtr makeRawData(const char* bytes, size_t count);

    /**
     * @brief 创建表单数据对象
     * @param fieldMap 表单数据
     * @return 数据对象
     */
    static curlex::FormRequestDataPtr makeFormData(const std::map<std::string, std::string>& fieldMap);

    /**
     * @brief 创建多部份表单数据对象
     * @return 数据对象
     */
    static curlex::MultipartFormRequestDataPtr makeMultipartFormData();

    /**
     * @brief 简单DELETE请求
     * @param req 请求对象
     * @param funcSet 函数集(在http线程调用)
     * @param respCb 响应回调(在主线程调用)
     */
    static void easyDelete(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb);

    /**
     * @brief 简单GET请求
     * @param req 请求对象
     * @param funcSet 函数集(在http线程调用)
     * @param respCb 响应回调(在主线程调用)
     */
    static void easyGet(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb);

    /**
     * @brief 简单PUT请求
     * @param req 请求对象
     * @param funcSet 函数集(在http线程调用)
     * @param respCb 响应回调(在主线程调用)
     */
    static void easyPut(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb);

    /**
     * @brief 简单POST请求
     * @param req 请求对象
     * @param funcSet 函数集(在http线程调用)
     * @param respCb 响应回调(在主线程调用)
     */
    static void easyPost(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb);

    /**
     * @brief 简单下载请求
     * @param req 请求对象
     * @param filename 保存到本地的文件名
     * @param recover 是否强制覆盖
     * @param funcSet 函数集(在http线程调用)
     * @param respCb 响应回调(在主线程调用)
     */
    static void easyDownload(const curlex::RequestPtr& req, const std::string& filename, bool recover, const curlex::FuncSet& funcSet,
                             const ResponseCallback& respCb);

private:
    /**
     * @brief 插入响应列表
     * @param resp 响应数据
     * @param respCb 响应回调
     */
    static void insertRespList(const curlex::Response& resp, const ResponseCallback& respCb);

    /**
     * @brief 响应参数, 用于跨线程传输
     */
    struct RespParam
    {
        curlex::Response resp;
        ResponseCallback respCb = nullptr;
    };

private:
    static threading::ExecutorPtr s_workers; /* 网络线程池 */
    static std::mutex s_respMutex; /* 互斥量 */
    static std::list<std::shared_ptr<RespParam>> s_respList; /* 响应列表 */
};
} // namespace http
