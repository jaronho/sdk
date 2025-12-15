#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <chrono>
#include <mutex>
#include <thread>

#include "httpclient/connection.h"
#include "utility/cmdline/cmdline.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

#define M_DELETE "delete" /* DELETE */
#define M_GET "get" /* GET */
#define M_PUT "put" /* PUT */
#define M_POST "post" /* POST 字节流 */
#define M_FORM "form" /* POST 表单 */
#define M_MULTIFORM "multiform" /* POST 多表单(支持上传文件) */
#define M_DOWNLOAD "download" /* 下载文件 */

/**
 * @brief 项
 */
struct Item
{
    Item(const std::string& key, const std::string value) : key(key), value(value) {}

    std::string key; /* 健 */
    std::string value; /* 值 */
};

/**
 * @brief 判断项列表是否存在指定健
 * @param itemList 项列表
 * @param key 健
 * @return true-存在, false-不存在
 */
bool isExistItem(const std::vector<Item>& itemList, const std::string& key)
{
    return (itemList.end() != std::find_if(itemList.begin(), itemList.end(), [&](const Item& item) { return (key == item.key); }));
}

/**
 * @brief 打印简单响应对象
 * @param resp 简单响应对象
 */
void printSimpleResponse(const curlex::Response& resp)
{
    std::string str;
    str.append("[" + utility::DateTime::getNow().yyyyMMddhhmmss("-", " ", ":", ".") + "] response => \n");
    str.append("{\n");
    str.append("    url: " + resp.url + ",\n");
    str.append("    localEndpoint: " + resp.localIp + ":" + std::to_string(resp.localPort) + ",\n");
    str.append("    remoteEndpoint: " + resp.remoteIp + ":" + std::to_string(resp.remotePort) + ",\n");
    str.append("    curlCode: " + std::to_string(resp.curlCode) + ",\n");
    str.append("    errorDesc: " + resp.errorDesc + ",\n");
    str.append("    httpCode: " + std::to_string(resp.httpCode) + ",\n");
    str.append("    headers:\n");
    str.append("    {\n");
    auto headerCount = resp.headers.size();
    auto headerIndex = 0;
    auto headerIter = resp.headers.begin();
    for (; resp.headers.end() != headerIter; ++headerIter)
    {
        auto headerName = headerIter->first;
        auto headerValue = headerIter->second;
        ++headerIndex;
        str.append("        " + headerName + ":" + headerValue + (headerCount == headerIndex ? "" : ",") + "\n");
    }
    str.append("    }\n");
    str.append(resp.body + "\n");
    str.append("}\n");
    std::cout << str;
}

/**
 * @brief 打印进度信息
 * @param tag 标签
 * @param now 当前进度
 * @param total 总进度
 * @param speed 速度
 */
#ifdef _WIN32
#define SPRINTF(buffer, f, ...) sprintf_s(buffer, sizeof(buffer), f, ##__VA_ARGS__)
#else
#define SPRINTF(buffer, f, ...) sprintf(buffer, f, ##__VA_ARGS__)
#endif

