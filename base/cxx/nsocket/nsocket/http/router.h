#pragma once
#include <functional>
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
 * @brief ·��
 */
class Router
{
    friend class Server;

protected:
    /**
     * @brief �յ��ͻ�������ͷ
     */
    virtual void onReqHead(int64_t sid, const REQUEST_PTR& req);

    /**
     * @brief �յ��ͻ�����������
     */
    virtual void onReqContent(int64_t sid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen);

    /**
     * @brief ��Ӧ�ͻ���
     */
    virtual RESPONSE_PTR onResponse(int64_t sid, const REQUEST_PTR& req);
};

/**
 * @brief ·��(���������ν���, ��Դ�����������, �����ϴ��ļ�֮���)
 */
class Router_batch : public Router
{
public:
    std::function<void(int64_t sid, const REQUEST_PTR& req)> headCb;
    std::function<void(int64_t sid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)> contentCb;
    std::function<RESPONSE_PTR(int64_t sid, const REQUEST_PTR& req)> respHandler;

protected:
    void onReqHead(int64_t sid, const REQUEST_PTR& req) override;
    void onReqContent(int64_t sid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    RESPONSE_PTR onResponse(int64_t sid, const REQUEST_PTR& req) override;
};

/**
 * @brief ·��(����һ���Խ���, ���С������������)
 */
class Router_simple : public Router
{
public:
    std::function<RESPONSE_PTR(const REQUEST_PTR& req, const std::string& data)> respHandler;

protected:
    void onReqHead(int64_t sid, const REQUEST_PTR& req) override;
    void onReqContent(int64_t sid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    RESPONSE_PTR onResponse(int64_t sid, const REQUEST_PTR& req) override;

private:
    std::unordered_map<int64_t, std::string> m_dataMap;
};

/**
 * @brief ·��(application/x-www-form-urlencoded)
 */
class Router_x_www_form_urlencoded : public Router
{
public:
    std::function<RESPONSE_PTR(const REQUEST_PTR& req, const CaseInsensitiveMultimap& fields)> respHandler;

protected:
    void onReqHead(int64_t sid, const REQUEST_PTR& req) override;
    void onReqContent(int64_t sid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    RESPONSE_PTR onResponse(int64_t sid, const REQUEST_PTR& req) override;

private:
    struct Wrapper
    {
        CaseInsensitiveMultimap fields;
        bool tmpKeyFlag = true;
        std::string tmpKey;
        std::string tmpValue;
    };

    std::unordered_map<int64_t, std::shared_ptr<Wrapper>> m_wrapperMap;
};

/**
 * @brief ·��(multipart/form-data)
 */
class Router_multipart_form_data : public Router
{
public:
    std::function<void(int64_t sid, const REQUEST_PTR& req)> headCb;
    std::function<void(int64_t sid, const REQUEST_PTR& req, const std::string& name, const std::string& contentType,
                       const std::string& text)>
        textCb;
    std::function<void(int64_t sid, const REQUEST_PTR& req, const std::string& name, const std::string& filename,
                       const std::string& contentType, size_t offset, const unsigned char* data, int dataLen, bool finish)>
        fileCb;
    std::function<RESPONSE_PTR(int64_t sid, const REQUEST_PTR& req)> respHandler;

protected:
    void onReqHead(int64_t sid, const REQUEST_PTR& req) override;
    void onReqContent(int64_t sid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) override;
    RESPONSE_PTR onResponse(int64_t sid, const REQUEST_PTR& req) override;

private:
    std::unordered_map<int64_t, std::shared_ptr<MultipartFormData>> m_formMap;
};
} // namespace http
} // namespace nsocket
