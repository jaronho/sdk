#pragma once

#include <map>
#include <memory>
#include <string>

#include "curl_req_data.h"

namespace curlex
{
/**
 * @brief 请求基类
 */
class Request
{
public:
    /**
     * @brief 请求类型
     */
    enum class Type
    {
        simple, /* 简单 */
        ssl_2way, /* SSL双向验证 */
        user_pwd /* 带用户名密码 */
    };

    /**
     * @brief 构造函数
     * @param url 资源地址
     */
    Request(const std::string& url);

    virtual ~Request() = default;

    /**
     * @brief 获取请求类型
     * @return 请求类型
     */
    virtual Type getType() const = 0;

    /**
     * @brief 获取URL
     * @return url
     */
    std::string getUrl() const;

    /**
     * @brief 是否支持重定向
     * @return true-支持, false-不支持
     */
    bool isEnableRedirect() const;

    /**
     * @brief 获取重定向最多递归返回数量
     * @return 数量
     */
    int getMaxRedirects() const;

    /**
     * @brief 设置支持重定
     * @param maxRedirects 可以递归返回的数量, -1无限制
     */
    void setEnableRedirect(int maxRedirects);

    /**
     * @brief 获取连接超时时间
     * @return 超时时间
     */
    int getConnectTimeout() const;

    /**
     * @brief 设置连接超时时间
     * @param seconds 超时时间(秒)
     */
    void setConnectTimeout(size_t seconds);

    /**
     * @brief 获取超时时间
     * @return 超时时间
     */
    int getTimeout() const;

    /**
     * @brief 设置超时时间
     * @param seconds 超时时间(秒)
     */
    void setTimeout(size_t seconds);

    /**
     * @brief 是否启用保活
     * @return true-启用, false-不启用
     */
    bool isKeepAlive() const;

    /**
     * @brief 获取保活空闲时间
     * @return 空闲时间
     */
    size_t getKeepAliveIdle() const;

    /**
     * @brief 获取保活探测间隔时间
     * @return 空闲时间
     */
    size_t getKeepAliveInterval() const;

    /**
     * @brief 设置保活
     * @param idel 空闲时间(秒)
     * @param interval 探测间隔时间(秒)
     */
    void setKeepAlive(size_t idel, size_t interval);

    /**
     * @brief 获取头部
     * @return 头部
     */
    std::map<std::string, std::string> getHeaders() const;

    /**
     * @brief 设置头部
     * @param headers 头部
     */
    void setHeaders(const std::map<std::string, std::string>& headers);

    /**
     * @brief 追加头部
     * @param key 键(若已经存在, 则覆盖掉之前的值)
     * @param value 值
     */
    void appendHeader(const std::string& key, const std::string& value);

    /**
     * @brief 获取cookie文件
     * @return 文件名
     */
    std::string getCookieFile() const;

    /**
     * @brief 设置cookie文件
     * @param filename 文件名
     */
    void setCookieFile(const std::string& filename);

    /**
     * @brief 获取数据
     * @return 数据
     */
    RequestDataPtr getData() const;

    /**
     * @brief 设置数据
     * @param data 数据
     */
    void setData(const RequestDataPtr& data);

private:
    Type m_type = Type::simple; /* 类型 */
    std::string m_url; /* 资源地址 */
    bool m_redirect = true; /* 是否支持重定向 */
    int m_maxRedirects = -1; /* 可以递归返回的数量 */
    int m_connectTimeout = -1; /* 连接超时时间(秒) */
    int m_timeout = -1; /* 超时时间(秒) */
    bool m_keepAlive = false; /* 是否保活 */
    size_t m_keepAliveIdle = 60; /* 保活空闲时间 */
    size_t m_keepAliveInterval = 60; /* 保活探测间隔 */
    std::map<std::string, std::string> m_headers; /* 头部 */
    std::string m_cookieFilename; /* cookie文件名 */
    RequestDataPtr m_data; /* 数据 */
};

using RequestPtr = std::shared_ptr<Request>;

/**
 * @brief 简单请求类
 */
class SimpleRequest final : public Request
{
public:
    SimpleRequest(const std::string& url);

    virtual ~SimpleRequest() = default;

    Type getType() const override;
};

using SimpleRequestPtr = std::shared_ptr<SimpleRequest>;

/**
 * @brief SSL双向认证请求类
 */
class Ssl2WayRequest final : public Request
{
public:
    Ssl2WayRequest(const std::string& certFile, const std::string& privateKeyFile, const std::string& privateKeyFilePwd,
                   const std::string& url);

    virtual ~Ssl2WayRequest() = default;

    Type getType() const override;

    /**
     * @brief 获取证书文件
     * @return 证书文件
     */
    std::string getCertFile() const;

    /**
     * @brief 获取私钥文件
     * @return 私钥文件
     */
    std::string getPrivateKeyFile() const;

    /**
     * @brief 获取私钥文件密码
     * @return 私钥文件密码
     */
    std::string getPrivateKeyFilePwd() const;

private:
    std::string m_certFile; /* 证书文件, 例如: client.crt */
    std::string m_privateKeyFile; /* 私钥文件, 例如: client.key */
    std::string m_privateKeyFilePwd; /* 私钥文件密码, 例如: qq123456 */
};

using Ssl2WayRequestPtr = std::shared_ptr<Ssl2WayRequest>;

/**
 * @brief 带用户名密码请求类
 */
class UserpwdRequest final : public Request
{
public:
    UserpwdRequest(const std::string& username, const std::string& password, const std::string& url);

    virtual ~UserpwdRequest() = default;

    Type getType() const override;

    /**
     * @brief 获取用户名
     * @return 用户名
     */
    std::string getUsername() const;

    /**
     * @brief 获取密码
     * @return 密码
     */
    std::string getPassword() const;

private:
    std::string m_username; /* 用户名 */
    std::string m_password; /* 密码 */
};

using UserpwdRequestPtr = std::shared_ptr<UserpwdRequest>;
} // namespace curlex