void printProgressInfo(const std::string& tag, int64_t now, int64_t total, double speed)
{
    static const double GBU = (double)1024 * 1024 * 1024;
    static const double MBU = (double)1024 * 1024;
    static const double KBU = (double)1024;
    /* 总进度 */
    double totalF = (double)total;
    char totalDesc[16] = {0};
    if (totalF > GBU)
    {
        totalF /= GBU;
        SPRINTF(totalDesc, "%0.2fGb", totalF);
    }
    else if (totalF > MBU)
    {
        totalF /= MBU;
        SPRINTF(totalDesc, "%0.1fMb", totalF);
    }
    else if (totalF > KBU)
    {
        totalF /= KBU;
        SPRINTF(totalDesc, "%dKb", (int)totalF);
    }
    else
    {
        SPRINTF(totalDesc, "%dB", (int)totalF);
    }
    /* 当前进度 */
    double nowF = (double)now;
    char nowDesc[16] = {0};
    if (nowF > GBU)
    {
        nowF /= GBU;
        SPRINTF(nowDesc, "%0.1fGb", nowF);
    }
    else if (nowF > MBU)
    {
        nowF /= MBU;
        SPRINTF(nowDesc, "%0.1fMb", nowF);
    }
    else if (nowF > KBU)
    {
        nowF /= KBU;
        SPRINTF(nowDesc, "%dKb", (int)nowF);
    }
    else
    {
        SPRINTF(nowDesc, "%dB", (int)nowF);
    }
    /* 速度 */
    char speedDesc[16] = {0};
    if (speed > GBU)
    {
        speed /= GBU;
        SPRINTF(speedDesc, "%0.1f Gb/s", speed);
    }
    else if (speed > MBU)
    {
        speed /= MBU;
        SPRINTF(speedDesc, "%0.1f Mb/s", speed);
    }
    else if (speed > KBU)
    {
        speed /= KBU;
        SPRINTF(speedDesc, "%d Kb/s", (int)speed);
    }
    else
    {
        SPRINTF(speedDesc, "%d B/s", (int)speed);
    }
    std::string str = "[" + utility::DateTime::getNow().yyyyMMddhhmmss("-", " ", ":", ".") + "] --- " + tag + " --- [" + totalDesc + " / "
                      + nowDesc + "], [" + speedDesc + "]\n";
    std::cout << str;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    /* 关闭控制台程序的快速编辑模式, 否则会出现点击界面, 程序将会变成阻塞状态, 不按回车无法继续运行 */
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE; /* 移除快速编辑模式 */
    SetConsoleMode(hStdin, mode);
#endif
    /* 命令参数 */
    cmdline::parser parser;
    parser.header("HTTP客户端");
    parser.add<std::string>("api", 'a', "要访问的服务端API, 例如: http://192.168.3.122/upload_file", true);
    parser.add<std::string>("method", 'm', "方法, 值: [delete, get, put, post, form, multiform, download]", true);
    parser.add<std::string>("param", 'p', "参数(多个分号分隔), 例如: action:1;feature=1", false, "");
    parser.add<std::string>("header", 'r', "头部(多个分号分隔), 例如: Accept:*/*;Authorization:token", false, "");
    parser.add<std::string>("data", 'd', "流数据(post有效), 例如: {\"school\":\"yi zhong\"}", false, "");
    parser.add<std::string>("text", 't', "文本(多个分号分隔, form/multiform有效), 例如: username:test1;fileType=1", false, "");
    parser.add<std::string>("file", 'f', "文件(多个分号分隔, multiform有效), 例如: f1:/root/1.txt;f2:/root/2.txt", false, "");
    parser.add<std::string>("filename", 'n', "文件名(download有效), 例如: /root/1.txt", false, "");
    parser.parse_check(argc, argv, "用法", "选项", "显示帮助信息并退出");
    printf("%s\n", parser.usage().c_str());
    /* 参数解析 */
    auto api = parser.get<std::string>("api");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    auto sslOn = parser.get<int>("ssl-on");
    auto sslWay = parser.get<int>("ssl-way");
    auto certFmt = parser.get<int>("cert-fmt");
    auto certFile = parser.get<std::string>("cert-file");
    auto pkFile = parser.get<std::string>("pk-file");
    auto pkPwd = parser.get<std::string>("pk-pwd");
#endif
    /* 解析方法 */
    auto method = parser.get<std::string>("method");
    if (!utility::StrTool::equal(M_DELETE, method, false) && !utility::StrTool::equal(M_GET, method, false)
        && !utility::StrTool::equal(M_PUT, method, false) && !utility::StrTool::equal(M_POST, method, false)
        && !utility::StrTool::equal(M_FORM, method, false) && !utility::StrTool::equal(M_MULTIFORM, method, false)
        && !utility::StrTool::equal(M_DOWNLOAD, method, false))
    {
        printf("method参数错误: %s\n", method.c_str());
        return 0;
    }
    /* 解析Param */
    std::vector<Item> paramList;
    auto param = parser.get<std::string>("param");
    auto paramVec = utility::StrTool::split(param, ";");
    for (const auto& item : paramVec)
    {
        auto pos = item.find(":");
        if (std::string::npos == pos)
        {
            printf("param参数错误, 格式不对: %s\n", item.c_str());
            return 0;
        }
        auto key = item.substr(0, pos), value = item.substr(pos + 1);
        utility::StrTool::trimLeftRight(key, ' ');
        if (key.empty())
        {
            printf("param参数错误, 键为空: %s\n", item.c_str());
            return 0;
        }
        utility::StrTool::trimLeftRight(value, ' ');
        if (isExistItem(paramList, key))
        {
            printf("param参数错误, 健重复: %s\n", item.c_str());
            return 0;
        }
        paramList.emplace_back(Item(key, value));
    }
    /* 解析Header */
    std::vector<Item> headerList;
    auto header = parser.get<std::string>("header");
    auto headerVec = utility::StrTool::split(header, ";");
    for (const auto& item : headerVec)
    {
        auto pos = item.find(":");
        if (std::string::npos == pos)
        {
            printf("header参数错误, 格式不对: %s\n", item.c_str());
            return 0;
        }
        auto key = item.substr(0, pos), value = item.substr(pos + 1);
        utility::StrTool::trimLeftRight(key, ' ');
        if (key.empty())
        {
            printf("header参数错误, 键为空: %s\n", item.c_str());
            return 0;
        }
        utility::StrTool::trimLeftRight(value, ' ');
        if (isExistItem(headerList, key))
        {
            printf("header参数错误, 健重复: %s\n", item.c_str());
            return 0;
        }
        headerList.emplace_back(Item(key, value));
    }
    /* 解析流数据 */
    auto data = parser.get<std::string>("data");
    /* 解析文本 */
    std::vector<Item> textList;
    auto text = parser.get<std::string>("text");
    auto textVec = utility::StrTool::split(text, ";");
    for (const auto& item : textVec)
    {
        auto pos = item.find(":");
        if (std::string::npos == pos)
        {
            printf("text参数错误, 格式不对: %s\n", item.c_str());
            return 0;
        }
        auto key = item.substr(0, pos), value = item.substr(pos + 1);
        utility::StrTool::trimLeftRight(key, ' ');
        if (key.empty())
        {
            printf("text参数错误, 键为空: %s\n", item.c_str());
            return 0;
        }
        utility::StrTool::trimLeftRight(value, ' ');
        if (isExistItem(textList, key))
        {
            printf("text参数错误, 健重复: %s\n", item.c_str());
            return 0;
        }
        textList.emplace_back(Item(key, value));
    }
    /* 解析文件 */
    std::vector<Item> fileList;
    auto file = parser.get<std::string>("file");
    auto fileVec = utility::StrTool::split(file, ";");
    for (const auto& item : fileVec)
    {
        auto pos = item.find(":");
        if (std::string::npos == pos)
        {
            printf("file参数错误, 格式不对: %s\n", item.c_str());
            return 0;
        }
        auto key = item.substr(0, pos), value = item.substr(pos + 1);
        utility::StrTool::trimLeftRight(key, ' ');
        if (key.empty())
        {
            printf("file参数错误, 键为空: %s\n", item.c_str());
            return 0;
        }
        utility::StrTool::trimLeftRight(value, ' ');
        if (isExistItem(fileList, key))
        {
            printf("file参数错误, 健重复: %s\n", item.c_str());
            return 0;
        }
        if (!utility::FileInfo(value).exist())
        {
            printf("file参数错误, 文件不存在: %s\n", item.c_str());
            return 0;
        }
        fileList.emplace_back(Item(key, value));
    }
    /* 解析文件名 */
    auto filename = parser.get<std::string>("filename");
    /* 组装URL */
    std::string url = api;
    for (size_t i = 0; i < paramList.size(); ++i)
    {
        url.append(0 == i ? "?" : "&");
        url.append(paramList[i].key).append("=").append(paramList[i].value);
    }
    /* 构建连接 */
    http::Connection conn(url);
    for (const auto& item : headerList)
    {
        conn.appendHeader(item.key, item.value);
    }
    if (utility::StrTool::equal(M_DELETE, method, false))
    {
        conn.doDelete([](const curlex::Response& resp) { printSimpleResponse(resp); }, false);
    }
    else if (utility::StrTool::equal(M_GET, method, false))
    {
        conn.setRecvProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("GET, recv", now, total, speed); });
        conn.doGet([](const curlex::Response& resp) { printSimpleResponse(resp); }, false);
    }
    else if (utility::StrTool::equal(M_PUT, method, false))
    {
        conn.setSendProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("PUT, send", now, total, speed); });
        conn.doPut([](const curlex::Response& resp) { printSimpleResponse(resp); }, false);
    }
    else if (utility::StrTool::equal(M_POST, method, false))
    {
        conn.setRawData(data.c_str(), data.size(), false);
        conn.setSendProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("POST raw, send", now, total, speed); });
        conn.setRecvProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("POST raw, recv", now, total, speed); });
        conn.doPost([](const curlex::Response& resp) { printSimpleResponse(resp); }, false);
    }
    else if (utility::StrTool::equal(M_FORM, method, false))
    {
        std::map<std::string, std::string> fieldMap;
        for (const auto& item : textList)
        {
            fieldMap.insert(std::make_pair(item.key, item.value));
        }
        conn.setFormData(fieldMap);
        conn.setSendProgressFunc(
            [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST form, send", now, total, speed); });
        conn.setRecvProgressFunc(
            [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST form, recv", now, total, speed); });
        conn.doPost([](const curlex::Response& resp) { printSimpleResponse(resp); }, false);
    }
    else if (utility::StrTool::equal(M_MULTIFORM, method, false))
    {
        for (const auto& item : textList)
        {
            conn.appendContentText(item.key, item.value);
        }
        for (const auto& item : fileList)
        {
            conn.appendContentFile(item.key, item.value);
        }
        conn.setSendProgressFunc(
            [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST multipart form, send", now, total, speed); });
        conn.setRecvProgressFunc(
            [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST multipart form, recv", now, total, speed); });
        conn.doPost([](const curlex::Response& resp) { printSimpleResponse(resp); }, false);
    }
    else if (utility::StrTool::equal(M_MULTIFORM, method, false))
    {
        if (filename.empty())
        {
            printf("filename参数错误, 文件名(全路径)为空\n");
            return 0;
        }
        utility::PathInfo pi(utility::FileInfo(filename).path());
        if (!pi.create())
        {
            printf("filename参数错误, 目录创建失败: %s\n", filename.c_str());
            return 0;
        }
        conn.setRecvProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("DOWDLOAD", now, total, speed); });
        conn.doDownload(
            filename, true, [](const curlex::Response& resp) { printSimpleResponse(resp); }, false);
    }
    else
    {
        printf("method参数错误: %s\n", method.c_str());
    }
    return 0;
}
