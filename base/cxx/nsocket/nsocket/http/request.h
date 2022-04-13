#pragma once
#include <functional>
#include <memory>
#include <string>

#include "../multimap.hpp"

namespace nsocket
{
namespace http
{
/**
 * @brief HTTP方法
 */
#ifdef DELETE
#undef DELETE
#endif
enum class Method
{
    CONNECT,
    DELETE,
    GET,
    HEAD,
    PATCH,
    POST,
    PUT,
    OPTIONS,
    TRACE
};

/**
 * @brief 获取HTTP方法描述
 * @param method 方法
 * @return 方法描述
 */
std::string method_desc(const Method& method);

/**
 * @brief HTTP请求
 */
class Request
{
public:
    /**
     * @brief 头部回调
     */
    using HEAD_CALLBACK = std::function<void()>;

    /**
     * @brief 内容回调
     * @param offset 当前分段数据的偏移值
     * @param data 分段数据
     * @param dataLen 分段数据长度 
     */
    using CONTENT_CALLBACK = std::function<void(size_t offset, const unsigned char* data, int dataLen)>;

    /**
     * @brief 结束回调
     */
    using FINISH_CALLBACK = std::function<void()>;

public:
    /**
     * @brief 解析
     * @param data 收到的数据
     * @param length 数据长度
     * @param headCb 头部回调
     * @param contentCb 内容回调
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parse(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const CONTENT_CALLBACK& contentCb,
              const FINISH_CALLBACK& finishCb);

    /**
     * @brief 是否分块
     * @return true-分块, false-非分块
     */
    bool isTransferEncodingChunked();

    /**
     * @brief 获取内容类型
     * @return 内容类型
     */
    std::string getContentType();

    /**
     * @brief 获取内容长度
     * @return 内容长度(当内容分块时, 返回分块内容长度之和, 非分块时, 返回`Content-Length`的值)
     */
    size_t getContentLength();

public:
    std::string host; /* 客户端主机 */
    int port = 0; /* 客户端端口 */
    std::string method; /* 方法 */
    bool isMethodAllowed = true; /* 是否允许该方法 */
    std::string uri; /* URI */
    CaseInsensitiveMultimap queries; /* 查询参数 */
    std::string version; /* 版本 */
    CaseInsensitiveMultimap headers; /* 头部 */

private:
    /**
     * @brief 重置
     */
    void reset();

    /**
     * @brief 解析方法
     * @param data 数据
     * @param length 数据长度
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseMethod(const unsigned char* data, int length);

    /**
     * @brief 解析URI
     * @param data 数据
     * @param length 数据长度
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseUri(const unsigned char* data, int length);

    /**
     * @brief 解析查询参数
     * @param data 数据
     * @param length 数据长度
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseQueries(const unsigned char* data, int length);

    /**
     * @brief 解析版本
     * @param data 数据
     * @param length 数据长度
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseVersion(const unsigned char* data, int length);

    /**
     * @brief 解析头部
     * @param data 数据
     * @param length 数据长度
     * @param headCb 头部回调
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseHeader(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const FINISH_CALLBACK& finishCb);

    /**
     * @brief 解析内容类型和长度
     */
    void parseContentTypeAndLength();

    /**
     * @brief 解析分块内容
     * @param data 数据
     * @param length 数据长度
     * @param contentCb 内容回调
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseChunk(const unsigned char* data, int length, const CONTENT_CALLBACK& contentCb, const FINISH_CALLBACK& finishCb);

    /**
     * @brief 解析非分块内容
     * @param data 数据
     * @param length 数据长度
     * @param contentCb 内容回调
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseUnchunk(const unsigned char* data, int length, const CONTENT_CALLBACK& contentCb, const FINISH_CALLBACK& finishCb);

    /**
     * @brief 获取方法名最大长度
     * @return 长度
     */
    int maxMethodLength();

    /**
     * @brief 检查方法名是否合法
     * @return true-合法, false-非法
     */
    bool checkMethod();

    /**
     * @brief 获取版本最大长度
     * @return 长度
     */
    int maxVersionLength();

    /**
     * @brief 检查版本是否合法
     * @return true-合法, false-非法
     */
    bool checkVersion();

    /**
     * @brief 清空临时数据
     */
    void clearTmp();

private:
    /**
     * @brief 分割符
     */
    enum class SepFlag
    {
        none,
        r, /* \r */
        rn, /* \r\n */
        rnr /* \r\n\r */
    };

    /**
     * @brief 解析步骤
     */
    enum class ParseStep
    {
        method, /* 方法 */
        uri, /* URI */
        queries, /* 请求参数 */
        version, /* 版本 */
        header, /* 头部 */
        chunk, /* 分块内容 */
        unchunk /* 非分块内容 */
    };

    SepFlag m_sepFlag = SepFlag::none; /* 分隔符 */
    ParseStep m_parseStep = ParseStep::method; /* 解析步骤 */
    bool m_tmpKeyFlag = true; /* 是否键 */
    std::string m_tmpKey; /* 键名 */
    std::string m_tmpValue; /* 键值 */
    bool m_transferEncodingChunked = false; /* 内容是否分块 */
    bool m_chunkParseFlag = true; /* 分块解析标识, true-解析头(大小), false-解析数据(内容) */
    std::string m_chunkSizeHex; /* 分块大小(十六进制) */
    size_t m_chunkSize = 0; /* 分块大小(十进制) */
    size_t m_chunkReceived = 0; /* 单个分块已接收长度 */
    std::string m_contentType; /* 内容类型 */
    size_t m_contentLength = 0; /* 内容长度(当内容分块时, 存储分块内容长度之和, 非分块时, 存储`Content-Length`的值) */
    size_t m_contentReceived = 0; /* 内容已接收长度 */
};
using REQUEST_PTR = std::shared_ptr<Request>;
} // namespace http
} // namespace nsocket
