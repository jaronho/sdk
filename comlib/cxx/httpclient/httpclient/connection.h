#pragma once

#include <atomic>
#include <string>

#include "http_client.h"

namespace http
{
/**
 * @brief HTTP连接
 */
class Connection final
{
public:
    /**
     * @brief 构造函数
     * @param url 服务器URL
     * @param localPort 本地端口, 填0表示自动分配
     */
    Connection(const std::string& url, unsigned int localPort = 0);

    /**
     * @brief 构造函数(SSL单向验证)
     * @param caFile (根)证书文件, 例如: ca.crt
     * @param url 服务器URL
     * @param localPort 本地端口, 填0表示自动分配
     */
    Connection(const std::string& caFile, const std::string& url, unsigned int localPort = 0);

    /**
     * @brief 构造函数(SSL双向验证)
     * @param fileFmt 文件格式
     * @param certFile 证书文件, 例如: client.crt
     * @param privateKeyFile 私钥文件, 例如: client.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: qq123456
     * @param url 服务器URL
     * @param localPort 本地端口, 填0表示自动分配
     */
    Connection(const curlex::FileFormat& fileFmt, const std::string& certFile, const std::string& privateKeyFile,
               const std::string& privateKeyFilePwd, const std::string& url, unsigned int localPort = 0);

    /**
     * @brief 构造函数
     * @param username 用户名
     * @param password 密码
     * @param url 服务器URL
     * @param localPort 本地端口, 填0表示自动分配
     */
    Connection(const std::string& username, const std::string& password, const std::string& url, unsigned int localPort = 0);

    virtual ~Connection() = default;

    /**
     * @brief 设置连接超时时间
     * @param seconds 秒
     */
    void setConnectTimeout(size_t seconds);

    /**
     * @brief 设置超时时间(收到服务器响应)
     * @param seconds 秒
     */
    void setTimeout(size_t seconds);

    /**
     * @brief 添加头部
     * @param key 键
     * @param value 值
     */
    void appendHeader(const std::string& key, const std::string& value);

    /**
     * @brief 设置停止回调
     * @param func 停止回调, 返回值: true-停止, false-继续
     */
    void setStopFunc(const std::function<bool()>& func);

    /**
     * @brief 设置发送(上传)进度回调
     * @param func 回调, 参数: now-当前字节数, total-总字节数, speed-速度(字节/秒)
     */
    void setSendProgressFunc(const std::function<void(int64_t now, int64_t total, double speed)>& func);

    /**
     * @brief 设置接收(下载)进度回调
     * @param func 回调, 参数: now-当前字节数, total-总字节数, speed-速度(字节/秒)
     */
    void setRecvProgressFunc(const std::function<void(int64_t now, int64_t total, double speed)>& func);

    /**
     * @brief 设置字节流数据
     * @param bytes 字节流
     * @param count 字节数
     * @param chunk 是否数据分块(默认否)
     * @return true-成功, false-失败
     */
    bool setRawData(const char* bytes, size_t count, bool chunk = false);

    /**
     * @brief 设置表单数据, 格式: application/x-www-form-urlencoded
     * @param data fieldMap, 底层会自动转为如: "name=jaron&gender=male&age=33"
     * @return true-成功, false-失败
     */
    bool setFormData(const std::map<std::string, std::string>& fieldMap);

    /**
     * @brief 添加多表单文本数据, 格式: multipart/form-data
     * @param key 键
     * @param value 值
     * @param contentType 内容类型
     * @return true-成功, false-失败
     */
    bool appendContentText(const std::string& key, const std::string& value, const std::string& contentType = std::string());

    /**
     * @brief 添加多表单文件, 格式: multipart/form-data
     * @param key 键
     * @param filename 文件名(全路径)
     * @return true-成功, false-失败
     */
    bool appendContentFile(const std::string& key, const std::string& filename);

    /**
     * @brief 连接信息转为描述字符串
     * @return 描述字符串
     */
    std::string toString();

    /**
     * @brief 执行DELETE请求
     * @param respCb 响应回调
     * @param asyncOp 是否异步操作(选填), 默认异步操作(需要HttpClient调用`start`, `tryOnce`或`waitOnce`接口)
     */
    void doDelete(const ResponseCallback& respCb, bool asyncOp = true);

    /**
     * @brief 执行GET请求
     * @param respCb 响应回调
     * @param asyncOp 是否异步操作(选填), 默认异步操作(需要HttpClient调用`start`, `tryOnce`或`waitOnce`接口)
     */
    void doGet(const ResponseCallback& respCb, bool asyncOp = true);

    /**
     * @brief 执行PUT请求
     * @param respCb 响应回调
     * @param asyncOp 是否异步操作(选填), 默认异步操作(需要HttpClient调用`start`, `tryOnce`或`waitOnce`接口)
     */
    void doPut(const ResponseCallback& respCb, bool asyncOp = true);

    /**
     * @brief 执行POST请求
     * @param respCb 响应回调
     * @param asyncOp 是否异步操作(选填), 默认异步操作(需要HttpClient调用`start`, `tryOnce`或`waitOnce`接口)
     */
    void doPost(const ResponseCallback& respCb, bool asyncOp = true);

    /**
     * @brief 执行文件下载请求
     * @param filename 本地要保存的文件名(全路径)
     * @param recover 若本地文件已存在是否强制覆盖, true-强制覆盖重写, false-支持断点续传
     * @param respCb 响应回调
     * @param asyncOp 是否异步操作(选填), 默认异步操作(需要HttpClient调用`start`, `tryOnce`或`waitOnce`接口)
     */
    void doDownload(const std::string& filename, bool recover, const ResponseCallback& respCb, bool asyncOp = true);

private:
    curlex::RequestPtr m_req = nullptr; /* 请求对象 */
    curlex::RequestDataPtr m_data = nullptr; /* 请求数据 */
    curlex::FuncSet m_funcSet; /* 函数集 */
    ResponseCallback m_respCallback = nullptr; /* 响应回调 */
};
} // namespace http
