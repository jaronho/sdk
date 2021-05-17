#pragma once

#include <curl/curl.h>
#include <functional>
#include <map>
#include <string>

namespace curlex
{
/**
 * @brief 数据接收函数
 * @param bytes 收到的字节流
 * @param count 字节个数
 * @return 实际处理的字节个数
 */
using CurlRecvFunc = std::function<size_t(void* bytes, size_t count)>;

/**
 * @brief 进度函数
 * @param nowUploaded 当前已上传的大小(字节)
 * @param totalUpload 需要上传的总大小(字节)
 * @param uploadSpeed 上传速度(字节/秒)
 * @param nowDownloaded 当前已下载的大小(字节)
 * @param totalDownload 需要下载的总大小(字节)
 * @param downloadSpeed 下载速度(字节/秒)
 * @return true-停止, false-继续
 */
using CurlProgressFunc = std::function<bool(int64_t nowUploaded, int64_t totalUpload, double uploadSpeed, int64_t nowDownloaded,
                                            int64_t totalDownload, double downloadSpeed)>;

/**
 * @brief 调试函数
 * @param type 信息类型
 * @param info 信息内容
 */
using CurlDebugFunc = std::function<void(curl_infotype type, const std::string& info)>;

/**
 * @brief Curl的C++封装
 */
class CurlObject
{
public:
    /**
     * @brief 构造函数
     */
    CurlObject();

    /**
     * @brief 构造函数
     * @param sslCaFilename 证书文件
     */
    CurlObject(const std::string& sslCaFilename);

    /**
     * @brief 构造函数
     * @param user 用户名
     * @param password 密码
     */
    CurlObject(const std::string& user, const std::string& password);

    ~CurlObject(void);

    /**
     * @brief 设置CURL选项
     * @param option 选项类型
     * @param value 选项值
     * @return 错误码
     */
    template<typename T>
    CURLcode setOption(CURLoption option, T value)
    {
        if (!m_curl)
        {
            return CURLE_FAILED_INIT;
        }
        return curl_easy_setopt(m_curl, option, value);
    }

    /**
     * @brief 判断对象是否有效
     * @return true-有效, false-无效
     */
    bool isValid(void);

    /**
     * @brief 设置URL
     * @param url 服务器URL
     * @return true-成功, false-失败
     */
    bool setUrl(const std::string& url);

    /**
     * @brief 设置支持重定向
     * @param maxRedirects 可以递归返回的数量, -1无限制
     * @return true-成功, false-失败
     */
    bool setEnableRedirect(int maxRedirects = -1L);

    /**
     * @brief 设置连接超时时间
     * @param seconds 超时时间(秒)
     * @return true-成功, false-失败
     */
    bool setConnectTimeout(size_t seconds = 30);

    /**
     * @brief 设置数据传输超时时间
     * @param seconds 超时时间(秒), 设置为0永远不超时
     * @return true-成功, false-失败
     */
    bool setTimeout(size_t seconds = 60);

    /**
     * @brief 设置保活
     * @param idle 空闲时间(秒)
     * @param interval 探测间隔(秒)
     * @return true-成功, false-失败
     */
    bool setKeepAlive(size_t idle = 60, size_t interval = 60);

    /**
     * @brief 设置请求头部
     * @param headers 头部
     * @return true-成功, false-失败
     */
    bool setHeaders(const std::map<std::string, std::string>& headers);

    /**
     * @brief 设置cookie文件
     * @param filename 文件名
     * @return true-成功, false-失败
     */
    bool setCookieFile(const std::string& filename);

    /**
     * @brief 设置数据接收函数
     * @param func 接收函数
     * @return true-成功, false-失败
     */
    bool setRecvFunc(const CurlRecvFunc& func);

    /**
     * @brief 设置进度函数
     * @param func 进度函数
     * @return true-成功, false-失败
     */
    bool setProgressFunc(CurlProgressFunc func);

    /**
     * @brief 设置调试函数
     * @param func 调试函数
     * @return true-成功, false-失败
     */
    bool setDebugFunc(const CurlDebugFunc& func);

