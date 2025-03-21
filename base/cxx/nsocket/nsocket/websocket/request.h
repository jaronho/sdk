#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../multimap.hpp"

namespace nsocket
{
namespace ws
{
/**
 * @brief WebSocket请求
 */
class Request
{
public:
    /**
     * @brief 头部回调
     */
    using HEAD_CALLBACK = std::function<void()>;

public:
    /**
     * @brief 解析
     * @param data 收到的数据
     * @param length 数据长度
     * @param headCb 头部回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parse(const unsigned char* data, int length, const HEAD_CALLBACK& headCb);

    /**
     * @brief 是否解析结束
     * @return true-结束, false-未结束
     */
    bool isParseEnd();

    /**
     * @brief 获取Sec-WebSocket-Version值
     * @return 值
     */
    int getSecWebSocketVersion();

    /**
     * @brief 获取Sec-WebSocket-Key值
     * @return 值
     */
    std::string getSecWebSocketKey();

    /**
     * @brief 创建请求数据
     * @param req 请求对象
     * @param hostPort 主机(端口)
     * @param secWebSocketKey [输出]安全密钥
     * @param data [输出]请求数据
     */
    static void create(Request req, const std::string& hostPort, std::string& secWebSocketKey, std::vector<unsigned char>& data);

public:
    std::string method; /* 方法 */
    std::string uri; /* URI */
    CaseInsensitiveMultimap queries; /* 查询参数 */
    std::string version; /* 版本 */
    CaseInsensitiveMultimap headers; /* 头部 */

private:
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
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseHeader(const unsigned char* data, int length, const HEAD_CALLBACK& headCb);

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
     * @brief 检查头部是否合法
     * @return true-合法, false-非法
     */
    bool checkHeader();

    /**
     * @brief 清空临时数据
     */
    void clearTmp();

    /**
     * @brief 计算Key值
     * @return Key值
     */
    static std::string calcSecWebSocketKey();

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
        frame /* 帧数据 */
    };

    SepFlag m_sepFlag = SepFlag::none; /* 分隔符 */
    ParseStep m_parseStep = ParseStep::method; /* 解析步骤 */
    bool m_tmpKeyFlag = true; /* 是否键 */
    std::string m_tmpKey; /* 键名 */
    std::string m_tmpValue; /* 键值 */
    int m_secWebSocketVersion = 0; /* Sec-WebSocket-Version值 */
    std::string m_secWebSocketKey; /* Sec-WebSocket-Key值 */
};
using REQUEST_PTR = std::shared_ptr<Request>;
} // namespace ws
} // namespace nsocket
