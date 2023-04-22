#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#pragma warning(disable : 6031)
#else
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "../../nsocket/http/server.h"

static const std::string UPLOAD_PATH = "upload";
std::unordered_map<uint64_t, std::shared_ptr<std::fstream>> g_fileHandlerMap;

std::string htmlString(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& title)
{
    std::string str;
    str.append("<html>");
    str.append("<h1>" + title + "</h1>");
    str.append("<b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Cid:&nbsp;</b>" + std::to_string(cid));
    str.append("</br>");
    str.append("</br>");
    str.append("<b>&nbsp;&nbsp;&nbsp;Client:&nbsp;</b>" + req->host + ":" + std::to_string(req->port));
    str.append("</br>");
    str.append("</br>");
    str.append("<b>&nbsp;Version:&nbsp;</b>" + req->version);
    str.append("</br>");
    str.append("</br>");
    str.append("<b>Method:&nbsp;</b>" + req->method + "&nbsp;&nbsp;&nbsp;&nbsp;<b>Uri:&nbsp;</b>" + req->uri);
    if (!req->queries.empty())
    {
        str.append("</br>");
        str.append("</br>");
        str.append("<b>Queries:</b>");
        str.append("<ul>");
        for (auto iter = req->queries.begin(); req->queries.end() != iter; ++iter)
        {
            str.append("<li>" + iter->first + ": " + iter->second + "</li>");
        }
        str.append("</ul>");
    }
    if (!req->headers.empty())
    {
        if (req->queries.empty())
        {
            str.append("</br>");
            str.append("</br>");
        }
        str.append("<b>Headers:</b>");
        str.append("<ul>");
        for (auto iter = req->headers.begin(); req->headers.end() != iter; ++iter)
        {
            str.append("<li>" + iter->first + ": " + iter->second + "</li>");
        }
        str.append("</ul>");
    }
    str.append("</html>");
    return str;
}

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is HTTP server                                                                                   **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4444                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-tls]                 specify enable ssl [0-disable, 1-enable]. default: 0                           **\n");
    printf("** [-way]                 specify ssl way verify [1, 2], default: 1                                      **\n");
    printf("** [-pem]                 specify file format [1-DER, 2-PEM]. default: 2                                 **\n");
    printf("** [-cf]                  specify certificate file. e.g. client.crt, ca.crt                              **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    int sslOn = 0;
    int sslWay = 1;
    int certFmt = 2;
    std::string certFile;
    std::string pkFile;
    std::string pkPwd;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-s")) /* 服务器地址 */
        {
            ++i;
            if (i < argc)
            {
                serverHost = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-p")) /* 服务器端口 */
        {
            ++i;
            if (i < argc)
            {
                serverPort = atoi(argv[i]);
                ++i;
            }
        }
#if (1 == ENABLE_NSOCKET_OPENSSL)
        else if (0 == strcmp(key, "-tls")) /* 是否启用TLS */
        {
            ++i;
            if (i < argc)
            {
                sslOn = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-way")) /* SSL校验 */
        {
            ++i;
            if (i < argc)
            {
                sslWay = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pem")) /* 文件格式 */
        {
            ++i;
            if (i < argc)
            {
                certFmt = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-cf")) /* 证书文件 */
        {
            ++i;
            if (i < argc)
            {
                certFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkf")) /* 私钥文件 */
        {
            ++i;
            if (i < argc)
            {
                pkFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* 私钥文件密码 */
        {
            ++i;
            if (i < argc)
            {
                pkPwd = argv[i];
                ++i;
            }
        }
#endif
        else
        {
            ++i;
        }
    }
    if (serverHost.empty())
    {
        serverHost = "127.0.0.1";
    }
    if (serverPort <= 0 || serverPort > 65535)
    {
        serverPort = 4444;
    }
    if (sslOn < 0)
    {
        sslOn = 0;
    }
    else if (sslOn > 1)
    {
        sslOn = 1;
    }
    if (sslWay < 1)
    {
        sslWay = 1;
    }
    else if (sslWay > 2)
    {
        sslWay = 2;
    }
    if (certFmt < 1)
    {
        certFmt = 1;
    }
    else if (certFmt > 2)
    {
        certFmt = 2;
    }
    nsocket::http::Server server("http_server", 10, serverHost, serverPort);
    server.setRouterNotFoundCallback([&](uint64_t cid, const nsocket::http::REQUEST_PTR& req) {
        printf("************************* Not Found URI Router *************************\n");
        printf("***     Cid: %zu\n", cid);
        printf("***  Client: %s:%d\n", req->host.c_str(), req->port);
        printf("*** Version: %s\n", req->version.c_str());
        printf("***  Method: %s\n", req->method.c_str());
        printf("***     Uri: %s\n", req->uri.c_str());
        if (!req->queries.empty())
        {
            printf("*** Queries:\n");
            for (auto iter = req->queries.begin(); req->queries.end() != iter; ++iter)
            {
                printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
            }
        }
        if (!req->headers.empty())
        {
            printf("*** Headers:\n");
            for (auto iter = req->headers.begin(); req->headers.end() != iter; ++iter)
            {
                printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
            }
        }
        printf("************************************************************************\n");
        auto str = htmlString(cid, req, "404 Not Found");
        auto resp = nsocket::http::makeResponse404();
        resp->body.insert(resp->body.end(), str.begin(), str.end());
        return resp;
    });
    /* 添加路由表 */
    {
        auto r = std::make_shared<nsocket::http::Router_simple>();
        r->methodNotAllowedCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req) {
            printf("-------------------------- Simple Router --------------------------\n");
            printf("Method: %s not allowed for Uri: %s\n", req->method.c_str(), req->uri.c_str());
            printf("Support methods:");
            for (auto method : r->getAllowMethods())
            {
                printf(" %s", nsocket::http::method_desc(method).c_str());
            }
            printf("\n");
            printf("--------------------------------------------------------------------\n");
            auto resp = nsocket::http::makeResponse405();
            return resp;
        };
        r->respHandler = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& data,
                             const nsocket::http::SEND_RESPONSE_FUNC& sendRespFunc) {
            printf("-------------------------- Simple Router --------------------------\n");
            printf("---  Client: %s:%d\n", req->host.c_str(), req->port);
            printf("---  Method: %s\n", req->method.c_str());
            printf("---     Uri: %s\n", req->uri.c_str());
            if (!req->queries.empty())
            {
                printf("--- Queries:\n");
                for (auto iter = req->queries.begin(); req->queries.end() != iter; ++iter)
                {
                    printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
                }
            }
            printf("--- Version: %s\n", req->version.c_str());
            if (!req->headers.empty())
            {
                printf("--- Headers:\n");
                for (auto iter = req->headers.begin(); req->headers.end() != iter; ++iter)
                {
                    printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
                }
            }
            printf("--- Content(%zu):\n", req->getContentLength());
            printf("%s\n", data.c_str());
            printf("-------------------------------------------------------------------\n");
            auto str = htmlString(cid, req, "Welcome To Home");
            auto resp = nsocket::http::makeResponse200();
            resp->body.insert(resp->body.end(), str.begin(), str.end());
            if (sendRespFunc)
            {
                sendRespFunc(resp);
            }
        };
        server.addRouter({nsocket::http::Method::GET}, {"/", "index", "index.htm", "index.html"}, r);
    }
    {
        auto r = std::make_shared<nsocket::http::Router_simple>();
        r->methodNotAllowedCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req) {
            printf("-------------------------- Simple Router --------------------------\n");
            printf("Method: %s not allowed for Uri: %s\n", req->method.c_str(), req->uri.c_str());
            printf("Support methods:");
            for (auto method : r->getAllowMethods())
            {
                printf(" %s", nsocket::http::method_desc(method).c_str());
            }
            printf("\n");
            printf("--------------------------------------------------------------------\n");
            auto resp = nsocket::http::makeResponse405();
            return resp;
        };
        r->respHandler = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& data,
                             const nsocket::http::SEND_RESPONSE_FUNC& sendRespFunc) {
            printf("-------------------------- Simple Router --------------------------\n");
            printf("---  Client: %s:%d\n", req->host.c_str(), req->port);
            printf("---  Method: %s\n", req->method.c_str());
            printf("---     Uri: %s\n", req->uri.c_str());
            if (!req->queries.empty())
            {
                printf("--- Queries:\n");
                for (auto iter = req->queries.begin(); req->queries.end() != iter; ++iter)
                {
                    printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
                }
            }
            printf("--- Version: %s\n", req->version.c_str());
            if (!req->headers.empty())
            {
                printf("--- Headers:\n");
                for (auto iter = req->headers.begin(); req->headers.end() != iter; ++iter)
                {
                    printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
                }
            }
            printf("--- Content(%zu):\n", req->getContentLength());
            printf("%s\n", data.c_str());
            printf("-------------------------------------------------------------------\n");
            std::string result = "{\"code\":0,\"msg\":\"ok\",\"data\":{\"cid\":\"" + std::to_string(cid) + "\",\"host\":\"" + req->host
                                 + "\",\"port\":" + std::to_string(req->port) + "}}";
            auto resp = nsocket::http::makeResponse200();
            resp->body.insert(resp->body.end(), result.begin(), result.end());
            if (sendRespFunc)
            {
                sendRespFunc(resp);
            }
        };
        server.addRouter({nsocket::http::Method::GET, nsocket::http::Method::POST}, {"/simple"}, r);
    }
    {
        auto r = std::make_shared<nsocket::http::Router_x_www_form_urlencoded>();
        r->methodNotAllowedCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req) {
            printf("-------------------------- Form Router --------------------------\n");
            printf("Method: %s not allowed for Uri: %s\n", req->method.c_str(), req->uri.c_str());
            printf("Support methods:");
            for (auto method : r->getAllowMethods())
            {
                printf(" %s", nsocket::http::method_desc(method).c_str());
            }
            printf("\n");
            printf("--------------------------------------------------------------------\n");
            auto resp = nsocket::http::makeResponse405();
            return resp;
        };
        r->respHandler = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::CaseInsensitiveMultimap& fields,
                             const nsocket::http::SEND_RESPONSE_FUNC& sendRespFunc) {
            printf("--------------------------- Form Router ---------------------------\n");
            printf("---  Client: %s:%d\n", req->host.c_str(), req->port);
            printf("---  Method: %s\n", req->method.c_str());
            printf("---     Uri: %s\n", req->uri.c_str());
            if (!req->queries.empty())
            {
                printf("--- Queries:\n");
                for (auto iter = req->queries.begin(); req->queries.end() != iter; ++iter)
                {
                    printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
                }
            }
            printf("--- Version: %s\n", req->version.c_str());
            if (!req->headers.empty())
            {
                printf("--- Headers:\n");
                for (auto iter = req->headers.begin(); req->headers.end() != iter; ++iter)
                {
                    printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
                }
            }
            if (!fields.empty())
            {
                printf("---  Fields:\n");
                for (auto iter = fields.begin(); fields.end() != iter; ++iter)
                {
                    printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
                }
            }
            printf("-------------------------------------------------------------------\n");
            std::string result = "{\"code\":0,\"msg\":\"ok\",\"data\":{\"cid\":\"" + std::to_string(cid) + "\",\"host\":\"" + req->host
                                 + "\",\"port\":" + std::to_string(req->port) + "}}";
            auto resp = nsocket::http::makeResponse200();
            resp->body.insert(resp->body.end(), result.begin(), result.end());
            if (sendRespFunc)
            {
                sendRespFunc(resp);
            }
        };
        server.addRouter({nsocket::http::Method::POST}, {"/form"}, r);
    }
    {
        /* 创建文件路径 */
#ifdef _WIN32
        if (0 != _access(UPLOAD_PATH.c_str(), 0))
        {
            _mkdir(UPLOAD_PATH.c_str());
        }
#else
        if (0 != access(UPLOAD_PATH.c_str(), F_OK))
        {
            mkdir(UPLOAD_PATH.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
        }
#endif
        auto r = std::make_shared<nsocket::http::Router_multipart_form_data>();
        r->methodNotAllowedCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req) {
            printf("-------------------------- Multi Router --------------------------\n");
            printf("Method: %s not allowed for Uri: %s\n", req->method.c_str(), req->uri.c_str());
            printf("Support methods:");
            for (auto method : r->getAllowMethods())
            {
                printf(" %s", nsocket::http::method_desc(method).c_str());
            }
            printf("\n");
            printf("--------------------------------------------------------------------\n");
            auto resp = nsocket::http::makeResponse405();
            return resp;
        };
        r->headCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req) {
            printf("--------------------------- Multi Router ---------------------------\n");
            printf("---     Cid: %lld, Client: %s:%d\n", cid, req->host.c_str(), req->port);
            printf("---  Method: %s\n", req->method.c_str());
            printf("---     Uri: %s\n", req->uri.c_str());
            if (!req->queries.empty())
            {
                printf("--- Queries:\n");
                for (auto iter = req->queries.begin(); req->queries.end() != iter; ++iter)
                {
                    printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
                }
            }
            printf("--- Version: %s\n", req->version.c_str());
            if (!req->headers.empty())
            {
                printf("--- Headers:\n");
                for (auto iter = req->headers.begin(); req->headers.end() != iter; ++iter)
                {
                    printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
                }
            }
            printf("--------------------------------------------------------------------\n");
        };
        r->textCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& name, const std::string& contentType,
                        const std::string& text) {
            printf("--------------------------- Multi Router ---------------------------\n");
            printf("--- cid: %lld, name: %s, content type: %s, text: %s\n", cid, name.c_str(), contentType.c_str(), text.c_str());
            printf("--------------------------------------------------------------------\n");
        };
        r->fileCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& name, const std::string& filename,
                        const std::string& contentType, size_t offset, const unsigned char* data, int dataLen, bool finish) {
            printf("--------------------------- Multi Router ---------------------------\n");
            printf("--- cid: %lld, name: %s, filename: %s, content type: %s, offset: %zu, data len: %d, finish: %s\n", cid, name.c_str(),
                   filename.c_str(), contentType.c_str(), offset, dataLen, finish ? "true" : "false");
            printf("--------------------------------------------------------------------\n");
            /* 创建和查找文件句柄 */
            std::shared_ptr<std::fstream> fs = nullptr;
            auto iter = g_fileHandlerMap.find(cid);
            if (g_fileHandlerMap.end() == iter)
            {
                std::string fullFilename = UPLOAD_PATH + "/" + filename;
                fs = std::make_shared<std::fstream>(fullFilename, std::ios::out | std::ios::binary);
                if (!fs->is_open())
                {
                    printf("*** file: %s open fail, %d, %s\n", fullFilename.c_str(), errno, strerror(errno));
                }
                iter = g_fileHandlerMap.insert(std::make_pair(cid, fs)).first;
            }
            else
            {
                fs = iter->second;
            }
            /* 写文件数据 */
            if (fs)
            {
                fs->seekp(offset, std::ios::beg);
                fs->write((const char*)data, dataLen);
                /* 文件写完毕 */
                if (finish)
                {
                    fs->flush();
                    fs->close();
                    g_fileHandlerMap.erase(iter);
                }
            }
        };
        r->respHandler = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::SEND_RESPONSE_FUNC& sendRespFunc) {
            /* 为了安全性起见, 在请求结束后, 再一次查找是否有未关闭的文件句柄, 有的话则关闭 */
            auto iter = g_fileHandlerMap.find(cid);
            if (g_fileHandlerMap.end() != iter)
            {
                iter->second->flush();
                iter->second->close();
                g_fileHandlerMap.erase(iter);
            }
            std::string result = "{\"code\":0,\"msg\":\"ok\",\"data\":{\"cid\":\"" + std::to_string(cid) + "\",\"host\":\"" + req->host
                                 + "\",\"port\":" + std::to_string(req->port) + "}}";
            auto resp = nsocket::http::makeResponse200();
            resp->body.insert(resp->body.end(), result.begin(), result.end());
            if (sendRespFunc)
            {
                sendRespFunc(resp);
            }
        };
        server.addRouter({nsocket::http::Method::POST}, {"/multi"}, r);
    }
    try
    {
        if (1 == sslOn && !certFile.empty() && !pkFile.empty())
        {
            printf("server: %s:%d, ssl way: %d, certFile: %s, pkFile: %s\n", serverHost.c_str(), serverPort, sslWay, certFile.c_str(),
                   pkFile.c_str());
        }
        else
        {
            printf("server: %s:%d\n", serverHost.c_str(), serverPort);
        }
        std::string errDesc;
        if (server.run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd, &errDesc))
        {
            while (1)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        else
        {
            printf("server run fail: %s\n", errDesc.c_str());
        }
    }
    catch (const std::exception& e)
    {
        printf("========== execption: %s\n", e.what());
    }
    catch (...)
    {
        printf("========== execption: unknown\n");
    }
    return 0;
}
