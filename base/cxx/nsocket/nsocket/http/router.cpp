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

std::vector<Method> Router::getAllowMethods()
{
    return m_methods;
}

void Router::onMethodNotAllowed(int64_t cid, const REQUEST_PTR& req) {}

void Router::onReqHead(int64_t cid, const REQUEST_PTR& req) {}

void Router::onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) {}

RESPONSE_PTR Router::onResponse(int64_t cid, const REQUEST_PTR& req)
{
    return nullptr;
}

void Router_batch::onMethodNotAllowed(int64_t cid, const REQUEST_PTR& req)
{
    if (methodNotAllowedCb)
    {
        methodNotAllowedCb(cid, req);
    }
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

void Router_simple::onMethodNotAllowed(int64_t cid, const REQUEST_PTR& req)
{
    if (methodNotAllowedCb)
    {
        methodNotAllowedCb(cid, req);
    }
}

void Router_simple::onReqHead(int64_t cid, const REQUEST_PTR& req)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_contentMap.end() == m_contentMap.find(cid))
    {
        m_contentMap.insert(std::make_pair(cid, std::make_shared<std::string>()));
    }
}

void Router_simple::onReqContent(int64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)
{
    if (!data || dataLen <= 0)
    {
        return;
    }
    std::shared_ptr<std::string> content = nullptr;
    {
        /* 限定锁区间, 避免阻塞其他路由, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_contentMap.find(cid);
        if (m_contentMap.end() != iter)
        {
            content = iter->second;
        }
    }
    if (content)
    {
        content->insert(content->end(), data, data + dataLen);
    }
}

RESPONSE_PTR Router_simple::onResponse(int64_t cid, const REQUEST_PTR& req)
{
    std::shared_ptr<std::string> content = nullptr;
    {
        /* 限定锁区间, 避免阻塞其他路由, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_contentMap.find(cid);
        if (m_contentMap.end() != iter)
        {
            content = iter->second;
            m_contentMap.erase(iter);
        }
    }
    RESPONSE_PTR resp = nullptr;
    if (content)
    {
        if (respHandler)
        {
            resp = respHandler(cid, req, *content.get());
        }
    }
    return resp;
}

void Router_x_www_form_urlencoded::onMethodNotAllowed(int64_t cid, const REQUEST_PTR& req)
{
    if (methodNotAllowedCb)
    {
        methodNotAllowedCb(cid, req);
    }
}

void Router_x_www_form_urlencoded::onReqHead(int64_t cid, const REQUEST_PTR& req)
{
    if (!case_insensitive_equal("application/x-www-form-urlencoded", req->getContentType()))
    {
        return;
    }
    std::lock_guard<std::mutex> locker(m_mutex);
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
    std::shared_ptr<Wrapper> wrapper = nullptr;
    {
        /* 限定锁区间, 避免阻塞其他路由, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_wrapperMap.find(cid);
        if (m_wrapperMap.end() != iter)
        {
            wrapper = iter->second;
        }
    }
    if (!wrapper)
    {
        return;
    }
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
    std::shared_ptr<Wrapper> wrapper = nullptr;
    {
        /* 限定锁区间, 避免阻塞其他路由, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_wrapperMap.find(cid);
        if (m_wrapperMap.end() != iter)
        {
            wrapper = iter->second;
            m_wrapperMap.erase(iter);
        }
    }
    if (!wrapper)
    {
        return nullptr;
    }
    if (!wrapper->tmpKey.empty())
    {
        wrapper->fields.insert(std::make_pair(wrapper->tmpKey, wrapper->tmpValue));
    }
    RESPONSE_PTR resp = nullptr;
    if (respHandler)
    {
        resp = respHandler(cid, req, wrapper->fields);
    }
    return resp;
}

void Router_multipart_form_data::onMethodNotAllowed(int64_t cid, const REQUEST_PTR& req)
{
    if (methodNotAllowedCb)
    {
        methodNotAllowedCb(cid, req);
    }
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
    std::lock_guard<std::mutex> locker(m_mutex);
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
    std::shared_ptr<MultipartFormData> formData = nullptr;
    {
        /* 限定锁区间, 避免阻塞其他路由, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_formMap.find(cid);
        if (m_formMap.end() != iter)
        {
            formData = iter->second;
        }
    }
    if (!formData)
    {
        return;
    }
    int used = formData->parse(
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
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_formMap.find(cid);
        if (m_formMap.end() != iter)
        {
            m_formMap.erase(iter);
        }
    }
}

RESPONSE_PTR Router_multipart_form_data::onResponse(int64_t cid, const REQUEST_PTR& req)
{
    {
        /* 限定锁区间, 避免阻塞其他路由, 提高并发性 */
        std::lock_guard<std::mutex> locker(m_mutex);
        auto iter = m_formMap.find(cid);
        if (m_formMap.end() != iter)
        {
            m_formMap.erase(iter);
        }
    }
    RESPONSE_PTR resp = nullptr;
    if (respHandler)
    {
        resp = respHandler(cid, req);
    }
    return resp;
}
} // namespace http
} // namespace nsocket
