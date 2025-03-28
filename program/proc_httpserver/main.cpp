#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <chrono>
#include <mutex>
#include <thread>

#include "fileparse/nlohmann/helper.hpp"
#include "nsocket/http/server.h"
#include "utility/cmdline/cmdline.h"
#include "utility/strtool/strtool.h"

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
    SetConsoleOutputCP(CP_UTF8);
    /* 关闭控制台程序的快速编辑模式, 否则会出现点击界面, 程序将会变成阻塞状态, 不按回车无法继续运行 */
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE; /* 移除快速编辑模式 */
    SetConsoleMode(hStdin, mode);
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("HTTP服务端");
    parser.add<std::string>("server", 's', "服务器地址, 默认:", false, "0.0.0.0");
    parser.add<int>("port", 'p', "服务器端口, 默认:", false, 4444, cmdline::range(1, 65535));
#if (1 == ENABLE_NSOCKET_OPENSSL)
    parser.add<int>("ssl-on", 't', "是否启用TLS, 值: 0-不启用, 1-启用, 默认:", false, 0, cmdline::range(0, 1));
    parser.add<int>("ssl-way", 'w', "SSL验证, 值: 1-单向验证, 2-双向验证, 默认:", false, 1, cmdline::oneof<int>(1, 2));
    parser.add<int>("cert-fmt", 'f', "证书文件格式, 值: 1-DER, 2-PEM, 默认:", false, 2, cmdline::oneof<int>(1, 2));
    parser.add<std::string>("cert-file", 'c', "证书文件名, 例如: server.crt, 默认:", false, "");
    parser.add<std::string>("pk-file", 'k', "私钥文件名, 例如: server.key, 默认:", false, "");
    parser.add<std::string>("pk-pwd", 'P', "私钥文件密码, 例如: 123456, 默认:", false, "");
#endif
    parser.add<std::string>("router", 'r', "自定义路由(JSON), 例如: [{\"method\":\"GET\",\"uri\":\"test1\"}], 默认:", false, "");
    parser.parse_check(argc, argv, "用法", "选项", "显示帮助信息并退出");
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    auto server = parser.get<std::string>("server");
    auto port = parser.get<int>("port");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    auto sslOn = parser.get<int>("ssl-on");
    auto sslWay = parser.get<int>("ssl-way");
    auto certFmt = parser.get<int>("cert-fmt");
    auto certFile = parser.get<std::string>("cert-file");
    auto pkFile = parser.get<std::string>("pk-file");
    auto pkPwd = parser.get<std::string>("pk-pwd");
#endif
    auto routerList = nlohmann::parse(parser.get<std::string>("router"));
    g_server = std::make_shared<nsocket::http::Server>("http_server", 10, server, port);
    /* 设置默认路由回调 */
    g_server->setDefaultRouterCallback([&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn) {
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
        conn.sendAndClose(resp->pack());
    });
    /* 添加路由表 */
    if (routerList.is_array())
    {
        auto r = std::make_shared<nsocket::http::Router_simple>();
        r->methodNotAllowedCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn) {
            printf("========================= 自定义路由(方法不允许) =========================\n");
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
            conn.sendAndClose(resp->pack());
        };
        r->respHandler = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& data,
                             const nsocket::http::Connector& conn) {
            printf("------------------------------- 自定义路由 -------------------------------\n");
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
            conn.sendAndClose(resp->pack());
        };
        if (!routerList.empty())
        {
            printf("自定义API接口:\n");
        }
        for (size_t i = 0; i < routerList.size(); ++i)
        {
            auto routerObj = routerList[i];
            if (routerObj.is_object())
            {
                auto method = utility::StrTool::toUpper(nlohmann::getter<std::string>(routerObj, "method"));
                auto uri = nlohmann::getter<std::string>(routerObj, "uri");
                if (method.empty() || uri.empty())
                {
                    continue;
                }
                nsocket::http::Method methodType;
                if ("CONNECT" == method)
                {
                    methodType = nsocket::http::Method::CONNECT;
                }
                else if ("DELETE" == method)
                {
                    methodType = nsocket::http::Method::DELETE;
                }
                else if ("GET" == method)
                {
                    methodType = nsocket::http::Method::GET;
                }
                else if ("HEAD" == method)
                {
                    methodType = nsocket::http::Method::HEAD;
                }
                else if ("PATCH" == method)
                {
                    methodType = nsocket::http::Method::PATCH;
                }
                else if ("POST" == method)
                {
                    methodType = nsocket::http::Method::POST;
                }
                else if ("PUT" == method)
                {
                    methodType = nsocket::http::Method::PUT;
                }
                else if ("OPTIONS" == method)
                {
                    methodType = nsocket::http::Method::OPTIONS;
                }
                else if ("TRACE" == method)
                {
                    methodType = nsocket::http::Method::TRACE;
                }
                else
                {
                    continue;
                }
                printf("  %s, %s\n", method.c_str(), uri.c_str());
                g_server->addRouter({methodType}, {uri}, r);
            }
        }
    }
    {
        auto r = std::make_shared<nsocket::http::Router_simple>();
        r->methodNotAllowedCb = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn) {
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
            conn.sendAndClose(resp->pack());
        };
        r->respHandler = [&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& data,
                             const nsocket::http::Connector& conn) {
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
            conn.sendAndClose(resp->pack());
        };
        g_server->addRouter({nsocket::http::Method::GET}, {"/", "index", "index.htm", "index.html"}, r);
    }
    /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
    try
    {
        if (1 == sslOn && !certFile.empty() && !pkFile.empty())
        {
            printf("启动服务器: %s:%d, SSL验证: %s\n", server.c_str(), port, 1 == sslWay ? "单向" : "双向");
        }
        else
        {
            printf("启动服务器: %s:%d\n", server.c_str(), port);
        }
        std::string errDesc;
        if (g_server->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd, &errDesc))
        {
            /* 主线程 */
            while (1)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        else
        {
            printf("服务器启动失败: %s\n", errDesc.c_str());
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
