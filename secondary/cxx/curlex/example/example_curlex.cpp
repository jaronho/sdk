#include "../curlex/curlex.h"

#include <iostream>

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
    std::cout << "response => " << std::endl;
    std::cout << "{" << std::endl;
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
    std::cout << "    }" << std::endl;
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
    std::cout << "    body:" << std::endl;
    std::cout << resp.body << std::endl;
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
    std::cout << "--- " << tag << " --- [" << totalDesc << " / " << nowDesc << "], [" << speedDesc << "]" << std::endl;
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
 * @brief 测试GET
 */
void testCurlGet()
{
    const std::string TAG = "GET, recv";
#if 0
    auto req = std::make_shared<curlex::SimpleRequest>("https://www.baidu.com");
#else
    std::string url = HOST + "/base64/SFRUUEJJTiBpcyBhd2Vzb21l";
    std::cout << "===================== test curl GET" << std::endl;
    std::cout << "url: " << url << std::endl;
    auto req = std::make_shared<curlex::SimpleRequest>(url);
#endif
    curlex::FuncSet funcSet;
    funcSet.recvProgressFunc = [&](int64_t now, int64_t total, double speed) { printProgressInfo(TAG, now, total, speed); };
    curlex::Response resp;
    auto ret = curlex::curlGet(req, funcSet, resp);
    std::cout << "ret: " << ret << std::endl;
    printSimpleResponse(resp);
}

/**
 * @brief 测试POST：字节流
 */
void testCurlPostRaw()
{
    const std::string TAG1 = "POST raw, send";
    const std::string TAG2 = "POST raw, recv";
    std::string url = HOST + "/anything";
    std::cout << "===================== test curl POST raw" << std::endl;
    std::cout << "url: " << url << std::endl;
    long length = 0;
    unsigned char* buffer = getFileData("test1.png", &length, false);
    auto data = std::make_shared<curlex::RawRequestData>(buffer, length);
    if (buffer)
    {
        delete[] buffer;
        buffer = nullptr;
    }
    auto req = std::make_shared<curlex::SimpleRequest>(url);
    req->setData(data);
    curlex::FuncSet funcSet;
    funcSet.sendProgressFunc = [&](int64_t now, int64_t total, double speed) { printProgressInfo(TAG1, now, total, speed); };
    funcSet.recvProgressFunc = [&](int64_t now, int64_t total, double speed) { printProgressInfo(TAG2, now, total, speed); };
    curlex::Response resp;
    auto ret = curlex::curlPost(req, funcSet, resp);
    std::cout << "ret: " << ret << std::endl;
    printSimpleResponse(resp);
}

/**
 * @brief 测试POST：表单数据
 */
void testCurlPostForm()
{
    const std::string TAG1 = "POST form, send";
    const std::string TAG2 = "POST form, recv";
    std::string url = HOST + "/anything";
    std::cout << "===================== test curl POST form" << std::endl;
    std::cout << "url: " << url << std::endl;
    auto data = std::make_shared<curlex::FormRequestData>("name=jaronho&sex=male&age=33");
    auto req = std::make_shared<curlex::SimpleRequest>(url);
    req->setData(data);
    curlex::FuncSet funcSet;
    funcSet.sendProgressFunc = [&](int64_t now, int64_t total, double speed) { printProgressInfo(TAG1, now, total, speed); };
    funcSet.recvProgressFunc = [&](int64_t now, int64_t total, double speed) { printProgressInfo(TAG2, now, total, speed); };
    curlex::Response resp;
    auto ret = curlex::curlPost(req, funcSet, resp);
    std::cout << "ret: " << ret << std::endl;
    printSimpleResponse(resp);
}

/**
 * @brief 测试POST：多部份表单
 */
void testCurlPostMultipartForm()
{
    const std::string TAG1 = "POST multipart form, send";
    const std::string TAG2 = "POST multipart form, recv";
    std::string url = HOST + "/anything";
    std::cout << "===================== test curl POST multipart form" << std::endl;
    std::cout << "url: " << url << std::endl;
    auto data = std::make_shared<curlex::MultipartFormRequestData>();
    data->addText("name", "jaronho");
    data->addText("sex", "male");
    data->addText("age", "33");
    data->addText("json", "{\"company\":\"jh\",\"address\":\"Xiamen\"}", "application/json");
    data->addFile("file1", "test1.txt");
    data->addFile("file2", "test2.txt");
    data->addFile("file3", "test1.png");
    auto req = std::make_shared<curlex::SimpleRequest>(url);
    req->setData(data);
    curlex::FuncSet funcSet;
    funcSet.sendProgressFunc = [&](int64_t now, int64_t total, double speed) { printProgressInfo(TAG1, now, total, speed); };
    funcSet.recvProgressFunc = [&](int64_t now, int64_t total, double speed) { printProgressInfo(TAG2, now, total, speed); };
    curlex::Response resp;
    auto ret = curlex::curlPost(req, funcSet, resp);
    std::cout << "ret: " << ret << std::endl;
    printSimpleResponse(resp);
}

/**
 * @brief 测试PUT
 */
void testCurlPut()
{
    const std::string TAG = "PUT raw, send";
    std::string url = HOST + "/anything";
    std::cout << "===================== test curl PUT raw" << std::endl;
    std::cout << "url: " << url << std::endl;
    auto req = std::make_shared<curlex::SimpleRequest>(url);
    curlex::FuncSet funcSet;
    funcSet.sendProgressFunc = [&](int64_t now, int64_t total, double speed) { printProgressInfo(TAG, now, total, speed); };
    curlex::Response resp;
    auto ret = curlex::curlPut(req, funcSet, resp);
    std::cout << "ret: " << ret << std::endl;
    printSimpleResponse(resp);
}

/**
 * @brief 测试DELETE
 */
void testCurlDelete()
{
    std::string url = HOST + "/anything";
    std::cout << "===================== test curl DELETE" << std::endl;
    std::cout << "url: " << url << std::endl;
    auto req = std::make_shared<curlex::SimpleRequest>(url);
    curlex::FuncSet funcSet;
    curlex::Response resp;
    auto ret = curlex::curlDelete(req, funcSet, resp);
    std::cout << "ret: " << ret << std::endl;
    printSimpleResponse(resp);
}

/**
 * @brief 测试文件下载
 */
void testCurlDownload()
{
    const std::string TAG = "DOWDLOAD";
    std::string url = "http://127.0.0.1/";
    std::string filename = "test_download.txt";
    std::cout << "===================== test curl DOWNLOAD" << std::endl;
    std::cout << "url: " << url << std::endl;
    auto req = std::make_shared<curlex::SimpleRequest>(url + filename);
    curlex::FuncSet funcSet;
    funcSet.recvProgressFunc = [&](int64_t now, int64_t total, double speed) { printProgressInfo(TAG, now, total, speed); };
    curlex::Response resp;
    auto ret = curlex::curlDownload(req, filename, true, funcSet, resp);
    std::cout << "ret: " << ret << std::endl;
    printResponse(resp);
}

int main()
{
    prepareTestFiles();

    testCurlGet();
    std::cout << std::endl << std::endl;
    testCurlPostRaw();
    std::cout << std::endl << std::endl;
    testCurlPostForm();
    std::cout << std::endl << std::endl;
    testCurlPostMultipartForm();
    std::cout << std::endl << std::endl;
    testCurlPut();
    std::cout << std::endl << std::endl;
    testCurlDelete();
    std::cout << std::endl << std::endl;
    testCurlDownload();
}
