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
        SIMPLE, /* 简单 */
        CAFILE, /* 带证书 */
        USERPWD /* 带用户名密码 */
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
    virtual Type getType() = 0;

    /**
     * @brief 获取URL
     * @return url
     */
    std::string getUrl();

    /**
     * @brief 是否支持重定向
     * @return true-支持, false-不支持
     */
    bool isEnableRedirect();

    /**
     * @brief 获取重定向最多递归返回数量
     * @return 数量
     */
    int getMaxRedirects();

    /**
     * @brief 设置支持重定
     * @param maxRedirects 可以递归返回的数量, -1无限制
     */
    void setEnableRedirect(int maxRedirects);

    /**
     * @brief 获取连接超时时间
     * @return 超时时间
     */
    int getConnectTimeout();

    /**
     * @brief 设置连接超时时间
     * @param seconds 超时时间(秒)
     */
    void setConnectTimeout(size_t seconds);

    /**
     * @brief 获取超时时间
     * @return 超时时间
     */
    int getTimeout();

    /**
     * @brief 设置超时时间
     * @param seconds 超时时间(秒)
     */
    void setTimeout(size_t seconds);

    /**
     * @brief 是否启用保活
     * @return true-启用, false-不启用
     */
    bool isKeepAlive();

    /**
     * @brief 获取保活空闲时间
     * @return 空闲时间
     */
    size_t getKeepAliveIdle();

    /**
     * @brief 获取保活探测间隔时间
     * @return 空闲时间
     */
    size_t getKeepAliveInterval();

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
    std::map<std::string, std::string> getHeaders();

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
    std::string getCookieFile();

    /**
     * @brief 设置cookie文件
     * @param filename 文件名
     */
    void setCookieFile(const std::string& filename);

    /**
     * @brief 获取数据
     * @return 数据
     */
    RequestDataPtr getData();

    /**
     * @brief 设置数据
     * @param data 数据
     */
    void setData(const RequestDataPtr& data);

private:
    Type m_type = Type::SIMPLE; /* 类型 */
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

    Type getType() override;
};

using SimpleRequestPtr = std::shared_ptr<SimpleRequest>;

/**
 * @brief 带证书请求类
 */
class CafileRequest final : public Request
{
public:
    CafileRequest(const std::string& sslCaFilename, const std::string& url);

    virtual ~CafileRequest() = default;

    Type getType() override;

    /**
     * @brief 获取证书文件名
     * @return 文件名
     */
    std::string getCaFilename();

private:
    std::string m_sslCaFilename; /* 证书文件名(全路径) */
};

using CafileRequestPtr = std::shared_ptr<CafileRequest>;

/**
 * @brief 带用户名密码请求类
 */
class UserpwdRequest final : public Request
{
public:
    UserpwdRequest(const std::string& username, const std::string& password, const std::string& url);

    virtual ~UserpwdRequest() = default;

    Type getType() override;

    /**
     * @brief 获取用户名
     * @return 用户名
     */
    std::string getUsername();

    /**
     * @brief 获取密码
     * @return 密码
     */
    std::string getPassword();

private:
    std::string m_username; /* 用户名 */
    std::string m_password; /* 密码 */
};

using UserpwdRequestPtr = std::shared_ptr<UserpwdRequest>;
} // namespace curlex