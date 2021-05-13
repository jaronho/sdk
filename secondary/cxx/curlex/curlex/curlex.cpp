#include "curlex.h"

#include <iostream>

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
    case Request::Type::SIMPLE: {
        obj = std::make_shared<CurlObject>();
        break;
    }
    case Request::Type::CAFILE: {
        auto cafileReq = std::dynamic_pointer_cast<CafileRequest>(req);
        obj = std::make_shared<CurlObject>(cafileReq->getCaFilename());
        break;
    }
    case Request::Type::USERPWD: {
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
        obj->setEnableRedirect();
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
    obj->setProgressFunc([&](int64_t nowUploaded, int64_t totalUpload, double uploadSpeed, int64_t nowDownloaded, int64_t totalDownload,
                             double downloadSpeed) {
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
        case RequestData::Type::RAW: {
            auto rawReqData = std::dynamic_pointer_cast<RawRequestData>(reqData);
            obj->setRawData(rawReqData->getBytes(), rawReqData->getByteCount());
            break;
        }
        case RequestData::Type::FORM: {
            auto textReqData = std::dynamic_pointer_cast<FormRequestData>(reqData);
            obj->setFormData(textReqData->getData());
            break;
        }
        case RequestData::Type::MULTIPART_FORM: {
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

bool curlGet(const RequestPtr& req, const FuncSet& funcSet, SimpleResponse& resp)
{
    auto obj = createCurlObject(req, funcSet);
    obj->setRecvFunc([&](void* bytes, size_t count) {
        resp.body.append(static_cast<char*>(bytes), count);
        return count;
    });
    return obj->perform(resp.curlCode, resp.errorDesc, resp.httpCode, resp.headers);
}

bool curlPost(const RequestPtr& req, const FuncSet& funcSet, SimpleResponse& resp)
{
    auto obj = createCurlObject(req, funcSet);
    auto code = obj->setOption(CURLOPT_POST, 1L);
    if (CURLE_OK != code)
    {
        return false;
    }
    obj->setRecvFunc([&](void* bytes, size_t count) {
        resp.body.append(static_cast<char*>(bytes), count);
        return count;
    });
    return obj->perform(resp.curlCode, resp.errorDesc, resp.httpCode, resp.headers);
}

bool curlPut(const RequestPtr& req, const FuncSet& funcSet, SimpleResponse& resp)
{
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
    obj->setRecvFunc([&](void* bytes, size_t count) {
        resp.body.append(static_cast<char*>(bytes), count);
        return count;
    });
    return obj->perform(resp.curlCode, resp.errorDesc, resp.httpCode, resp.headers);
}

bool curlDelete(const RequestPtr& req, const FuncSet& funcSet, SimpleResponse& resp)
{
    auto obj = createCurlObject(req, funcSet);
    auto code = obj->setOption(CURLOPT_CUSTOMREQUEST, "DELETE");
    if (CURLE_OK != code)
    {
        return false;
    }
    obj->setRecvFunc([&](void* bytes, size_t count) {
        resp.body.append(static_cast<char*>(bytes), count);
        return count;
    });
    return obj->perform(resp.curlCode, resp.errorDesc, resp.httpCode, resp.headers);
}

bool curlDownload(const RequestPtr& req, const std::string& filename, bool recover, const FuncSet& funcSet, Response& resp)
{
    if (filename.empty())
    {
        return false;
    }
#ifdef _WIN32
    FILE* fp;
    if (0 != fopen_s(&fp, filename.c_str(), recover ? "wb" : "ab+"))
#else
    FILE* fp = fopen(filename.c_str(), recover ? "wb" : "ab+");
    if (!fp)
#endif
    {
        return false;
    }
    fseek(fp, 0L, SEEK_END);
    auto offset = ftell(fp);
    auto obj = createCurlObject(req, funcSet);
    if (offset > 0)
    {
        obj->setResumeOffset(offset);
    }
    obj->setRecvFunc([&](void* bytes, size_t count) {
        size_t writeCount = fwrite(bytes, 1, count, fp);
        return writeCount;
    });
    auto ret = obj->perform(resp.curlCode, resp.errorDesc, resp.httpCode, resp.headers);
    fclose(fp);
    return ret;
}
} // namespace curlex
