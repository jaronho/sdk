#include "curlex.h"

#include <stdio.h>

namespace curlex
{
/**
 * @brief 根据请求参数创建Curl对象
 * @param req 请求参数
 * @param funcSet 函数集
 * @return Curl对象
 */
std::shared_ptr<CurlObject> createCurlObject(const RequestPtr& req, const FuncSet& funcSet)
{
    /* step1: 创建对象 */
    std::shared_ptr<CurlObject> obj;
    switch (req->getType())
    {
    case Request::Type::simple: {
        obj = std::make_shared<CurlObject>();
        break;
    }
    case Request::Type::ssl_1way: {
        auto sslReq = std::dynamic_pointer_cast<Ssl1WayRequest>(req);
        obj = std::make_shared<CurlObject>(sslReq->getCaFile());
        break;
    }
    case Request::Type::ssl_2way: {
        auto sslReq = std::dynamic_pointer_cast<Ssl2WayRequest>(req);
        obj = std::make_shared<CurlObject>(sslReq->getFileFormat(), sslReq->getCertFile(), sslReq->getPrivateKeyFile(),
                                           sslReq->getPrivateKeyFilePwd());
        break;
    }
    case Request::Type::user_pwd: {
        auto userpwdReq = std::dynamic_pointer_cast<UserpwdRequest>(req);
        obj = std::make_shared<CurlObject>(userpwdReq->getUsername(), userpwdReq->getPassword());
        break;
    }
    default:
        return obj;
    }
    /* step2: 设置公共属性 */
    obj->setUrl(req->getUrl());
    if (req->isEnableRedirect())
    {
        obj->setEnableRedirect(req->getMaxRedirects());
    }
    int connectTimeout = req->getConnectTimeout();
    if (connectTimeout >= 0)
    {
        obj->setConnectTimeout(static_cast<size_t>(connectTimeout));
    }
    int timeout = req->getTimeout();
    if (timeout >= 0)
    {
        obj->setTimeout(static_cast<size_t>(timeout));
    }
    if (req->isKeepAlive())
    {
        obj->setKeepAlive(req->getKeepAliveIdle(), req->getKeepAliveInterval());
    }
    auto headers = req->getHeaders();
    if (!headers.empty())
    {
        obj->setHeaders(headers);
    }
    auto cookieFile = req->getCookieFile();
    if (!cookieFile.empty())
    {
        obj->setCookieFile(cookieFile);
    }
    /* step3: 设置函数 */
    obj->setProgressFunc([&funcSet](int64_t nowUploaded, int64_t totalUpload, double uploadSpeed, int64_t nowDownloaded,
                                    int64_t totalDownload, double downloadSpeed) {
        if (funcSet.isStopFunc && funcSet.isStopFunc())
        {
            return true;
        }
        if (funcSet.sendProgressFunc)
        {
            funcSet.sendProgressFunc(nowUploaded, totalUpload, uploadSpeed);
        }
        if (funcSet.recvProgressFunc)
        {
            funcSet.recvProgressFunc(nowDownloaded, totalDownload, downloadSpeed);
        }
        return false;
    });
    obj->setDebugFunc(funcSet.debugFunc);
    /* step4: 设置数据 */
    auto reqData = req->getData();
    if (reqData)
    {
        switch (reqData->getType())
        {
        case RequestData::Type::raw: {
            auto rawReqData = std::dynamic_pointer_cast<RawRequestData>(reqData);
            obj->setRawData(rawReqData->getBytes(), rawReqData->isChunk());
            break;
        }
        case RequestData::Type::form: {
            auto formReqData = std::dynamic_pointer_cast<FormRequestData>(reqData);
            obj->setFormData(formReqData->getFieldMap());
            break;
        }
        case RequestData::Type::multipart_form: {
            auto multipartFormReqData = std::dynamic_pointer_cast<MultipartFormRequestData>(reqData);
            auto textMap = multipartFormReqData->getTextMap();
            auto textIter = textMap.begin();
            for (; textMap.end() != textIter; ++textIter)
            {
                obj->addMultipartFormText(textIter->first, textIter->second.text, textIter->second.contentType);
            }
            auto fileMap = multipartFormReqData->getFileMap();
            auto fileIter = fileMap.begin();
            for (; fileMap.end() != fileIter; ++fileIter)
            {
                obj->addMultipartFormFile(fileIter->first, fileIter->second);
            }
            break;
        }
        }
    }
    return obj;
}

bool curlDelete(const RequestPtr& req, const FuncSet& funcSet, Response& resp)
{
    resp.url = req->getUrl();
    auto obj = createCurlObject(req, funcSet);
    auto code = obj->setOption(CURLOPT_CUSTOMREQUEST, "DELETE");
    if (CURLE_OK != code)
    {
        return false;
    }
    obj->setRecvFunc([&resp](void* bytes, size_t count) {
        resp.body.append(static_cast<char*>(bytes), count);
        return count;
    });
    return obj->perform(resp.localIp, resp.localPort, resp.remoteIp, resp.remotePort, resp.curlCode, resp.errorDesc, resp.httpCode,
                        resp.headers, resp.elapsed);
}

bool curlGet(const RequestPtr& req, const FuncSet& funcSet, Response& resp)
{
    resp.url = req->getUrl();
    auto obj = createCurlObject(req, funcSet);
    obj->setRecvFunc([&resp](void* bytes, size_t count) {
        resp.body.append(static_cast<char*>(bytes), count);
        return count;
    });
    return obj->perform(resp.localIp, resp.localPort, resp.remoteIp, resp.remotePort, resp.curlCode, resp.errorDesc, resp.httpCode,
                        resp.headers, resp.elapsed);
}

bool curlPut(const RequestPtr& req, const FuncSet& funcSet, Response& resp)
{
    resp.url = req->getUrl();
    auto obj = createCurlObject(req, funcSet);
    auto code = obj->setOption(CURLOPT_PUT, 1L);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = obj->setOption(CURLOPT_UPLOAD, 1L);
    if (CURLE_OK != code)
    {
        return false;
    }
    obj->setRecvFunc([&resp](void* bytes, size_t count) {
        resp.body.append(static_cast<char*>(bytes), count);
        return count;
    });
    return obj->perform(resp.localIp, resp.localPort, resp.remoteIp, resp.remotePort, resp.curlCode, resp.errorDesc, resp.httpCode,
                        resp.headers, resp.elapsed);
}

bool curlPost(const RequestPtr& req, const FuncSet& funcSet, Response& resp)
{
    resp.url = req->getUrl();
    auto obj = createCurlObject(req, funcSet);
    auto code = obj->setOption(CURLOPT_POST, 1L);
    if (CURLE_OK != code)
    {
        return false;
    }
    obj->setRecvFunc([&resp](void* bytes, size_t count) {
        resp.body.append(static_cast<char*>(bytes), count);
        return count;
    });
    return obj->perform(resp.localIp, resp.localPort, resp.remoteIp, resp.remotePort, resp.curlCode, resp.errorDesc, resp.httpCode,
                        resp.headers, resp.elapsed);
}

bool curlDownload(const RequestPtr& req, const std::string& filename, bool recover, const FuncSet& funcSet, Response& resp)
{
    resp.url = req->getUrl();
    if (filename.empty())
    {
        return false;
    }
    auto f = fopen(filename.c_str(), recover ? "wb+" : "ab+");
    if (!f)
    {
        return false;
    }
#ifdef _WIN32
    _fseeki64(f, 0, SEEK_END);
    auto offset = _ftelli64(f);
#else
    fseeko64(f, 0, SEEK_END);
    auto offset = ftello64(f);
#endif
    auto obj = createCurlObject(req, funcSet);
    if (offset > 0)
    {
        obj->setResumeOffset(offset);
    }
    obj->setRecvFunc([&f](void* bytes, size_t count) { return fwrite(bytes, 1, count, f); });
    auto ret = obj->perform(resp.localIp, resp.localPort, resp.remoteIp, resp.remotePort, resp.curlCode, resp.errorDesc, resp.httpCode,
                            resp.headers, resp.elapsed);
    fflush(f);
    fclose(f);
    return ret;
}
} // namespace curlex
