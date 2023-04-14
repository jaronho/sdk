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
 * @brief WebSocket响应
 */
class Response
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
     * @param secWebSocketKey 安全密钥
     * @param headCb 头部回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parse(const unsigned char* data, int length, const std::string& secWebSocketKey, const HEAD_CALLBACK& headCb);

    /**
     * @brief 是否解析结束
     * @return true-结束, false-未结束
     */
    bool isParseEnd();

    /**
     * @brief 创建响应数据
     * @param resp 响应对象
     * @param secWebSocketKey key值
     * @param data [输出]响应数据
     */
    static void create(Response resp, const std::string& secWebSocketKey, std::vector<unsigned char>& data);

public:
    CaseInsensitiveMultimap headers; /* 头部 */

private:
    /**
     * @brief 解析首行
     * @param data 数据
     * @param length 数据长度
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseFirstLine(const unsigned char* data, int length);

    /**
     * @brief 解析头部
     * @param data 数据
     * @param length 数据长度
     * @param secWebSocketKey 安全密钥
     * @param finishCb 结束回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parseHeader(const unsigned char* data, int length, const std::string& secWebSocketKey, const HEAD_CALLBACK& headCb);

    /**
     * @brief 获取版本最大长度
     * @return 长度
     */
    int maxVersionLength();

    /**
     * @brief 检查头部是否合法
     * @param secWebSocketKey 安全密钥
     * @return true-合法, false-非法
     */
    bool checkHeader(const std::string& secWebSocketKey);

    /**
     * @brief 清空临时数据
     */
    void clearTmp();

    /**
     * @brief 计算Accept值
     * @param secWebSocketKey key值
     * @return Accept值
     */
    static std::string calcSecWebSocketAccept(const std::string& secWebSocketKey);

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
        firstline, /* 首行 */
        header, /* 头部 */
        frame /* 帧数据 */
    };

    SepFlag m_sepFlag = SepFlag::none; /* 分隔符 */
    ParseStep m_parseStep = ParseStep::firstline; /* 解析步骤 */
    std::string m_firstLine; /* 首行 */
    bool m_tmpKeyFlag = true; /* 是否键 */
    std::string m_tmpKey; /* 键名 */
    std::string m_tmpValue; /* 键值 */
};
using RESPONSE_PTR = std::shared_ptr<Response>;
} // namespace ws
} // namespace nsocket
