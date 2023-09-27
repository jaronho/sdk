#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <thread>

#include "httpclient/connection.h"
#include "threading/platform.h"

/**
 * @brief 读取文件内容
 * @param filePath 文件路径
 * @param fileSize [输出]文件大小
 * @param isText 是否文本文件
 * @return 文件内容
 */
char* getFileData(const std::string& filePath, long* fileSize, bool isText)
{
    std::fstream f(filePath, std::ios::in | std::ios::binary);
    if (!f.is_open())
    {
        return nullptr;
    }
    f.seekg(0, std::ios::end);
    *fileSize = f.tellg();
    f.seekg(0, std::ios::beg);
    if (0 == *fileSize)
    {
        f.close();
        return nullptr;
    }
    char* buffer = new char[isText ? *fileSize + 1 : *fileSize];
    f.read((char*)buffer, *fileSize);
    *fileSize = f.gcount();
    if (isText)
    {
        *(buffer + *fileSize) = '\0'; // set EOF
    }
    f.close();
    return buffer;
}

/**
 * @brief 写内容到文件
 * @param data 文件内容
 * @param dataSize 内容大小
 * @param filePath 文件路径
 * @return true-成功, false-失败
 */
bool writeDataToFile(const char* data, long dataSize, const std::string& filePath)
{
    std::fstream f(filePath, std::ios::out | std::ios::binary);
    if (!f.is_open())
    {
        return false;
    }
    if (data && dataSize > 0)
    {
        f.write(data, dataSize);
        f.flush();
    }
    f.close();
    return true;
}

/**
 * @brief 打印响应对象
 * @param resp 响应对象
 */
void printResponse(const curlex::Response& resp)
{
    auto tid = threading::Platform::getThreadId();
    std::string str;
    str.append("[" + std::to_string(tid) + "] response => \n");
    str.append("{\n");
    str.append("    url:" + resp.url + ",\n");
    str.append("    localEndpoint:" + resp.localIp + ":" + std::to_string(resp.localPort) + ",\n");
    str.append("    remoteEndpoint:" + resp.remoteIp + ":" + std::to_string(resp.remotePort) + ",\n");
    str.append("    curlCode:" + std::to_string(resp.curlCode) + ",\n");
    str.append("    errorDesc:" + resp.errorDesc + ",\n");
    str.append("    httpCode:" + std::to_string(resp.httpCode) + ",\n");
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
    str.append("}\n");
    std::cout << str;
}

/**
 * @brief 打印简单响应对象
 * @param resp 简单响应对象
 */
void printSimpleResponse(const curlex::Response& resp)
{
    auto tid = threading::Platform::getThreadId();
    std::string str;
    str.append("[" + std::to_string(tid) + "] response => \n");
    str.append("{\n");
    str.append("    url:" + resp.url + ",\n");
    str.append("    localEndpoint:" + resp.localIp + ":" + std::to_string(resp.localPort) + ",\n");
    str.append("    remoteEndpoint:" + resp.remoteIp + ":" + std::to_string(resp.remotePort) + ",\n");
    str.append("    curlCode:" + std::to_string(resp.curlCode) + ",\n");
    str.append("    errorDesc:" + resp.errorDesc + ",\n");
    str.append("    httpCode:" + std::to_string(resp.httpCode) + ",\n");
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
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("--- " + tag + " --- [" + totalDesc + " / " + nowDesc + "], [" + speedDesc + "]\n");
    std::cout << str;
}

/**
 * @brief 准备测试文件
 */
void prepareTestFiles()
{
    /* 创建测试文本文件 */
    std::string text1 = "Hello world!";
    writeDataToFile(text1.c_str(), text1.size(), "test1.txt");

    std::string text2 = "Reading the fucking source code!!!";
    writeDataToFile(text2.c_str(), text2.size(), "test2.txt");

    /* 创建测试图片文件 */
    unsigned char buffer1[] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x4B, 0x00,
        0x00, 0x00, 0x40, 0x08, 0x02, 0x00, 0x00, 0x00, 0xDD, 0xEB, 0x1D, 0x7E, 0x00, 0x00, 0x00, 0x01, 0x73, 0x52, 0x47, 0x42, 0x00,
        0xAE, 0xCE, 0x1C, 0xE9, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41, 0x4D, 0x41, 0x00, 0x00, 0xB1, 0x8F, 0x0B, 0xFC, 0x61, 0x05, 0x00,
        0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x12, 0x74, 0x00, 0x00, 0x12, 0x74, 0x01, 0xDE, 0x66, 0x1F, 0x78, 0x00,
        0x00, 0x00, 0x61, 0x49, 0x44, 0x41, 0x54, 0x68, 0x43, 0xED, 0xCF, 0x41, 0x0D, 0x00, 0x20, 0x0C, 0x00, 0x31, 0x84, 0xEC, 0x39,
        0xFF, 0xCE, 0xF0, 0x80, 0x8E, 0x23, 0x4D, 0x6A, 0xA0, 0xE7, 0xCE, 0xFE, 0xCD, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF,
        0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0,
        0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF,
        0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xCF, 0xB0, 0xEF, 0xF7, 0xE1, 0xEC, 0x03, 0x62, 0x31, 0x0D, 0x0B, 0xE7,
        0x00, 0x3E, 0x45, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};
    writeDataToFile((const char*)buffer1, sizeof(buffer1) / sizeof(unsigned char), "test1.png");
}

/* 可以用于测试的HTTP服务器 */
const std::string HOST = "http://httpbin.org";

/**
 * @brief 测试DELETE
 */
