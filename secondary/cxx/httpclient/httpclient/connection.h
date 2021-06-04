#pragma once

#include <atomic>
#include <string>

#include "http_client.h"

namespace http
{
/**
 * @brief HTTP连接
 */
class Connection final
{
public:
    Connection(const std::string& url);

    Connection(const std::string& sslCaFilename, const std::string& url);

    Connection(const std::string& username, const std::string& password, const std::string& url);

    virtual ~Connection() = default;

    void setConnectTimeout(size_t seconds);

    void setTimeout(size_t seconds);

    void appendHeader(const std::string& key, const std::string& value);

    void setStopFunc(const std::function<bool()>& func);

    void setSendProgressFunc(const std::function<void(int64_t now, int64_t total, double speed)>& func);

    void setRecvProgressFunc(const std::function<void(int64_t now, int64_t total, double speed)>& func);

    bool setRawData(const unsigned char* bytes, size_t count);

    bool setFormData(const std::string& data);

    bool appendContentText(const std::string& key, const std::string& value, const std::string& contentType = std::string());

    bool appendContentFile(const std::string& key, const std::string& filename);

    std::string toString();

    void doDelete(const ResponseCallback& respCb, bool asyncOp = true);

    void doGet(const ResponseCallback& respCb, bool asyncOp = true);

    void doPut(const ResponseCallback& respCb, bool asyncOp = true);

    void doPost(const ResponseCallback& respCb, bool asyncOp = true);

    void doDownload(const std::string& filename, bool recover, const ResponseCallback& respCb, bool asyncOp = true);

private:
    curlex::RequestPtr m_req = nullptr; /* 请求对象 */
    curlex::RequestDataPtr m_data = nullptr; /* 请求数据 */
    curlex::FuncSet m_funcSet; /* 函数集 */
    ResponseCallback m_respCallback = nullptr; /* 响应回调 */
};
} // namespace http
