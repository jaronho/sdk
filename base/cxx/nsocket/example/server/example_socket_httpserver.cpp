#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
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
std::unordered_map<int64_t, std::shared_ptr<std::fstream>> g_fileHandlerMap;

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is HTTP server                                                                                   **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4335                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-cf]                  specify certificate file. e.g. server.crt                                      **\n");
    printf("** [-pkf]                 specify private key file, e.g. server.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    std::string certFile;
    std::string privateKeyFile;
    std::string privateKeyFilePwd;
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
                privateKeyFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* 私钥文件密码 */
        {
            ++i;
            if (i < argc)
            {
                privateKeyFilePwd = argv[i];
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
    if (serverPort <= 0)
    {
        serverPort = 4335;
    }
    printf("server: %s:%d\n", serverHost.c_str(), serverPort);
    nsocket::http::Server server("http_server", 10, serverHost, serverPort);
    if (!server.isValid())
    {
        printf("server invalid, please check host or port\n");
        return 0;
    }
    server.setRouterNotFoundCallback([&](const nsocket::http::REQUEST_PTR& req) {
        printf("************************* Not Found URI Router *************************\n");
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
        printf("*** Version: %s\n", req->version.c_str());
        if (!req->headers.empty())
        {
            printf("*** Headers:\n");
            for (auto iter = req->headers.begin(); req->headers.end() != iter; ++iter)
            {
                printf("             %s: %s\n", iter->first.c_str(), iter->second.c_str());
            }
        }
        printf("************************************************************************\n");
    });
    /* 添加路由表 */
    {
        auto r = std::make_shared<nsocket::http::Router_simple>();
        r->respHandler = [&](const nsocket::http::REQUEST_PTR& req, const std::string& data) {
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
            printf("--- Content(%u):\n", req->getContentLength());
            printf("%s\n", data.c_str());
            printf("-------------------------------------------------------------------\n");
            return nullptr;
        };
        server.addRouter("/simple", r);
    }
    {
        auto r = std::make_shared<nsocket::http::Router_x_www_form_urlencoded>();
        r->respHandler = [&](const nsocket::http::REQUEST_PTR& req, const nsocket::CaseInsensitiveMultimap& fields) {
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
            return nullptr;
        };
        server.addRouter("/form", r);
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
        r->headCb = [&](int64_t cid, const nsocket::http::REQUEST_PTR& req) {
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
        r->textCb = [&](int64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& name, const std::string& contentType,
                        const std::string& text) {
            printf("--------------------------- Multi Router ---------------------------\n");
            printf("--- cid: %lld, name: %s, content type: %s, text: %s\n", cid, name.c_str(), contentType.c_str(), text.c_str());
            printf("--------------------------------------------------------------------\n");
        };
        r->fileCb = [&](int64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& name, const std::string& filename,
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
        r->respHandler = [&](int64_t cid, const nsocket::http::REQUEST_PTR& req) {
            /* 为了安全性起见, 在请求结束后, 再一次查找是否有未关闭的文件句柄, 有的话则关闭 */
            auto iter = g_fileHandlerMap.find(cid);
            if (g_fileHandlerMap.end() != iter)
            {
                iter->second->flush();
                iter->second->close();
                g_fileHandlerMap.erase(iter);
            }
            return nullptr;
        };
        server.addRouter("/multi", r);
    }
    try
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        auto sslContext = nsocket::TcpServer::getSsl2WayContext(certFile, privateKeyFile, privateKeyFilePwd, true);
        server.run(sslContext);
#else
        server.run();
#endif
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
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
