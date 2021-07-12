#pragma once

#include <map>
#include <memory>
#include <string>

namespace curlex
{
/**
 * @brief 请求数据基类
 */
class RequestData
{
public:
    /**
     * @brief 数据类型
     */
    enum class Type
    {
        RAW, /* 原始字节流 */
        FORM, /* 表单 */
        MULTIPART_FORM /* 多部分表单 */
    };

    virtual ~RequestData() = default;

    /**
     * @brief 获取数据类型
     * @return 数据类型
     */
    virtual Type getType() = 0;

    /**
     * @brief 转换为字符串(用于信息打印)
     * @return 字符串
     */
    virtual std::string toString() = 0;
};

using RequestDataPtr = std::shared_ptr<RequestData>;

/**
 * @brief 二进制字节流数据
 */
class RawRequestData final : public RequestData
{
public:
    /**
     * @brief 构造函数
     * @param bytes 字节流
     * @param count 字节数
     */
    RawRequestData(const char* bytes, size_t count);

    ~RawRequestData();

    Type getType() override;

    std::string toString() override;

    /**
     * @brief 获取字节流
     * @return 字节流
     */
    const char* getBytes();

    /**
     * @brief 获取字节数
     * @return 字节数
     */
    size_t getByteCount();

private:
    char* m_bytes = nullptr; /* 字节流 */
    size_t m_byteCount = 0; /* 字节数 */
};

using RawRequestDataPtr = std::shared_ptr<RawRequestData>;

/**
 * @brief 表单数据
 */
class FormRequestData final : public RequestData
{
public:
    /**
     * @brief 构造函数
     * @param data 表单内容
     */
    FormRequestData(const std::string& data);

    virtual ~FormRequestData() = default;

    Type getType() override;

    std::string toString() override;

    /**
     * @brief 获取表单内容
     * @return 表单内容
     */
    std::string getData();

private:
    std::string m_data; /* 表单内容 */
};

using FormRequestDataPtr = std::shared_ptr<FormRequestData>;

/**
 * @brief 多部分表单数据
 */
class MultipartFormRequestData final : public RequestData
{
public:
    /**
     * @brief 文本信息
     */
    struct TextInfo
    {
        std::string text; /* 文本内容 */
        std::string contentType; /* 内容类型 */
    };

    /**
     * @brief 构造函数
     */
    MultipartFormRequestData();

    virtual ~MultipartFormRequestData() = default;

    Type getType() override;

    std::string toString() override;

    /**
     * @brief 获取文本列表
     * @return 文本列表
     */
    std::map<std::string, TextInfo> getTextMap();

    /**
     * @brief 添加文本
     * @param fieldName 字段名
     * @param text 文本内容
     * @param contentType 内容类型(选填)
     * @return true-成功, false-失败
     */
    bool addText(const std::string& fieldName, const std::string& text, const std::string& contentType = std::string());

    /**
     * @brief 获取文件列表
     * @return 文件列表
     */
    std::map<std::string, std::string> getFileMap();

    /**
     * @brief 添加文件
     * @param fieldName 字段名
     * @param filename 文件名(全路径)
     * @return true-成功, false-失败
     */
    bool addFile(const std::string& fieldName, const std::string& filename);

private:
    std::map<std::string, TextInfo> m_textMap; /* 文本列表 */
    std::map<std::string, std::string> m_fileMap; /* 文件列表 */
};

using MultipartFormRequestDataPtr = std::shared_ptr<MultipartFormRequestData>;
} // namespace curlex
