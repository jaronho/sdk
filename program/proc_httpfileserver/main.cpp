#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>

#include "fileparse/nlohmann/helper.hpp"
#include "nsocket/http/server.h"
#include "utility/charset/charset.h"
#include "utility/cmdline/cmdline.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"
#include "utility/util/util.h"

/**
 * @brief 项信息
 */
struct ItemInfo
{
    std::string name; /* 名称 */
    utility::FileAttribute attr; /* 属性 */
};

utility::PathInfo g_rootDir; /* 根目录 */
std::shared_ptr<nsocket::http::Server> g_server = nullptr; /* 服务器 */

/**
 * @brief 默认HTML
 */
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

/**
 * @brief 文件大小单位
 */
std::string fileSizeUnit(size_t fileSize)
{
    char buf[64] = {0};
    if (fileSize >= 1024 * 1024 * 1024) /* 大于1G */
    {
        sprintf(buf, "<font color=\"#FF0000\">%.2f G</font>", fileSize / 1024.f / 1024.f / 1024.f);
    }
    else if (fileSize >= 512 * 1024 * 1024) /* 大于512M */
    {
        sprintf(buf, "<font color=\"#E0670B\">%.1f M</font>", fileSize / 1024.f / 1024.f);
    }
    else if (fileSize >= 100 * 1024 * 1024) /* 大于100M */
    {
        sprintf(buf, "<font color=\"#0000FF\">%.1f M</font>", fileSize / 1024.f / 1024.f);
    }
    else if (fileSize >= 1024 * 1024) /* 大于1M */
    {
        sprintf(buf, "<font color=\"#30BF50\">%.1f M</font>", fileSize / 1024.f / 1024.f);
    }
    else if (fileSize >= 1024) /* 大于1K */
    {
        sprintf(buf, "%.1f K", fileSize / 1024.f);
    }
    else /* 小于1K */
    {
        sprintf(buf, "%zu B", fileSize);
    }
    return buf;
}

/**
 * @brief 处理目录
 */
void handleDir(const nsocket::http::Connector& conn, const std::string& rootDir, const std::string& uri)
{
    /* 页面头部 */
    std::string str;
    str.append("<html>");
    str.append("\n");
    str.append("<head><title>Index of " + uri + "</title></head>");
    str.append("\n");
    str.append("<body>");
    str.append("\n");
    str.append("<h1>Index of " + uri + "</h1>");
    str.append("\n");
    str.append("<hr>");
    str.append("\n");
    str.append("<pre>");
    str.append("\n");
    /* 返回上一级 */
    bool newLine = false;
    if ("/" != uri) /* 当前在子目录 */
    {
        str.append("<a href=\"../\">../</a>");
        newLine = true;
    }
    /* 内容搜索 */
    std::vector<ItemInfo> itemList;
    utility::PathInfo pi(rootDir + "/" + uri);
    pi.traverse(
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            ItemInfo info;
            info.name = name;
            info.attr = attr;
            itemList.emplace_back(info);
            return false;
        },
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            ItemInfo info;
            info.name = name;
            info.attr = attr;
            itemList.emplace_back(info);
        },
        nullptr, false, false);
    /* 内容排序 */
    std::sort(itemList.begin(), itemList.end(), [&](const ItemInfo& a, const ItemInfo& b) {
        if (a.attr.isDir && b.attr.isFile)
        {
            return true;
        }
        else if (a.attr.isFile && b.attr.isDir)
        {
            return false;
        }
        return a.name < b.name;
    });
    /* 内容渲染 */
    for (const auto& info : itemList)
    {
        auto target = utility::FileInfo(info.name).filename();
        if (newLine)
        {
            str.append("\n");
        }
        newLine = true;
        static const int MAX_TARGET_NAME_LENGTH = 80;
        auto length = target.size();
        /* 名字太长换行 */
        std::string tmpTarget = target;
        std::string lines;
        while (tmpTarget.size() >= MAX_TARGET_NAME_LENGTH - 1)
        {
            length = MAX_TARGET_NAME_LENGTH - 1;
            lines += lines.empty() ? "" : "\n";
            lines += tmpTarget.substr(0, MAX_TARGET_NAME_LENGTH - 1);
            tmpTarget = tmpTarget.substr(MAX_TARGET_NAME_LENGTH - 1);
        }
        if (!lines.empty() && !tmpTarget.empty())
        {
            length = tmpTarget.size();
            lines += "\n";
            lines += tmpTarget;
        }
        /* 渲染 */
        int spaceCount = 0;
        if (info.attr.isDir)
        {
            str.append("<a href=\"" + target + "/\">" + (lines.empty() ? target : lines) + "/</a>");
            spaceCount = MAX_TARGET_NAME_LENGTH - length - 1;
        }
        else
        {
            str.append("<a href=\"" + target + "\">" + (lines.empty() ? target : lines) + +"</a>");
            spaceCount = MAX_TARGET_NAME_LENGTH - length;
        }
        spaceCount = spaceCount > 0 ? spaceCount : 1;
        str.append(std::string(spaceCount, ' ')).append(info.attr.modifyTimeFmt());
        str.append(std::string(6, ' ')).append(info.attr.isDir ? "-" : fileSizeUnit(info.attr.size));
    }
    /* 页面尾部 */
    str.append("\n");
    str.append("</pre>");
    str.append("\n");
    str.append("<hr>");
    str.append("\n");
    str.append("</body>");
    str.append("\n");
    str.append("</html>");
    /* 发送响应 */
    auto resp = nsocket::http::makeResponse200();
