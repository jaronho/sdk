#include <chrono>
#include <mutex>
#include <thread>

#include "nsocket/http/server.h"
#include "utility/cmdline/cmdline.h"

std::shared_ptr<nsocket::http::Server> g_server = nullptr; /* 服务器 */

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
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("HTTP服务端");
    parser.add<std::string>("server", 's', "服务器地址, 默认:", false, "127.0.0.1");
    parser.add<int>("port", 'p', "服务器端口, 默认:", false, 4444, cmdline::range(1, 65535));
#if (1 == ENABLE_NSOCKET_OPENSSL)
    parser.add<int>("cert-format", 'f', "证书文件格式, 值: 0-DER, 1-PEM, 默认:", false, 1, cmdline::oneof<int>(0, 1));
    parser.add<std::string>("cert-file", 'c', "证书文件名, 例如: server.crt, 默认:", false, "");
    parser.add<std::string>("key-file", 'k', "私钥文件名, 例如: server.key, 默认:", false, "");
    parser.add<std::string>("key-pwd", 'P', "私钥文件密码, 例如: 123456, 默认:", false, "");
    parser.add<int>("ssl-way", 'w', "SSL验证, 值: 1-单向验证, 2-双向验证, 默认:", false, 1, cmdline::oneof<int>(1, 2));
#endif
    parser.parse_check(argc, argv, "用法", "选项", "显示帮助信息并退出");
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    auto server = parser.get<std::string>("server");
    auto port = parser.get<int>("port");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    auto certFormat = parser.get<int>("cert-format");
    auto certFile = parser.get<std::string>("cert-file");
    auto privateKeyFile = parser.get<std::string>("key-file");
    auto privateKeyFilePwd = parser.get<std::string>("key-pwd");
    auto way = parser.get<int>("ssl-way");
#endif
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("服务器: %s:%d, SSL验证: %s\n", server.c_str(), port, 1 == way ? "单向验证" : "双向验证");
#else
    printf("服务器: %s:%d\n", server.c_str(), port);
#endif
    g_server = std::make_shared<nsocket::http::Server>("http_server", 10, server, port);
    if (!g_server->isValid())
    {
        printf("服务器启动失败, 请检查地址和端口是否可用\n");
        return 0;
    }
    /* 设置路由未找到回调 */
    g_server->setRouterNotFoundCallback([&](uint64_t cid, const nsocket::http::REQUEST_PTR& req) {
        printf("****************************** 路由未找到 ******************************\n");
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
            printf("========================= 简单路由(方法不允许) =========================\n");
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
            printf("========================================================================\n");
            auto str = htmlString(cid, req, "405 Method Not Allow");
            auto resp = nsocket::http::makeResponse405();
            resp->body.insert(resp->body.end(), str.begin(), str.end());
            return resp;
        };
        r->respHandler = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& data,
                             const nsocket::http::SEND_RESPONSE_FUNC& sendRespFunc) {
            printf("------------------------------- 简单路由 -------------------------------\n");
            printf("---     Cid: %zu\n", cid);
            printf("---  Client: %s:%d\n", req->host.c_str(), req->port);
            printf("--- Version: %s\n", req->version.c_str());
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
            printf("------------------------------------------------------------------------\n");
            auto str = htmlString(cid, req, "Welcome To Home");
            auto resp = nsocket::http::makeResponse200();
            resp->body.insert(resp->body.end(), str.begin(), str.end());
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
            if (1 == way || privateKeyFile.empty()) /* 单向SSL */
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
