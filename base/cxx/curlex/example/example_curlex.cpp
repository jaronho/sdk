#include <fstream>
#include <iostream>
#include <thread>

#include "curlex/curlex.h"

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
    std::cout << "response => " << std::endl;
    std::cout << "{" << std::endl;
    std::cout << "    url:" << resp.url << "," << std::endl;
    std::cout << "    localEndpoint:" << resp.localIp << ":" << resp.localPort << "," << std::endl;
    std::cout << "    remoteEndpoint:" << resp.remoteIp << ":" << resp.remotePort << "," << std::endl;
    std::cout << "    curlCode:" << resp.curlCode << "," << std::endl;
    std::cout << "    errorDesc:" << resp.errorDesc << "," << std::endl;
    std::cout << "    httpCode:" << resp.httpCode << "," << std::endl;
    std::cout << "    headers:" << std::endl;
    std::cout << "    {" << std::endl;
    auto headerCount = resp.headers.size();
    auto headerIndex = 0;
    auto headerIter = resp.headers.begin();
    for (; resp.headers.end() != headerIter; ++headerIter)
    {
        auto headerName = headerIter->first;
        auto headerValue = headerIter->second;
        ++headerIndex;
        std::cout << "        " << headerName << ":" << headerValue << (headerCount == headerIndex ? "" : ",") << std::endl;
    }
    std::cout << "    }," << std::endl;
    std::cout << "    elapsed:" << resp.elapsed << "(ms)" << std::endl;
    std::cout << "}" << std::endl;
}

/**
 * @brief 打印简单响应对象
 * @param resp 简单响应对象
 */
void printSimpleResponse(const curlex::Response& resp)
{
    std::cout << "response => " << std::endl;
    std::cout << "{" << std::endl;
    std::cout << "    url:" << resp.url << "," << std::endl;
    std::cout << "    localEndpoint:" << resp.localIp << ":" << resp.localPort << "," << std::endl;
    std::cout << "    remoteEndpoint:" << resp.remoteIp << ":" << resp.remotePort << "," << std::endl;
    std::cout << "    curlCode:" << resp.curlCode << "," << std::endl;
    std::cout << "    errorDesc:" << resp.errorDesc << "," << std::endl;
    std::cout << "    httpCode:" << resp.httpCode << "," << std::endl;
    std::cout << "    headers:" << std::endl;
    std::cout << "    {" << std::endl;
    auto headerCount = resp.headers.size();
    auto headerIndex = 0;
    auto headerIter = resp.headers.begin();
    for (; resp.headers.end() != headerIter; ++headerIter)
    {
        auto headerName = headerIter->first;
        auto headerValue = headerIter->second;
        ++headerIndex;
        std::cout << "        " << headerName << ":" << headerValue << (headerCount == headerIndex ? "" : ",") << std::endl;
    }
    std::cout << "    }," << std::endl;
    //std::cout << "    body:" << std::endl;
    //std::cout << resp.body << "," << std::endl;
    std::cout << "    elapsed:" << resp.elapsed << "(ms)" << std::endl;
    std::cout << "}" << std::endl;
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
    std::cout << "--- " << tag << " --- [" << totalDesc << " / " << nowDesc << "], [" << speedDesc << "]" << std::endl;
}

/* 可以用于测试的HTTP服务器 */
const std::string HOST = "http://httpbin.org";

/**
 * @brief 测试POST：字节流(分块)
 */
void testCurlPostRawChunk()
{
    std::string url = HOST + "/anything";
    std::cout << "===================== test curl POST raw (chunk)" << std::endl;
    std::cout << "url: " << url << std::endl;

    long length = 0;
    char* buffer = getFileData("D:\\temp\\bingdu.zip", &length, false);
    auto data = std::make_shared<curlex::RawRequestData>(buffer, length, true);
    if (buffer)
    {
        delete[] buffer;
        buffer = nullptr;
    }
    auto req = std::make_shared<curlex::SimpleRequest>(url);
    req->setData(data);
    curlex::FuncSet funcSet;
    funcSet.sendProgressFunc = [&](int64_t now, int64_t total, double speed) {
        printProgressInfo("POST raw (chunk), send", now, total, speed);
    };
    funcSet.recvProgressFunc = [&](int64_t now, int64_t total, double speed) {
        printProgressInfo("POST raw (chunk), recv", now, total, speed);
    };
    curlex::Response resp;
    auto ret = curlex::curlPost(req, funcSet, resp);
    std::cout << "ret: " << ret << std::endl;
    printSimpleResponse(resp);
}

int main()
{
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        testCurlPostRawChunk();
        std::cout << std::endl << std::endl;
    }
}
