#pragma once
#include <functional>
#include <string>
#include <unordered_map>

namespace nsocket
{
namespace http
{
/**
 * @brief 表单数据
 */
class MultipartFormData
{
public:
    /**
     * @brief 文本回调
     * @param name 名称
     * @param contentType 内容类型
     * @param text 文本内容
     */
    using TEXT_CALLBACK = std::function<void(const std::string& name, const std::string& contentType, const std::string& text)>;

    /**
     * @brief 文件回调
     * @param name 名称
     * @param filename 文件名
     * @param contentType 内容类型
     * @param offset 文件数据在文件中的偏移值
     * @param data 文件数据
     * @param dataLen 数据长度
     * @param finish 是否结束
     */
    using FILE_CALLBACK = std::function<void(const std::string& name, const std::string& filename, const std::string& contentType,
                                             size_t offset, const unsigned char* data, int dataLen, bool finish)>;

public:
    /**
     * @brief 构造函数
     * @param boundary 边界线
     */
    MultipartFormData(const std::string& boundary);

    /**
     * @brief 解析
     * @param data 表单数据
     * @param length 数据长度
     * @param textCb 文本回调
     * @param fileCb 文件回调
     * @return 已解析的数据长度, <=0表示解析出错
     */
    int parse(const unsigned char* data, int length, const TEXT_CALLBACK& textCb, const FILE_CALLBACK& fileCb);

private:
    /**
     * @brief 解析边界线
     */
    int parseBoundary(const unsigned char* data, int length);

    /**
     * @brief 解析内容处理信息
     */
    int parseContentDisposition(const unsigned char* data, int length);

    /**
     * @brief 解析名称和文件名
     */
    bool parseNameAndFilename();

    /**
     * @brief 解析内容类型
     */
    int parseContentType(const unsigned char* data, int length);

    /**
     * @brief 解析空行
     */
    int parseEmptyLine(const unsigned char* data, int length);

    /**
     * @brief 解析内容
     */
    int parseContent(const unsigned char* data, int length, const TEXT_CALLBACK& textCb, const FILE_CALLBACK& fileCb);

    /**
     * @brief 解析文本内容
     */
    int parseTextContent(const unsigned char* data, int length, const TEXT_CALLBACK& textCb);

    /**
     * @brief 解析文件内容
     */
    int parseFileContent(const unsigned char* data, int length, const FILE_CALLBACK& fileCb);

    /**
     * @brief 处理前一行遗留数据
     */
    void handlePrevLine(std::string& prevLine, const FILE_CALLBACK& fileCb);

private:
    /**
     * @brief 分割符
     */
    enum class SepFlag
    {
        NONE,
        R, /* \r */
        RN /* \r\n */
    };

    /**
     * @brief 解析步骤
     */
    enum class ParseStep
    {
        BOUNDARY, /* 边界线 */
        CONTENT_DISPOSITION, /* 内容处理信息 */
        CONTENT_TYPE, /* 内容类型 */
        EMPTY_LINE, /* 空行 */
        CONTENT, /* 内容 */
        ENDING /* 表单结束 */
    };

    std::string m_boundary; /* 边界线 */
    SepFlag m_sepFlag = SepFlag::NONE; /* 分隔符 */
    ParseStep m_parseStep = ParseStep::BOUNDARY; /* 解析步骤 */
    std::string m_nowLine; /* 当前行数据 */
    std::string m_name; /* 表单项名 */
    std::string m_filename; /* 文件名 */
    std::string m_contentType; /* 表单项内容类型 */
    size_t m_fileOffset = 0; /* 文件偏移 */
};
} // namespace http
} // namespace nsocket