void testHttpDelete()
{
    std::string url = HOST + "/anything";
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("===================== test curl DELETE, url: " + url + "\n");
    std::cout << str;

    http::Connection conn(url);
    conn.doDelete([](const curlex::Response& resp) { printSimpleResponse(resp); });
}

/**
 * @brief 测试GET
 */
void testHttpGet()
{
#if 0
    std::string url = "https://www.baidu.com";
#else
    std::string url = HOST + "/base64/SFRUUEJJTiBpcyBhd2Vzb21l";
#endif
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("===================== test curl GET, url: " + url + "\n");
    std::cout << str;

    http::Connection conn(url);
    conn.setRecvProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("GET, recv", now, total, speed); });
    conn.doGet([](const curlex::Response& resp) { printSimpleResponse(resp); });
}

/**
 * @brief 测试PUT
 */
void testHttpPut()
{
    std::string url = HOST + "/anything";
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("===================== test curl PUT, url: " + url + "\n");
    std::cout << str;

    http::Connection conn(url);
    conn.setSendProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("PUT, send", now, total, speed); });
    conn.doPut([](const curlex::Response& resp) { printSimpleResponse(resp); });
}

/**
 * @brief 测试POST：字节流(分块)
 */
void testHttpPostRawChunk()
{
    std::string url = HOST + "/anything";
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("===================== test curl POST raw (chunk), url: " + url + "\n");
    std::cout << str;

    long length = 0;
    char* buffer = getFileData("test1.png", &length, false);
    http::Connection conn(url);
    conn.setRawData(buffer, length, true);
    if (buffer)
    {
        delete[] buffer;
        buffer = nullptr;
    }
    conn.setSendProgressFunc(
        [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST raw (chunk), send", now, total, speed); });
    conn.setRecvProgressFunc(
        [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST raw (chunk), recv", now, total, speed); });
    conn.doPost([](const curlex::Response& resp) { printSimpleResponse(resp); });
}

/**
 * @brief 测试POST：字节流(非分块)
 */
void testHttpPostRawNotChunk()
{
    std::string url = HOST + "/anything";
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("===================== test curl POST raw (not chunk), url: " + url + "\n");
    std::cout << str;

    std::string data = "{\"school\":\"yi zhong\",\"class\":[{\"grade\":1,\"number\":6,\"count\":42}]}";
    http::Connection conn(url);
    conn.setRawData(data.c_str(), data.size(), false);
    conn.setSendProgressFunc(
        [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST raw (not chunk), send", now, total, speed); });
    conn.setRecvProgressFunc(
        [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST raw (not chunk), recv", now, total, speed); });
    conn.doPost([](const curlex::Response& resp) { printSimpleResponse(resp); });
}

/**
 * @brief 测试POST：表单数据
 */
void testHttpPostForm()
{
    std::string url = HOST + "/anything";
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("===================== test curl POST form, url: " + url + "\n");
    std::cout << str;

    std::map<std::string, std::string> fieldMap;
    fieldMap.insert(std::make_pair("name", "jaronho"));
    fieldMap.insert(std::make_pair("sex", "male"));
    fieldMap.insert(std::make_pair("age", "33"));

    http::Connection conn(url);
    conn.setFormData(fieldMap);
    conn.setSendProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("POST form, send", now, total, speed); });
    conn.setRecvProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("POST form, recv", now, total, speed); });
    conn.doPost([](const curlex::Response& resp) { printSimpleResponse(resp); });
}

/**
 * @brief 测试POST：多部份表单
 */
void testHttpPostMultipartForm()
{
    std::string url = HOST + "/anything";
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("===================== test curl POST multipart form, url: " + url + "\n");
    std::cout << str;

    http::Connection conn(url);
    conn.appendContentText("name", "jaronho");
    conn.appendContentText("sex", "male");
    conn.appendContentText("age", "33");
    conn.appendContentText("json", "{\"company\":\"jh\",\"address\":\"Xiamen\"}", "application/json");
    conn.appendContentFile("file1", "test1.txt");
    conn.appendContentFile("file2", "test2.txt");
    conn.appendContentFile("file3", "test1.png");
    conn.setSendProgressFunc(
        [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST multipart form, send", now, total, speed); });
    conn.setRecvProgressFunc(
        [&](int64_t now, int64_t total, double speed) { printProgressInfo("POST multipart form, recv", now, total, speed); });
    conn.doPost([](const curlex::Response& resp) { printSimpleResponse(resp); });
}

/**
 * @brief 测试文件下载
 */
void testHttpDownload()
{
    std::string url = "http://127.0.0.1/";
    std::string filename = "test_download.txt";
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("===================== test curl DOWNLOAD, url: " + url + "\n");
    std::cout << str;

    http::Connection conn(url + filename);
    conn.setRecvProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("DOWDLOAD", now, total, speed); });
    conn.doDownload(filename, true, [](const curlex::Response& resp) { printSimpleResponse(resp); });
}

int main()
{
    std::list<std::function<void()>> funcList;
    funcList.emplace_back(testHttpDelete);
    funcList.emplace_back(testHttpGet);
    funcList.emplace_back(testHttpPut);
    funcList.emplace_back(testHttpPostRawChunk);
    funcList.emplace_back(testHttpPostRawNotChunk);
    funcList.emplace_back(testHttpPostForm);
    funcList.emplace_back(testHttpPostMultipartForm);
    funcList.emplace_back(testHttpDownload);

    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("start http client ...\n");
    std::cout << str;
    http::HttpClient::start();
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (!funcList.empty())
        {
            std::function<void()> func = *(funcList.begin());
            funcList.pop_front();
            func();
        }
    }
    return 0;
}
