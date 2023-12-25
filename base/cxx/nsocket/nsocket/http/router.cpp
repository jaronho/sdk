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

Connector::Connector(const std::function<void(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& cb)>& sendFunc,
                     const std::function<void()>& closeFunc)
    : m_sendFunc(sendFunc), m_closeFunc(closeFunc)
{
}

void Connector::close() const
{
    if (m_closeFunc)
    {
        m_closeFunc();
    }
}

void Connector::send(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& cb) const
{
    if (m_sendFunc)
    {
        m_sendFunc(data, cb);
    }
}

void Connector::sendAndClose(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& cb) const
{
    if (m_sendFunc)
    {
        m_sendFunc(data, cb);
    }
    if (m_closeFunc)
    {
        m_closeFunc();
    }
}

std::vector<Method> Router::getAllowMethods()
{
    return m_methods;
}

void Router::onReqHead(uint64_t cid, const REQUEST_PTR& req) {}

void Router::onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen) {}

void Router::onResponse(uint64_t cid, const REQUEST_PTR& req, const Connector& conn)
{
    conn.close();
}

void Router_batch::onReqHead(uint64_t cid, const REQUEST_PTR& req)
{
    if (headCb)
    {
        headCb(cid, req);
    }
}

void Router_batch::onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)
{
    if (contentCb)
    {
        contentCb(cid, req, offset, data, dataLen);
    }
}

void Router_batch::onResponse(uint64_t cid, const REQUEST_PTR& req, const Connector& conn)
{
    if (respHandler)
    {
        respHandler(cid, req, conn);
    }
    else
    {
        conn.close();
    }
}

void Router_simple::onReqHead(uint64_t cid, const REQUEST_PTR& req)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_contentMap.end() == m_contentMap.find(cid))
    {
        m_contentMap.insert(std::make_pair(cid, std::make_shared<std::string>()));
    }
}

void Router_simple::onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)
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

void Router_simple::onResponse(uint64_t cid, const REQUEST_PTR& req, const Connector& conn)
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
    if (content && respHandler)
    {
        respHandler(cid, req, *content.get(), conn);
    }
    else
    {
        conn.close();
    }
}

void Router_x_www_form_urlencoded::onReqHead(uint64_t cid, const REQUEST_PTR& req)
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

void Router_x_www_form_urlencoded::onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)
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
            wrapper->fields.insert(std::make_pair(url_decode(wrapper->tmpKey), url_decode(wrapper->tmpValue)));
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

void Router_x_www_form_urlencoded::onResponse(uint64_t cid, const REQUEST_PTR& req, const Connector& conn)
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
    if (wrapper && respHandler)
    {
        if (!wrapper->tmpKey.empty())
        {
            wrapper->fields.insert(std::make_pair(url_decode(wrapper->tmpKey), url_decode(wrapper->tmpValue)));
        }
        respHandler(cid, req, wrapper->fields, conn);
    }
    else
    {
        conn.close();
    }
}

void Router_multipart_form_data::onReqHead(uint64_t cid, const REQUEST_PTR& req)
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

void Router_multipart_form_data::onReqContent(uint64_t cid, const REQUEST_PTR& req, size_t offset, const unsigned char* data, int dataLen)
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

void Router_multipart_form_data::onResponse(uint64_t cid, const REQUEST_PTR& req, const Connector& conn)
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
    if (respHandler)
    {
        respHandler(cid, req, conn);
    }
    else
    {
        conn.close();
    }
}
} // namespace http
} // namespace nsocket
