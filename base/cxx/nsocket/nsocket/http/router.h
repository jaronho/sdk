#pragma once
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

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

protected:
    /**
     * @brief 收到客户端请求头
     */
    virtual void onReqHead(int64_t cid, const REQUEST_PTR& req);

    /**
     * @brief 收到客户端请求内容
     */
    virtual void onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen);

    /**
     * @brief 响应客户端
     */
    virtual RESPONSE_PTR onResponse(int64_t cid, const REQUEST_PTR& req);
};

/**
 * @brief 路由(对内容批次接收, 针对大数量的请求, 比如上传文件之类的)
 */
class Router_batch final : public Router
{
public:
    std::function<void(int64_t cid, const REQUEST_PTR& req)> headCb;
    std::function<void(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)> contentCb;
    std::function<RESPONSE_PTR(int64_t cid, const REQUEST_PTR& req)> respHandler;

protected:
    void onReqHead(int64_t cid, const REQUEST_PTR& req) override;
    void onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    RESPONSE_PTR onResponse(int64_t cid, const REQUEST_PTR& req) override;
};

/**
 * @brief 路由(内容一次性接收, 针对小数据量的请求)
 */
class Router_simple final : public Router
{
public:
    std::function<RESPONSE_PTR(const REQUEST_PTR& req, const std::string& data)> respHandler;

protected:
    void onReqHead(int64_t cid, const REQUEST_PTR& req) override;
    void onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    RESPONSE_PTR onResponse(int64_t cid, const REQUEST_PTR& req) override;

private:
    std::mutex m_mutex;
    std::unordered_map<int64_t, std::shared_ptr<std::string>> m_contentMap;
};

/**
 * @brief 路由(application/x-www-form-urlencoded)
 */
class Router_x_www_form_urlencoded final : public Router
{
public:
    std::function<RESPONSE_PTR(const REQUEST_PTR& req, const CaseInsensitiveMultimap& fields)> respHandler;

protected:
    void onReqHead(int64_t cid, const REQUEST_PTR& req) override;
    void onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    RESPONSE_PTR onResponse(int64_t cid, const REQUEST_PTR& req) override;

private:
    struct Wrapper
    {
        CaseInsensitiveMultimap fields;
        bool tmpKeyFlag = true;
        std::string tmpKey;
        std::string tmpValue;
    };

    std::mutex m_mutex;
    std::unordered_map<int64_t, std::shared_ptr<Wrapper>> m_wrapperMap;
};

/**
 * @brief 路由(multipart/form-data)
 */
class Router_multipart_form_data final : public Router
{
public:
    std::function<void(int64_t cid, const REQUEST_PTR& req)> headCb;
    std::function<void(int64_t cid, const REQUEST_PTR& req, const std::string& name, const std::string& contentType,
                       const std::string& text)>
        textCb;
    std::function<void(int64_t cid, const REQUEST_PTR& req, const std::string& name, const std::string& filename,
                       const std::string& contentType, size_t offset, const unsigned char* data, int dataLen, bool finish)>
        fileCb;
    std::function<RESPONSE_PTR(int64_t cid, const REQUEST_PTR& req)> respHandler;

protected:
    void onReqHead(int64_t cid, const REQUEST_PTR& req) override;
    void onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    RESPONSE_PTR onResponse(int64_t cid, const REQUEST_PTR& req) override;

private:
    std::mutex m_mutex;
    std::unordered_map<int64_t, std::shared_ptr<MultipartFormData>> m_formMap;
};
} // namespace http
} // namespace nsocket