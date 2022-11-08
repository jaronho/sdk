#pragma once
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "multipart_form_data.h"
#include "request.h"
#include "response.h"

namespace nsocket
{
namespace http
{
/**
 * @brief 路由
 */
class Router
{
    friend class Server;

public:
    /**
     * @brief 获取允许的方法
     */
    std::vector<Method> getAllowMethods();

protected:
    /**
     * @brief 客户端请求方法不允许
     */
    virtual void onMethodNotAllowed(uint64_t cid, const REQUEST_PTR& req);

    /**
     * @brief 收到客户端请求头
     */
    virtual void onReqHead(uint64_t cid, const REQUEST_PTR& req);

    /**
     * @brief 收到客户端请求内容
     */
    virtual void onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen);

    /**
     * @brief 响应客户端
     */
    virtual void onResponse(uint64_t cid, const REQUEST_PTR& req, const SEND_RESPONSE_FUNC& sendRespFunc);

private:
    std::vector<Method> m_methods; /* 支持的方法, 例如: {Method::POST} */
};

/**
 * @brief 路由(对内容批次接收, 针对大数量的请求, 比如上传文件之类的)
 */
class Router_batch : public Router
{
public:
    std::function<void(uint64_t cid, const REQUEST_PTR& req)> methodNotAllowedCb;
    std::function<void(uint64_t cid, const REQUEST_PTR& req)> headCb;
    std::function<void(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)> contentCb;
    std::function<void(uint64_t cid, const REQUEST_PTR& req, const SEND_RESPONSE_FUNC& sendRespFunc)> respHandler;

protected:
    void onMethodNotAllowed(uint64_t cid, const REQUEST_PTR& req) override;
    void onReqHead(uint64_t cid, const REQUEST_PTR& req) override;
    void onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    void onResponse(uint64_t cid, const REQUEST_PTR& req, const SEND_RESPONSE_FUNC& sendRespFunc) override;
};

/**
 * @brief 路由(内容一次性接收, 针对小数据量的请求)
 */
class Router_simple : public Router
{
public:
    std::function<void(uint64_t cid, const REQUEST_PTR& req)> methodNotAllowedCb;
    std::function<void(uint64_t cid, const REQUEST_PTR& req, const std::string& data, const SEND_RESPONSE_FUNC& sendRespFunc)> respHandler;

protected:
    void onMethodNotAllowed(uint64_t cid, const REQUEST_PTR& req) override;
    void onReqHead(uint64_t cid, const REQUEST_PTR& req) override;
    void onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    void onResponse(uint64_t cid, const REQUEST_PTR& req, const SEND_RESPONSE_FUNC& sendRespFunc) override;

private:
    std::mutex m_mutex;
    std::unordered_map<uint64_t, std::shared_ptr<std::string>> m_contentMap;
};

/**
 * @brief 路由(application/x-www-form-urlencoded)
 */
class Router_x_www_form_urlencoded : public Router
{
public:
    std::function<void(uint64_t cid, const REQUEST_PTR& req)> methodNotAllowedCb;
    std::function<void(uint64_t cid, const REQUEST_PTR& req, const CaseInsensitiveMultimap& fields, const SEND_RESPONSE_FUNC& sendRespFunc)>
        respHandler;

protected:
    void onMethodNotAllowed(uint64_t cid, const REQUEST_PTR& req) override;
    void onReqHead(uint64_t cid, const REQUEST_PTR& req) override;
    void onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    void onResponse(uint64_t cid, const REQUEST_PTR& req, const SEND_RESPONSE_FUNC& sendRespFunc) override;

private:
    struct Wrapper
    {
        CaseInsensitiveMultimap fields;
        bool tmpKeyFlag = true;
        std::string tmpKey;
        std::string tmpValue;
    };

    std::mutex m_mutex;
    std::unordered_map<uint64_t, std::shared_ptr<Wrapper>> m_wrapperMap;
};

/**
 * @brief 路由(multipart/form-data)
 */
class Router_multipart_form_data : public Router
{
public:
    std::function<void(uint64_t cid, const REQUEST_PTR& req)> methodNotAllowedCb;
    std::function<void(uint64_t cid, const REQUEST_PTR& req)> headCb;
    std::function<void(uint64_t cid, const REQUEST_PTR& req, const std::string& name, const std::string& contentType,
                       const std::string& text)>
        textCb;
    std::function<void(uint64_t cid, const REQUEST_PTR& req, const std::string& name, const std::string& filename,
                       const std::string& contentType, size_t offset, const unsigned char* data, int dataLen, bool finish)>
        fileCb;
    std::function<void(uint64_t cid, const REQUEST_PTR& req, const SEND_RESPONSE_FUNC& sendRespFunc)> respHandler;

protected:
    void onMethodNotAllowed(uint64_t cid, const REQUEST_PTR& req) override;
    void onReqHead(uint64_t cid, const REQUEST_PTR& req) override;
    void onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    void onResponse(uint64_t cid, const REQUEST_PTR& req, const SEND_RESPONSE_FUNC& sendRespFunc) override;

private:
    std::mutex m_mutex;
    std::unordered_map<uint64_t, std::shared_ptr<MultipartFormData>> m_formMap;
};
} // namespace http
} // namespace nsocket