    /**
     * @brief 设置下载时从文件中的哪个偏移位置开始(适用于下载文件)
     * @param offset 偏移位置
     * @return true-成功, false-失败
     */
    bool setResumeOffset(int64_t offset);

    /**
     * @brief 设置原始字节流数据(适用于POST/PUT/DELETE)
     * @param bytes 字节流
     * @param count 字节数
     * @return true-成功, false-失败
     */
    bool setRawData(const unsigned char* bytes, size_t count);

    /**
     * @brief 设置表单数据(适用于POST/PUT/DELETE)
     * @param data 表单数据, 例如: "name=jaron&gender=male&age=33"
     * @return true-成功, false-失败
     */
    bool setFormData(const std::string& data);

    /**
     * @brief 添加多部分表单文本数据(适用于POST/PUT/DELETE)
     * @param fieldName 字段名
     * @param text 文本内容
     * @param contentType 内容类型(选填), 例如: text/html, application/json等
     * @return true-成功, false-失败
     */
    bool addMultipartFormText(const std::string& fieldName, const std::string& text, const std::string& contentType = std::string());

    /**
     * @brief 添加多部分表单文件数据(适用于POST/PUT/DELETE)
     * @param fieldName 字段名
     * @param filename 文件名称(全路径)
     * @return true-成功, false-失败
     */
    bool addMultipartFormFile(const std::string& fieldName, const std::string& filename);

    /**
     * @brief 执行请求
     * @param curlCode curl码
     * @param errorDesc 错误信息
     * @param respCode http响应码
     * @param respHeaders - http响应头
     * @return true-成功, false-失败
     */
    bool perform(int& curlCode, std::string& errorDesc, int& respCode, std::map<std::string, std::string>& respHeaders);

private:
    bool initialize();
    bool initialize(const std::string& sslCaFilename);
    bool initialize(const std::string& user, const std::string& password);
    friend size_t onSendDataFunc(void* buffer, size_t size, size_t number, void* userdata);
    friend size_t onRecvDataFunc(void* buffer, size_t size, size_t number, void* userdata);
    friend size_t onResponseHeaderFunc(void* buffer, size_t size, size_t number, void* userdata);
    friend int onProgressFunc(void* userdata, int64_t totalDownload, int64_t nowDownloaded, int64_t totalUpload, int64_t nowUploaded);
    friend int onDebugFunc(CURL* handle, curl_infotype type, char* data, size_t size, void* userdata);

    /**
     * @brief 发送对象
     */
    class SendObject
    {
    public:
        /**
         * @brief 重置函数
         * @param data 数据
         * @param length 数据长度
         * @return true-成功, false-失败
         */
        bool reset(const unsigned char* data, size_t length);

        /**
         * @brief 重置函数
         * @param data 数据
         * @return true-成功, false-失败
         */
        bool reset(const std::string& data);

        /**
         * @brief 读数据
         * @param dest 目标缓冲区
         * @param count 本次要读的数据长度
         * @return 已读的数据长度
         */
        size_t read(void* dest, size_t count);

    private:
        std::string m_data; /* 要发送的数据 */
        size_t m_length = 0; /* 数据总长度 */
        size_t m_readed = 0; /* 已读的数据长度 */
    };

    /**
     * @brief 进度对象
     */
    struct ProgressObject
    {
        CURL* curl = nullptr; /* curl指针 */
        CurlProgressFunc func = nullptr; /* 进度函数 */
    };

private:
    CURL* m_curl = nullptr; /* curl指针 */
    struct curl_slist* m_headers = nullptr; /* 请求头数据 */
    struct curl_httppost* m_httpPost = nullptr; /* 进行多部分表单操作时需要 */
    struct curl_httppost* m_lastPost = nullptr; /* 进行多部分表单操作时需要 */
    char m_errorBuffer[CURL_ERROR_SIZE] = {0}; /* 错误信息缓存 */
    SendObject m_sendObject; /* 发送对象 */
    CurlRecvFunc m_recvFunc = nullptr; /* 接收函数 */
    ProgressObject m_progressObject; /* 进度对象 */
    CurlDebugFunc m_debugFunc = nullptr; /* 调试函数 */
};
} // namespace curlex
