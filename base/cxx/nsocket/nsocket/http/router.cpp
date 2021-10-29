#include "router.h"

namespace nsocket
{
namespace http
{
static bool parseMultipartFormDataBoundary(const std::string& contentType, std::string& boundary)
{
    static const std::string TYPE_TAG = "multipart/form-data";
    static const std::string BOUNDARY_TAG = ";boundary=";
    boundary.clear();
    if (contentType.size() < TYPE_TAG.size())
    {
        return false;
    }
    for (size_t i = 0; i < contentType.size(); ++i)
    {
        const auto& ch = contentType[i];
        if (i < TYPE_TAG.size())
        {
            if (tolower(ch) != tolower(TYPE_TAG[i]))
            {
                return false;
            }
        }
        else if (i - TYPE_TAG.size() < BOUNDARY_TAG.size())
        {
            if (tolower(ch) != tolower(BOUNDARY_TAG[i - TYPE_TAG.size()]))
            {
                return false;
            }
        }
        else
        {
            boundary.push_back(ch);
        }
    }
    return true;
}

void Router::onReqHead(int64_t cid, const REQUEST_PTR& req) {}

void Router::onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) {}

RESPONSE_PTR Router::onResponse(int64_t cid, const REQUEST_PTR& req)
{
    return std::make_shared<Response>();
}

void Router_batch::onReqHead(int64_t cid, const REQUEST_PTR& req)
{
    if (headCb)
    {
        headCb(cid, req);
    }
}

void Router_batch::onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)
{
    if (contentCb)
    {
        contentCb(cid, req, offset, data, dataLen);
    }
}

RESPONSE_PTR Router_batch::onResponse(int64_t cid, const REQUEST_PTR& req)
{
    if (respHandler)
    {
        return respHandler(cid, req);
    }
    return nullptr;
}

void Router_simple::onReqHead(int64_t cid, const REQUEST_PTR& req)
{
    if (m_dataMap.end() == m_dataMap.find(cid))
    {
        m_dataMap.insert(std::make_pair(cid, std::string()));
    }
}

void Router_simple::onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)
{
    if (!data || dataLen <= 0)
    {
        return;
    }
    auto iter = m_dataMap.find(cid);
    if (m_dataMap.end() == iter)
    {
        return;
    }
    iter->second.insert(iter->second.end(), data, data + dataLen);
}

RESPONSE_PTR Router_simple::onResponse(int64_t cid, const REQUEST_PTR& req)
{
    auto iter = m_dataMap.find(cid);
    if (m_dataMap.end() == iter)
    {
        return nullptr;
    }
    RESPONSE_PTR resp = nullptr;
    if (respHandler)
    {
        resp = respHandler(req, iter->second);
    }
    m_dataMap.erase(iter);
    return resp;
}

void Router_x_www_form_urlencoded::onReqHead(int64_t cid, const REQUEST_PTR& req)
{
    if (!case_insensitive_equal("application/x-www-form-urlencoded", req->getContentType()))
    {
        return;
    }
    if (m_wrapperMap.end() == m_wrapperMap.find(cid))
    {
        m_wrapperMap.insert(std::make_pair(cid, std::make_shared<Wrapper>()));
    }
}

void Router_x_www_form_urlencoded::onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)
{
    if (!data || dataLen <= 0)
    {
        return;
    }
    auto iter = m_wrapperMap.find(cid);
    if (m_wrapperMap.end() == iter)
    {
        return;
    }
    auto wrapper = iter->second;
    for (int i = 0; i < dataLen; ++i)
    {
        const auto& ch = data[i];
        if ('=' == ch)
        {
            if (wrapper->tmpKey.empty())
            {
                return;
            }
            if (wrapper->tmpKeyFlag)
            {
                wrapper->tmpKeyFlag = false;
            }
            else
            {
                wrapper->tmpValue.push_back(ch);
            }
        }
        else if ('&' == ch)
        {
            if (wrapper->tmpKey.empty())
            {
                return;
            }
            wrapper->fields.insert(std::make_pair(wrapper->tmpKey, wrapper->tmpValue));
            wrapper->tmpKeyFlag = true;
            wrapper->tmpKey.clear();
            wrapper->tmpValue.clear();
        }
        else
        {
            if (wrapper->tmpKeyFlag)
            {
                wrapper->tmpKey.push_back(ch);
            }
            else
            {
                wrapper->tmpValue.push_back(ch);
            }
        }
    }
}

RESPONSE_PTR Router_x_www_form_urlencoded::onResponse(int64_t cid, const REQUEST_PTR& req)
{
    auto iter = m_wrapperMap.find(cid);
    if (m_wrapperMap.end() == iter)
    {
        return nullptr;
    }
    auto wrapper = iter->second;
    if (!wrapper->tmpKey.empty())
    {
        wrapper->fields.insert(std::make_pair(wrapper->tmpKey, wrapper->tmpValue));
    }
    RESPONSE_PTR resp = nullptr;
    if (respHandler)
    {
        resp = respHandler(req, wrapper->fields);
    }
    m_wrapperMap.erase(iter);
    return resp;
}

void Router_multipart_form_data::onReqHead(int64_t cid, const REQUEST_PTR& req)
{
    std::string boundary;
    if (!parseMultipartFormDataBoundary(req->getContentType(), boundary))
    {
        return;
    }
    if (headCb)
    {
        headCb(cid, req);
    }
    if (m_formMap.end() == m_formMap.find(cid))
    {
        m_formMap.insert(std::make_pair(cid, std::make_shared<MultipartFormData>(boundary)));
    }
}

void Router_multipart_form_data::onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)
{
    if (!data || dataLen <= 0)
    {
        return;
    }
    auto iter = m_formMap.find(cid);
    if (m_formMap.end() == iter)
    {
        return;
    }
    int used = iter->second->parse(
        data, dataLen,
        [&](const std::string& name, const std::string& contentType, const std::string& text) {
            if (textCb)
            {
                textCb(cid, req, name, contentType, text);
            }
        },
        [&](const std::string& name, const std::string& filename, const std::string& contentType, size_t offset, const unsigned char* data,
            int dataLen, bool finish) {
            if (fileCb)
            {
                fileCb(cid, req, name, filename, contentType, offset, data, dataLen, finish);
            }
        });
    if (used <= 0) /* 解析失败 */
    {
        m_formMap.erase(iter);
    }
}

RESPONSE_PTR Router_multipart_form_data::onResponse(int64_t cid, const REQUEST_PTR& req)
{
    auto iter = m_formMap.find(cid);
    if (m_formMap.end() == iter)
    {
        return nullptr;
    }
    RESPONSE_PTR resp = nullptr;
    if (respHandler)
    {
        resp = respHandler(cid, req);
    }
    m_formMap.erase(iter);
    return resp;
}
} // namespace http
} // namespace nsocket
