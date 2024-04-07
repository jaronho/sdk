#include "http_file_server.h"

#include <algorithm>

#include "utility/charset/charset.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/process/process.h"

namespace hfs
{
/**
 * @brief 项信息
 */
struct ItemInfo
{
    std::string name; /* 名称 */
    utility::FileAttribute attr; /* 属性 */
};

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

HttpFileServer::HttpFileServer(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, std::string rootDir,
                               size_t fileBlockSize, bool reuseAddr, size_t bz, const std::chrono::steady_clock::duration& handshakeTimeout)
    : m_name(name)
    , m_threadCount(threadCount)
    , m_host(host)
    , m_port(port)
    , m_reuseAddr(reuseAddr)
    , m_bufferSize(bz)
    , m_handshakeTimeout(handshakeTimeout)
{
    rootDir = rootDir.empty() ? utility::FileInfo(utility::Process::getProcessExeFile()).path() : rootDir;
    utility::PathInfo pi(rootDir);
    pi.create();
    m_rootDir = pi.path();
    static const size_t MIN_FILE_BLOCK_SIZE = (4 * 1024);
    static const size_t MAX_FILE_BLOCK_SIZE = (16 * 1024 * 1024);
    if (fileBlockSize < MIN_FILE_BLOCK_SIZE || fileBlockSize > MAX_FILE_BLOCK_SIZE)
    {
        fileBlockSize = MIN_FILE_BLOCK_SIZE;
    }
    m_fileBlockSize = fileBlockSize;
}

HttpFileServer::~HttpFileServer()
{
    stop();
}

std::string HttpFileServer::getRootDir() const
{
    return m_rootDir;
}

void HttpFileServer::setNotAllowHandler(const NotHandler& handler)
{
    std::lock_guard<std::mutex> locker(m_mutexNotAllowHandler);
    m_notAllowHandler = handler;
}

void HttpFileServer::setNotFoundHandler(const NotHandler& handler)
{
    std::lock_guard<std::mutex> locker(m_mutexNotFoundHandler);
    m_notFoundHandler = handler;
}

void HttpFileServer::setDirAccessHandler(const DirAccessHandler& handler)
{
    std::lock_guard<std::mutex> locker(m_mutexDirAccessHandler);
    m_dirAccessHandler = handler;
}

void HttpFileServer::setFileGetHandler(const FileGetHandler& handler)
{
    std::lock_guard<std::mutex> locker(m_mutexFileGetHandler);
    m_fileGetHandler = handler;
}

std::vector<std::string> HttpFileServer::addRouter(const std::vector<nsocket::http::Method>& methods,
                                                   const std::vector<std::string>& uriList,
                                                   const std::shared_ptr<nsocket::http::Router>& router)
{
    std::shared_ptr<nsocket::http::Server> httpServer = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexHttpServer);
        httpServer = m_httpServer;
    }
    if (httpServer)
    {
        return httpServer->addRouter(methods, uriList, router);
    }
    return {};
}

bool HttpFileServer::run(bool sslOn, int sslWay, int certFmt, const std::string& certFile, const std::string& pkFile,
                         const std::string& pkPwd, std::string* errDesc)
{
    auto httpServer =
        std::make_shared<nsocket::http::Server>(m_name, m_threadCount, m_host, m_port, m_reuseAddr, m_bufferSize, m_handshakeTimeout);
    const std::weak_ptr<HttpFileServer> wpSelf = shared_from_this();
    httpServer->setDefaultRouterCallback(
        [&, wpSelf](uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn) {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->handleDefaultRouter(cid, req, conn);
            }
        });
    {
        std::lock_guard<std::mutex> locker(m_mutexHttpServer);
        m_httpServer = httpServer;
    }
    return httpServer->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd, errDesc);
}

void HttpFileServer::stop()
{
    std::shared_ptr<nsocket::http::Server> httpServer = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexHttpServer);
        httpServer = m_httpServer;
        m_httpServer.reset();
    }
    if (httpServer)
    {
        httpServer->stop();
    }
}

bool HttpFileServer::isRunning()
{
    std::shared_ptr<nsocket::http::Server> httpServer = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexHttpServer);
        httpServer = m_httpServer;
    }
    if (httpServer && httpServer->isRunning())
    {
        return true;
    }
    return false;
}