#ifdef _WIN32
    resp->headers.insert(std::make_pair("Content-Type", "text/html; charset=gb2312"));
#else
    resp->headers.insert(std::make_pair("Content-Type", "text/html; charset=utf-8"));
#endif
    resp->body.insert(resp->body.end(), str.begin(), str.end());
    conn.send(resp->pack());
}

/**
 * @brief 处理文件
 */
void handleFile(const nsocket::http::Connector& conn, const std::string& fileName, size_t fileSize)
{
    utility::FileInfo fi(fileName);
    auto mimeType = nsocket::http::getFileMimeType(fileName);
    std::string textFileData;
    if (fi.isTextFile()) /* 检测文本文件编码 */
    {
        textFileData = fi.readAll();
        switch (utility::Charset::getCoding(textFileData))
        {
        case utility::Charset::Coding::gbk:
            mimeType += "; charset=gb2312";
            break;
        case utility::Charset::Coding::utf8:
            mimeType += "; charset=utf-8";
            break;
        }
    }
    /* 发送头部 */
    auto resp = nsocket::http::makeResponse200();
    resp->headers.insert(std::make_pair("Content-Type", mimeType));
    resp->headers.insert(std::make_pair("Content-Length", std::to_string(fileSize)));
    conn.send(resp->pack());
    /* 发送文件 */
    if (fileSize <= 0)
    {
        return;
    }
    if (textFileData.empty())
    {
        size_t offset = 0, count = 0;
        while (1)
        {
            offset += count;
            count = 1024 * 1024;
            auto data = fi.read(offset, count);
            if (!data)
            {
                break;
            }
            conn.send(std::vector<unsigned char>(data, data + count));
            free(data);
        }
    }
    else
    {
        conn.send(std::vector<unsigned char>(textFileData.c_str(), textFileData.c_str() + fileSize));
    }
}

int main(int argc, char* argv[])
{
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("HTTP文件服务器");
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
    parser.add<std::string>("dir", 'd', "资源文件路径, 例如: /home/data/files, 默认:", false, "");
    parser.add<int>("thread-num", 'n', "并发线程数量, 默认:", false, 10, cmdline::range(1, 1024));
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
    auto fileDir = parser.get<std::string>("dir");
    auto threadNum = parser.get<int>("thread-num");
    fileDir = fileDir.empty() ? utility::PathInfo::getcwd() : fileDir;
    g_rootDir = utility::PathInfo(fileDir);
    if (!g_rootDir.exist())
    {
        printf("资源文件路径: %s 不存在\n", g_rootDir.path().c_str());
        return 0;
    }
    else if (!g_rootDir.isAbsolute())
    {
        printf("资源文件路径: %s 不能为相对路径\n", g_rootDir.path().c_str());
        return 0;
    }
    printf("资源文件路径: %s\n", g_rootDir.path().c_str());
    g_server = std::make_shared<nsocket::http::Server>("http_server", threadNum, server, port);
    /* 设置默认路由回调 */
    g_server->setDefaultRouterCallback([&](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn) {
        bool keeyAlive = false;
        printf("************************* [%s] *************************\n", utility::DateTime::getNow().yyyyMMddhhmmss().c_str());
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
                if (utility::StrTool::equal("Connection", iter->first, false) && utility::StrTool::equal("keep-alive", iter->second, false))
                {
                    keeyAlive = true;
                }
            }
        }
        printf("***********************************************************************\n");
        if ("GET" != req->method) /* 方法不允许 */
        {
            printf("方法 %s 不被允许\n", req->method.c_str());
            auto str = htmlString(cid, req, "405 Method Not Allow");
            auto resp = nsocket::http::makeResponse405();
            resp->body.insert(resp->body.end(), str.begin(), str.end());
            conn.sendAndClose(resp->pack());
            return;
        }
        auto uri = req->uri;
        uri = utility::Util::urlDecode(uri);
#ifdef _WIN32
        uri = utility::Charset::utf8ToGbk(uri);
#endif
        auto target = g_rootDir.path().append(uri);
        utility::FileAttribute attr;
        if (!utility::getFileAttribute(target, attr))
        {
            printf("资源: %s 不存在\n", target.c_str());
            auto str = htmlString(cid, req, "404 Not Found: " + uri);
            auto resp = nsocket::http::makeResponse404();
            resp->body.insert(resp->body.end(), str.begin(), str.end());
            conn.sendAndClose(resp->pack());
            return;
        }
        printf("资源: %s\n", target.c_str());
        if (attr.isDir) /* 目录 */
        {
            printf("目录, 创建时间: %s, 修改时间: %s, 访问时间: %s\n", attr.createTimeFmt().c_str(), attr.modifyTimeFmt().c_str(),
                   attr.accessTimeFmt().c_str());
            handleDir(conn, g_rootDir.path(), uri);
        }
        else if (attr.isFile) /* 文件 */
        {
            printf("文件, 创建时间: %s, 修改时间: %s, 访问时间: %s, 大小: %zu (字节)\n", attr.createTimeFmt().c_str(),
                   attr.modifyTimeFmt().c_str(), attr.accessTimeFmt().c_str(), attr.size);
            handleFile(conn, target, attr.size);
        }
        if (!keeyAlive) /* 非保活才关闭连接 */
        {
            conn.close();
        }
    });
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
