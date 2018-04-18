/**********************************************************************
* Author:	jaron.ho
* Date:		2018-04-10
* Brief:	http server
**********************************************************************/
#include "HttpServer.h"
static bool startHttpServer(const char* ip, unsigned short port, void(*cb)(struct evhttp_request*, void*), void* arg);
static const char* httpMethodName(evhttp_cmd_type method);
static void httpServerCallback(struct evhttp_request* req, void* arg);
static char* handleHttpRequest(char major, char minor, const char* method, const char* host, unsigned short port, 
                               const struct evkeyvalq* headers, const unsigned char* body, unsigned int bodySize, 
                               const char* uri, struct evkeyvalq* responseHeaders);
//------------------------------------------------------------------------
static bool startHttpServer(const char* ip, unsigned short port, void (*cb)(struct evhttp_request*, void*), void* arg) {
    if (!ip || 0 == strlen(ip)) {
	    printf_s("start http server failed, ip is empty ...\n");
        return false;
    }
    if (0 == port) {
	    printf_s("start http server failed, port is 0 ...\n");
	    return false;
    }
    struct event_base* base = event_base_new();
    struct evhttp* http_server = evhttp_new(base);
    if (!http_server) {
	    printf_s("start http server failed, evhttp_new error ...\n");
        return false;
    }
    // bind address
    if (0 != evhttp_bind_socket(http_server, ip, port)) {
	    printf_s("start http server failed, bind socket %s:%u error ...\n", ip, port);
        return false;
    }
    // set http request handle callabck
    if (cb) {
        evhttp_set_gencb(http_server, cb, arg);
    }
    // start event loop, it will callback when http request triggered
    printf_s("start http server %s:%u ok ...\n", ip, port);
    event_base_dispatch(base);
    evhttp_free(http_server);
    return true;
}
//------------------------------------------------------------------------
static const char* httpMethodName(evhttp_cmd_type method) {
    if (EVHTTP_REQ_GET == method) {
        return "GET";
    } else if (EVHTTP_REQ_POST == method) {
        return "POST";
    } else if (EVHTTP_REQ_HEAD == method) {
        return "HEAD";
    } else if (EVHTTP_REQ_PUT == method) {
        return "PUT";
    } else if (EVHTTP_REQ_DELETE == method) {
        return "DELETE";
    } else if (EVHTTP_REQ_OPTIONS == method) {
        return "OPTIONS";
    } else if (EVHTTP_REQ_TRACE == method) {
        return "TRACE";
    } else if (EVHTTP_REQ_CONNECT == method) {
        return "CONNECT";
    } else if (EVHTTP_REQ_PATCH == method) {
        return "PATCH";
    }
    return "unknown";
}
//------------------------------------------------------------------------
static void httpServerCallback(struct evhttp_request* req, void* arg) {
    // cache client request info
    char major = req->major;                                                    // http major version
    char minor = req->minor;                                                    // http minor version
    const char* method = httpMethodName(evhttp_request_get_command(req));       // http method
    const char* host = evhttp_request_get_host(req);                            // client host
    unsigned short port = req->remote_port;                                     // client port
    const struct evkeyvalq* headers = evhttp_request_get_input_headers(req);    // client headers
    struct evbuffer* buffer = evhttp_request_get_input_buffer(req);             // client buffer
    const unsigned char* body = evbuffer_pullup(buffer, -1);                    // client body
    const char* uri = evhttp_request_get_uri(req);                              // server uri
    // handle request
    struct evkeyvalq* responseHeaders = evhttp_request_get_output_headers(req);
    char* responseBody = handleHttpRequest(major, minor, method, host, port, headers, body, req->body_size, uri, responseHeaders);
    // reply to client
    struct evbuffer* buf = evbuffer_new();
    if (!buf) {
        if (responseBody) {
            free(responseBody);
        }
        evhttp_send_error(req, HTTP_INTERNAL, "ERROR_BUFFER_ALLOCATE");
        return;
    }
    if (responseBody) {
        evbuffer_add_printf(buf, responseBody);
        free(responseBody);
    }
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}
//------------------------------------------------------------------------
static char* handleHttpRequest(char major, char minor, const char* method, const char* host, unsigned short port,
                               const struct evkeyvalq* headers, const unsigned char* body, unsigned int bodySize,
                               const char* uri, struct evkeyvalq* responseHeaders) {
    std::map<std::string, std::string> headerMap;
    std::map<std::string, HttpField*> bodyMap;
    std::string realUri = uri;
    char* responseBody = NULL;
    unsigned int errorCode = 0;
    char errorBuf[256] = { 0 };
    // parse header into map
    for (struct evkeyval* header = headers->tqh_first; header; header = header->next.tqe_next) {
        std::string headerKey = header->key;
        std::transform(headerKey.begin(), headerKey.end(), headerKey.begin(), ::tolower);
        headerMap[headerKey] = header->value;
    }
    // parse body into map
    if (0 == strcmp("GET", method)) {               // parse GET body
        struct evkeyvalq params;
        evhttp_parse_query(uri, &params);
        for (struct evkeyval* param = params.tqh_first; param; param = param->next.tqe_next) {
            HttpField* field = new HttpField();
            field->setName(param->key);
            field->setType(HttpField::TYPE_TEXT);
            field->setContent(param->value, strlen(param->value));
            if (bodyMap[param->key]) {
                delete bodyMap[param->key];
            }
            bodyMap[param->key] = field;
        }
        evhttp_clear_headers(&params);
    } else if (0 == strcmp("POST", method)) {       // parse POST body
        std::map<std::string, std::string>::iterator iter = headerMap.find("content-type");
        if (headerMap.end() != iter) {
            if ("application/x-www-form-urlencoded" == iter->second) {
                if (body) {
                    struct evkeyvalq params;
                    evhttp_parse_query_str((const char*)body, &params);
                    for (struct evkeyval* param = params.tqh_first; param; param = param->next.tqe_next) {
                        HttpField* field = new HttpField();
                        field->setName(param->key);
                        field->setType(HttpField::TYPE_TEXT);
                        field->setContent(param->value, strlen(param->value));
                        if (bodyMap[param->key]) {
                            delete bodyMap[param->key];
                        }
                        bodyMap[param->key] = field;
                    }
                    evhttp_clear_headers(&params);
                }
            } else if (std::string::npos != iter->second.find("multipart/form-data")) {
                MultipartFormData* forms = new MultipartFormData();
                if (!forms->parse(iter->second, (const char*)body, bodySize, &bodyMap)) {
                    errorCode = 4;
                    sprintf_s(errorBuf, "can not parse multipart form-data for uri '%s'", uri);
                }
                delete forms;
            } else {
                errorCode = 3;
                sprintf_s(errorBuf, "no support %s request which content-type is '%s'", method, iter->second.c_str());
            }
        } else {
            errorCode = 2;
            sprintf_s(errorBuf, "no support %s request which without content-type", method);
        }
    } else {                                        // parse other method
        errorCode = 1;
        sprintf_s(errorBuf, "no support %s request", method);
    }
    // parse real uri
    size_t pos = realUri.find_first_of('?');
    if (std::string::npos != pos) {
	    realUri = realUri.substr(0, pos);
    }
    // handle request
    std::map<std::string, std::string> reponseHeaderMap;
    std::string result;
    if (errorCode) {   // handle error
        result = HttpServer::getInstance()->handleError(major, minor, method, host, port, headerMap, bodyMap, realUri, errorCode, errorBuf, reponseHeaderMap);
    } else {           // handle router
        result = HttpServer::getInstance()->handleRouter(major, minor, method, host, port, headerMap, bodyMap, realUri, reponseHeaderMap);
    }
    // handle clear
    headerMap.clear();
    std::map<std::string, HttpField*>::iterator fieldIter = bodyMap.begin();
    for (; bodyMap.end() != fieldIter; ++fieldIter) {
        delete fieldIter->second;
    }
    bodyMap.clear();
    // handle response
    std::map<std::string, std::string>::iterator reponseHeaderIter = reponseHeaderMap.begin();
    for (; reponseHeaderMap.end() != reponseHeaderIter; ++reponseHeaderIter) {
        evhttp_add_header(responseHeaders, reponseHeaderIter->first.c_str(), reponseHeaderIter->second.c_str());
    }
    reponseHeaderMap.clear();
    if (!result.empty()) {
        responseBody = (char*)malloc(result.size() + 1);
        if (responseBody) {
            memcpy(responseBody, result.c_str(), result.size());
            *(responseBody + result.size()) = '\0';
        }
    }
    return responseBody;
}
//------------------------------------------------------------------------
HttpField::HttpField(void) {
    mName = "";
    mType = 0;
    mContent = NULL;
    mContentLength = 0;
    mFilename = "";
}
//------------------------------------------------------------------------
HttpField::~HttpField(void) {
    if (mContent) {
        delete mContent;
    }
}
//------------------------------------------------------------------------
std::string HttpField::getName(void) {
    return mName;
}
//------------------------------------------------------------------------
void HttpField::setName(const std::string& name) {
    mName = name;
}
//------------------------------------------------------------------------
unsigned int HttpField::getType(void) {
    return mType;
}
//------------------------------------------------------------------------
void HttpField::setType(unsigned int type) {
    if (TYPE_TEXT == type || TYPE_FILE == type) {
        mType = type;
    }
}
//------------------------------------------------------------------------
const char* HttpField::getContent(void) {
    return mContent;
}
//------------------------------------------------------------------------
void HttpField::setContent(const char* content, size_t length) {
    if (!mContent) {
        mContent = new char[TYPE_TEXT == mType ? length + 1 : length];
    } else {
        mContent = (char*)realloc(mContent, mContentLength + (TYPE_TEXT == mType ? length + 1 : length));
    }
    if (!mContent) {
        return;
    }
    memcpy(mContent + mContentLength, content, length);
    mContentLength += length;
    if (TYPE_TEXT == mType) {
        *(mContent + mContentLength) = '\0';
    }
}
//------------------------------------------------------------------------
size_t HttpField::getContentLength(void) {
    return mContentLength;
}
//------------------------------------------------------------------------
std::string HttpField::getFilename(void) {
    return mFilename;
}
//------------------------------------------------------------------------
void HttpField::setFilename(const std::string& filename) {
    mFilename = filename;
}
//------------------------------------------------------------------------
std::string HttpField::getFileContentType(void) {
    return mFileContentType;
}
//------------------------------------------------------------------------
void HttpField::setFileContentType(const std::string& fileContentType) {
    mFileContentType = fileContentType;
}
//------------------------------------------------------------------------
static HttpServer* mInstance = NULL;
//------------------------------------------------------------------------
HttpServer* HttpServer::getInstance(void) {
    if (!mInstance) {
        mInstance = new HttpServer();
    }
    return mInstance;
}
//------------------------------------------------------------------------
void HttpServer::destroyInstance(void) {
    if (mInstance) {
        std::map<std::string, HttpRouter*>::iterator iter = mInstance->mRouterMap.begin();
        for (; mInstance->mRouterMap.end() != iter; ++iter) {
            delete iter->second;
        }
        mInstance->mRouterMap.clear();
        delete mInstance;
        mInstance = NULL;
    }
}
//------------------------------------------------------------------------
std::string HttpServer::handleError(char major,
                                    char minor,
                                    const std::string& method,
                                    const std::string& host,
                                    unsigned short port,
                                    const std::map<std::string, std::string>& headers,
                                    const std::map<std::string, HttpField*>& body,
                                    const std::string& uri,
                                    unsigned int errorCode,
                                    const std::string& errorBuf,
                                    std::map<std::string, std::string>& responseHeaders) {
    if (mIsPrintReceive) {
        printReceive(major, minor, method, host, port, headers, body, uri);
    }
    std::string buf = getErrorResponse(errorCode, errorBuf);
    if (mIsPrintError) {
        printf_s("ERROR: %s\n", buf.c_str());
    }
    if (mErrorCallback) {
        mErrorCallback(method, host, port, headers, body, uri, errorCode, errorBuf, responseHeaders);
    }
    return buf;
}
//------------------------------------------------------------------------
std::string HttpServer::handleRouter(char major,
                                     char minor,
                                     const std::string& method,
                                     const std::string& host,
                                     unsigned short port,
                                     const std::map<std::string, std::string>& headers,
                                     const std::map<std::string, HttpField*>& body,
                                     const std::string& uri,
                                     std::map<std::string, std::string>& responseHeaders) {
    if (mIsPrintReceive) {
        printReceive(major, minor, method, host, port, headers, body, uri);
    }
    if (method.empty()) {
        unsigned int errorCode = 101;
        std::string errorBuf = "method is empty";
        std::string buf = getErrorResponse(errorCode, errorBuf);
        if (mIsPrintError) {
            printf_s("ERROR: %s\n", buf.c_str());
        }
        if (mErrorCallback) {
            mErrorCallback(method, host, port, headers, body, uri, errorCode, errorBuf, responseHeaders);
        }
        return buf;
    }
    if (uri.empty()) {
        unsigned int errorCode = 102;
        std::string errorBuf = "uri is empty";
        std::string buf = getErrorResponse(errorCode, errorBuf);
        if (mIsPrintError) {
            printf_s("ERROR: %s\n", buf.c_str());
        }
        if (mErrorCallback) {
            mErrorCallback(method, host, port, headers, body, uri, errorCode, errorBuf, responseHeaders);
        }
        return buf;
    }
    if (!mRouterMap[uri]) {
        unsigned int errorCode = 103;
        std::string errorBuf = "can not find router for '" + uri + "'";
        std::string buf = getErrorResponse(errorCode, errorBuf);
        if (mIsPrintError) {
            printf_s("ERROR: %s\n", buf.c_str());
        }
        if (mErrorCallback) {
            mErrorCallback(method, host, port, headers, body, uri, errorCode, errorBuf, responseHeaders);
        }
        return buf;
    }
    if ("GET" == method && !mRouterMap[uri]->support_get) {
        unsigned int errorCode = 104;
        std::string errorBuf = "can not support GET request for '" + uri + "'";
        std::string buf = getErrorResponse(errorCode, errorBuf);
        if (mIsPrintError) {
            printf_s("ERROR: %s\n", buf.c_str());
        }
        if (mErrorCallback) {
            mErrorCallback(method, host, port, headers, body, uri, errorCode, errorBuf, responseHeaders);
        }
        return buf;
    }
    if ("POST" == method && !mRouterMap[uri]->support_post) {
        unsigned int errorCode = 105;
        std::string errorBuf = "can not support POST request for '" + uri + "'";
        std::string buf = getErrorResponse(errorCode, errorBuf);
        if (mIsPrintError) {
            printf_s("ERROR: %s\n", buf.c_str());
        }
        if (mErrorCallback) {
            mErrorCallback(method, host, port, headers, body, uri, errorCode, errorBuf, responseHeaders);
        }
        return buf;
    }
    if (!mRouterMap[uri]->callback) {
        unsigned int errorCode = 106;
        std::string errorBuf = "can not find execute function for '" + uri + "'";
        std::string buf = getErrorResponse(errorCode, errorBuf);
        if (mIsPrintError) {
            printf_s("ERROR: %s\n", buf.c_str());
        }
        if (mErrorCallback) {
            mErrorCallback(method, host, port, headers, body, uri, errorCode, errorBuf, responseHeaders);
        }
        return buf;
    }
    return mRouterMap[uri]->callback(method, host, port, headers, body, uri, responseHeaders);
}
//------------------------------------------------------------------------
void HttpServer::setErrorCallback(HTTP_ERROR_CALLBACK errorCallback) {
    if (errorCallback && !mErrorCallback) {
        mErrorCallback = errorCallback;
    }
}
//------------------------------------------------------------------------
void HttpServer::addRouter(const std::string& uri, HttpRouter* router) {
    if (uri.empty() || !router) {
        return;
    }
    if (mRouterMap[uri]) {
        delete router;
        printf_s("exist router for \"%s\"\n", uri.c_str());
    } else {
        mRouterMap[uri] = router;
    }
}
//------------------------------------------------------------------------
void HttpServer::addRouter(const std::string& uri, HTTP_ROUTER_CALLBACK callback) {
    if (uri.empty() || !callback) {
        return;
    }
    HttpRouter* router = new HttpRouter();
    router->support_get = true;
    router->support_post = true;
    router->callback = callback;
    addRouter(uri, router);
}
//------------------------------------------------------------------------
void HttpServer::addRouterGet(const std::string& uri, HTTP_ROUTER_CALLBACK callback) {
    if (uri.empty() || !callback) {
        return;
    }
    HttpRouter* router = new HttpRouter();
    router->support_get = true;
    router->support_post = false;
    router->callback = callback;
    addRouter(uri, router);
}
//------------------------------------------------------------------------
void HttpServer::addRouterPost(const std::string& uri, HTTP_ROUTER_CALLBACK callback) {
    if (uri.empty() || !callback) {
        return;
    }
    HttpRouter* router = new HttpRouter();
    router->support_get = false;
    router->support_post = true;
    router->callback = callback;
    addRouter(uri, router);
}
//------------------------------------------------------------------------
void HttpServer::run(const std::string& ip, unsigned int port, bool printReceive /*= false*/, bool printError /*= true*/) {
    if (mIsRunning) {
        printf_s("http server is running aleady ...\n");
        return;
    }
    if (ip.empty()) {
        printf_s("start http server failed, ip is empty ...\n");
        return;
    }
    if (0 == port) {
        printf_s("start http server failed, port is 0 ...\n");
        return;
    }
    mIsRunning = true;
    mIsPrintReceive = printReceive;
    mIsPrintError = printError;
#ifdef WIN32
    WSADATA wsaData;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        mIsRunning = false;
        printf_s("WSA startup failed ...\n");
        return;
    }