void HttpFileServer::handleDefaultRouter(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn)
{
    bool keeyAlive = false;
    for (auto iter = req->headers.begin(); req->headers.end() != iter; ++iter)
    {
        auto name = iter->first;
        std::transform(name.begin(), name.end(), name.begin(), tolower);
        auto value = iter->second;
        std::transform(value.begin(), value.end(), value.begin(), tolower);
        if ("connection" == name && std::string::npos != value.find("keep-alive"))
        {
            keeyAlive = true;
        }
    }
    auto uri = req->uri;
    uri = nsocket::http::url_decode(uri);
#ifdef _WIN32
    uri = utility::Charset::utf8ToGbk(uri);
#endif
    if ("GET" != req->method) /* 方法不允许 */
    {
        NotHandler handler = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexNotAllowHandler);
            handler = m_notAllowHandler;
        }
        if (handler)
        {
            handler(conn, cid, req, uri);
        }
        else
        {
            auto htmlStr = htmlString(cid, req, "405 Method Not Allow");
            auto resp = nsocket::http::makeResponse405();
            resp->body.insert(resp->body.end(), htmlStr.begin(), htmlStr.end());
            conn.sendAndClose(resp->pack());
        }
        return;
    }
    auto target = m_rootDir + uri;
    utility::FileAttribute attr;
    if (!utility::getFileAttribute(target, attr))
    {
        NotHandler handler = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexNotFoundHandler);
            handler = m_notFoundHandler;
        }
        if (handler)
        {
            handler(conn, cid, req, uri);
        }
        else
        {
            auto htmlStr = htmlString(cid, req, "404 Not Found: " + uri);
            auto resp = nsocket::http::makeResponse404();
            resp->body.insert(resp->body.end(), htmlStr.begin(), htmlStr.end());
            conn.sendAndClose(resp->pack());
        }
        return;
    }
    bool internHandler = false;
    if (attr.isDir) /* 目录 */
    {
        DirAccessHandler handler = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexDirAccessHandler);
            handler = m_dirAccessHandler;
        }
        if (handler)
        {
            handler(conn, keeyAlive, m_rootDir, uri);
        }
        else
        {
            internHandler = true;
            handleDir(conn, m_rootDir, uri);
        }
    }
    else if (attr.isFile) /* 文件 */
    {
        FileGetHandler handler = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexFileGetHandler);
            handler = m_fileGetHandler;
        }
        if (handler)
        {
            handler(conn, keeyAlive, target, attr.size);
        }
        else
        {
            internHandler = true;
            handleFile(conn, target, attr.size);
        }
    }
    if (internHandler && !keeyAlive) /* 非保活才关闭连接 */
    {
        conn.close();
    }
}

void HttpFileServer::handleDir(const nsocket::http::Connector& conn, const std::string& rootDir, const std::string& uri)
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
        /* 固定长度并换行 */
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
        /* 链接内容 */
        if (info.attr.isDir)
        {
            str.append("<a href=\"" + target + "/\">" + (lines.empty() ? target : lines) + "/</a>");
        }
        else
        {
            str.append("<a href=\"" + target + "\">" + (lines.empty() ? target : lines) + +"</a>");
        }
        /* 补位空格 */
        auto spaceCount = MAX_TARGET_NAME_LENGTH - length - (info.attr.isDir ? 1 : 0);
        spaceCount = spaceCount > 0 ? spaceCount : 1;
        std::vector<unsigned int> nonAsciiChars;
        utility::Charset::getCoding(target, &nonAsciiChars);
        for (auto byteCount : nonAsciiChars)
        {
            if (byteCount > 2)
            {
                spaceCount += byteCount - 2;
            }
        }
        /* 最后修改时间/文件大小 */
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

void HttpFileServer::handleFile(const nsocket::http::Connector& conn, const std::string& fileName, size_t fileSize)
{
    utility::FileInfo fi(fileName);
    auto mimeType = nsocket::http::getFileMimeType(fileName);
    std::string textFileData;
    if (fi.isTextFile()) /* 检测文本文件编码 */
    {
        textFileData = fi.readAll();
        switch (utility::Charset::getCoding(textFileData))
        {
        case utility::Charset::Coding::utf8:
            mimeType += "; charset=utf-8";
            break;
        case utility::Charset::Coding::gbk:
            mimeType += "; charset=gb2312";
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
            count = m_fileBlockSize;
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
} // namespace hfs
