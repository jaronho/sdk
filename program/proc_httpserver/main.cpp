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

#include "nsocket/http/server.h"
#include "utility/cmdline/cmdline.h"

std::shared_ptr<nsocket::http::Server> g_server = nullptr; /* 服务器 */

int main(int argc, char* argv[])
{
    /* 命令参数 */
    cmdline::parser parser;
    parser.set_program_name("HTTP服务端");
    parser.add<std::string>("address", 'a', "服务器地址", false, "127.0.0.1");
    parser.add<int>("port", 'p', "服务器端口", false, 4444);
#if (1 == ENABLE_NSOCKET_OPENSSL)
    parser.add<int>("cert_format", 'f', "证书文件格式, 值: 0-DER, 1-PEM, 默认1", false, 1);
    parser.add<std::string>("cert_file", 'c', "证书文件名, 例如: server.crt", false, "");
    parser.add<std::string>("key_file", 'k', "私钥文件名, 例如: server.key", false, "");
    parser.add<std::string>("key_pwd", 's', "私钥文件密码, 例如: 123456", false, "");
    parser.add<int>("ssl_way", 'w', "SSL验证, 值: 1-单向验证, 2-双向验证, 默认1", false, 1);
#endif
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    auto address = parser.get<std::string>("address");
    auto port = parser.get<int>("port");
    port = (port > 0 && port < 65536) ? port : 4444;
#if (1 == ENABLE_NSOCKET_OPENSSL)
    auto certFormat = parser.get<int>("cert_format");
    certFormat = certFormat < 0 ? 0 : (certFormat > 1 ? 1 : certFormat);
    auto certFile = parser.get<std::string>("cert_file");
    auto privateKeyFile = parser.get<std::string>("key_file");
    auto privateKeyFilePwd = parser.get<std::string>("key_pwd");
    auto way = parser.get<int>("ssl_way");
    way = way < 1 ? 1 : (way > 2 ? 2 : way);
#endif
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("服务器: %s:%d, SSL验证: %s\n", address.c_str(), port, 1 == way ? "单向验证" : "双向验证");
#else
    printf("服务器: %s:%d\n", address.c_str(), port);
#endif
    g_server = std::make_shared<nsocket::http::Server>("http_server", 10, address, port);
    if (!g_server->isValid())
    {
        printf("服务器启动失败, 请检查地址和端口是否可用\n");
        return 0;
    }
    /* 设置路由未找到回调 */
    g_server->setRouterNotFoundCallback([&](uint64_t cid, const nsocket::http::REQUEST_PTR& req) {
        printf("************************* 路由未找到 *************************\n");
        printf("---  Client: %s:%d\n", req->host.c_str(), req->port);
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
        std::string result;
        result.append("<html>");
        result.append("<h1>404 Not Found</h1>");
        result.append("<b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Cid:&nbsp;</b>" + std::to_string(cid));
        result.append("</br>");
        result.append("</br>");
        result.append("<b>&nbsp;&nbsp;&nbsp;Client:&nbsp;</b>" + req->host + ":" + std::to_string(req->port));
        result.append("</br>");
        result.append("</br>");
        result.append("<b>Method:&nbsp;</b>" + req->method);
        result.append("</br>");
        result.append("</br>");
        result.append("<b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Uri:&nbsp;</b>" + req->uri);
        result.append("</br>");
        result.append("</br>");
        result.append("<b>&nbsp;Version:&nbsp;</b>" + req->version);
        result.append("</html>");
        auto resp = std::make_shared<nsocket::http::Response>();
        resp->statusCode = nsocket::http::StatusCode::client_error_not_found;
        resp->body.insert(resp->body.end(), result.begin(), result.end());
        return resp;
    });
    /* 添加路由表 */
    {
        auto r = std::make_shared<nsocket::http::Router_simple>();
        r->methodNotAllowedCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req) {
            printf("************************* 简单路由(方法不允许) *************************\n");
            printf("---  Client: %s:%d\n", req->host.c_str(), req->port);
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
            std::string result;
            result.append("<html>");
            result.append("<h1>405 Method Not Allow</h1>");
            result.append("<b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Cid:&nbsp;</b>" + std::to_string(cid));
            result.append("</br>");
            result.append("</br>");
            result.append("<b>&nbsp;&nbsp;&nbsp;Client:&nbsp;</b>" + req->host + ":" + std::to_string(req->port));
            result.append("</br>");
            result.append("</br>");
            result.append("<b>Method:&nbsp;</b>" + req->method);
            result.append("</br>");
            result.append("</br>");
            result.append("<b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Uri:&nbsp;</b>" + req->uri);
            result.append("</br>");
            result.append("</br>");
            result.append("<b>&nbsp;Version:&nbsp;</b>" + req->version);
            result.append("</html>");
            auto resp = std::make_shared<nsocket::http::Response>();
            resp->statusCode = nsocket::http::StatusCode::client_error_method_not_allowed;
            resp->body.insert(resp->body.end(), result.begin(), result.end());
            return resp;
        };
        r->respHandler = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& data,
                             const nsocket::http::SEND_RESPONSE_FUNC& sendRespFunc) {
            printf("-------------------------- 简单路由 --------------------------\n");
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
            std::string result;
            result.append("<html>");
            result.append("<h1>Home</h1>");
            result.append("<b>&nbsp;&nbsp;&nbsp;&nbsp;Cid:&nbsp;</b>" + std::to_string(cid));
            result.append("</br>");
            result.append("</br>");
            result.append("<b>Client:&nbsp;</b>" + req->host + ":" + std::to_string(req->port));
            result.append("</html>");
            auto resp = std::make_shared<nsocket::http::Response>();
            resp->body.insert(resp->body.end(), result.begin(), result.end());
            if (sendRespFunc)
            {
                sendRespFunc(resp);
            }
        };
        g_server->addRouter({nsocket::http::Method::GET}, {"/", "index", "index.htm", "index.html"}, r);
    }
    /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
    try
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        if (certFile.empty())
        {
            g_server->run();
        }
        else
        {
            std::shared_ptr<boost::asio::ssl::context> sslContext;
            if (1 == way) /* 单向SSL */
            {
                sslContext = nsocket::TcpServer::getSsl1WayContext(certFormat ? boost::asio::ssl::context::file_format::pem
                                                                              : boost::asio::ssl::context::file_format::asn1,
                                                                   certFile, privateKeyFile, privateKeyFilePwd, true);
            }
            else /* 双向SSL */
            {
                sslContext = nsocket::TcpServer::getSsl2WayContext(certFormat ? boost::asio::ssl::context::file_format::pem
                                                                              : boost::asio::ssl::context::file_format::asn1,
                                                                   certFile, privateKeyFile, privateKeyFilePwd, true);
            }
            g_server->run(sslContext);
        }
#else
        g_server->run();
#endif
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    catch (const std::exception& e)
    {
        printf("========== 异常: %s\n", e.what());
    }
    catch (...)
    {
        printf("========== 异常: 未知\n");
    }
    return 0;
}