#endif
    startHttpServer(ip.c_str(), port, httpServerCallback, NULL);
#ifdef WIN32
    WSACleanup();
#endif
    mIsRunning = false;
}
//------------------------------------------------------------------------
std::string HttpServer::getErrorResponse(unsigned int errorCode, const std::string& errorBuf) {
    char buf[256] = { 0 };
    sprintf_s(buf, "{\"code\":%d,\"msg\":\"%s\"}", errorCode, errorBuf.c_str());
    return buf;
}
//------------------------------------------------------------------------
void HttpServer::printReceive(char major,
                              char minor,
                              const std::string& method,
                              const std::string& host,
                              unsigned short port,
                              const std::map<std::string, std::string>& headers,
                              const std::map<std::string, HttpField*>& body,
                              const std::string& uri) {
    printf_s("--------------------------------------------------[[\n");
    printf_s("HTTP / %d.%d\n", major, minor);
    printf_s("Receive a %s request from %s:%u\n", method.c_str(), host.c_str(), port);
    time_t timep;
    time(&timep);
    char dateBuf[64] = { 0 };
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    printf_s("Date: %s\n", dateBuf);
    printf_s("Headers:\n");
    std::map<std::string, std::string>::const_iterator headerIter = headers.begin();
    for (; headers.end() != headerIter; ++headerIter) {
        printf_s("    %s: %s\n", headerIter->first.c_str(), headerIter->second.c_str());
    }
    printf_s("Body:\n");
    std::map<std::string, HttpField*>::const_iterator bodyIter = body.begin();
    for (; body.end() != bodyIter; ++bodyIter) {
        if (HttpField::TYPE_TEXT == bodyIter->second->getType()) {
            printf_s("    [TEXT] %s: %s\n", bodyIter->second->getName().c_str(), bodyIter->second->getContent());
        } else if (HttpField::TYPE_FILE == bodyIter->second->getType()) {
            printf_s("    [FILE] %s: filename => %s, file content type => %s, file size => %d\n",
                bodyIter->second->getName().c_str(), bodyIter->second->getFilename().c_str(),
                bodyIter->second->getFileContentType().c_str(), bodyIter->second->getContentLength());
        } else {
            printf_s("    [%d] can not deal\n", bodyIter->second->getType());
        }
    }
    printf_s("Uri: %s\n", uri.c_str());
    printf_s("--------------------------------------------------]]\n");
}
//------------------------------------------------------------------------
