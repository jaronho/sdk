#include "httpclient/connection.h"
#include "threading/platform.h"

#include <iostream>
#include <list>

/**
 * @brief 读取文件内容
 * @param filePath 文件路径
 * @param fileSize [输出]文件大小
 * @param isText 是否文本文件
 * @return 文件内容
 */
unsigned char* getFileData(const std::string& filePath, long* fileSize, bool isText)
{
#ifdef _WIN32
    FILE* fp;
    if (0 != fopen_s(&fp, filePath.c_str(), "rb"))
#else
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp)
#endif
    {
        return nullptr;
    }
    fseek(fp, 0, SEEK_END);
    *fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (0 == *fileSize)
    {
        fclose(fp);
        return nullptr;
    }
    unsigned char* buffer = new unsigned char[isText ? *fileSize + 1 : *fileSize];
    *fileSize = fread(buffer, 1, *fileSize, fp);
    if (isText)
    {
        *(buffer + *fileSize) = '\0'; // set EOF
    }
    fclose(fp);
    return buffer;
}

/**
 * @brief 写内容到文件
 * @param data 文件内容
 * @param dataSize 内容大小
 * @param filePath 文件路径
 * @return true-成功, false-失败
 */
bool writeDataToFile(const unsigned char* data, long dataSize, const std::string& filePath)
{
#ifdef _WIN32
    FILE* fp;
    if (0 != fopen_s(&fp, filePath.c_str(), "wb"))
#else
    FILE* fp = fopen(filePath.c_str(), "wb");
    if (!fp)
#endif
    {
        return false;
    }
    if (data && dataSize > 0)
    {
        fwrite(data, dataSize, sizeof(unsigned char), fp);
        fflush(fp);
    }
    fclose(fp);
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
#define SPRINTF(buffer, f, ...) sprintf_s(buffer, f, ##__VA_ARGS__)
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
    writeDataToFile((const unsigned char*)text1.c_str(), text1.size(), "test1.txt");

    std::string text2 = "Reading the fucking source code!!!";
    writeDataToFile((const unsigned char*)text2.c_str(), text2.size(), "test2.txt");

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
    writeDataToFile(buffer1, sizeof(buffer1) / sizeof(unsigned char), "test1.png");
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
 * @brief 测试POST：字节流
 */
void testHttpPostRaw()
{
    std::string url = HOST + "/anything";
    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("===================== test curl POST raw, url: " + url + "\n");
    std::cout << str;

    long length = 0;
    unsigned char* buffer = getFileData("test1.png", &length, false);
    http::Connection conn(url);
    conn.setRawData(buffer, length);
    if (buffer)
    {
        delete[] buffer;
        buffer = nullptr;
    }
    conn.setSendProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("POST raw, send", now, total, speed); });
    conn.setRecvProgressFunc([&](int64_t now, int64_t total, double speed) { printProgressInfo("POST raw, recv", now, total, speed); });
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

    http::Connection conn(url);
    conn.setFormData("name=jaronho&sex=male&age=33");
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
    funcList.push_back(testHttpDelete);
    funcList.push_back(testHttpGet);
    funcList.push_back(testHttpPut);
    funcList.push_back(testHttpPostRaw);
    funcList.push_back(testHttpPostForm);
    funcList.push_back(testHttpPostMultipartForm);
    funcList.push_back(testHttpDownload);

    auto tid = threading::Platform::getThreadId();
    std::string str("[" + std::to_string(tid) + "] ");
    str.append("start http client ...\n");
    std::cout << str;
    http::HttpClient::start();
    while (1)
    {
        http::HttpClient::runOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (!funcList.empty())
        {
            std::function<void()> func = *(funcList.begin());
            funcList.pop_front();
            func();
        }
    }
    http::HttpClient::stop();
    return 0;
}
