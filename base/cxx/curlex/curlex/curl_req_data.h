#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

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
        raw, /* 原始字节流 */
        form, /* 表单 */
        multipart_form /* 多部分表单 */
    };

    virtual ~RequestData() = default;

    /**
     * @brief 获取数据类型
     * @return 数据类型
     */
    virtual Type getType() const = 0;

    /**
     * @brief 转换为字符串(用于信息打印)
     * @return 字符串
     */
    virtual std::string toString() const = 0;
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
     * @param chunk 是否数据分块(默认否)
     */
    RawRequestData(const char* bytes, size_t count, bool chunk = false);

    virtual ~RawRequestData() = default;

    Type getType() const override;

    std::string toString() const override;

    /**
     * @brief 获取字节流
     * @return 字节流
     */
    std::vector<char> getBytes() const;

    /**
     * @brief 是否数据分块
     * @return true-是, false-否
     */
    bool isChunk() const;

private:
    std::vector<char> m_bytes; /* 字节流 */
    bool m_chunk = false; /* 是否数据分块 */
};

using RawRequestDataPtr = std::shared_ptr<RawRequestData>;

/**
 * @brief 表单数据, 格式: application/x-www-form-urlencoded
 */
class FormRequestData final : public RequestData
{
public:
    /**
     * @brief 构造函数
     * @param dataMap 表单内容
     */
    FormRequestData(const std::map<std::string, std::string>& m_fieldMap);

    virtual ~FormRequestData() = default;

    Type getType() const override;

    std::string toString() const override;

    /**
     * @brief 获取表单内容
     * @return 表单内容
     */
    std::map<std::string, std::string> getFieldMap() const;

private:
    std::map<std::string, std::string> m_fieldMap; /* 表单内容 */
};

using FormRequestDataPtr = std::shared_ptr<FormRequestData>;

/**
 * @brief 多部分表单数据, 格式: multipart/form-data
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
     * @brief 缓冲区信息
     */
    struct BufferInfo
    {
        std::string name; /* 缓冲区名称 */
        const char* buffer; /* 缓冲区数据 */
        size_t bufferSize = 0; /* 缓冲区大小 */
    };

    /**
     * @brief 构造函数
     */
    MultipartFormRequestData();

    virtual ~MultipartFormRequestData() = default;

    Type getType() const override;

    std::string toString() const override;

    /**
     * @brief 获取文本列表
     * @return 文本列表
     */
    std::map<std::string, TextInfo> getTextMap() const;

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
    std::map<std::string, std::string> getFileMap() const;

    /**
     * @brief 添加文件
     * @param fieldName 字段名
     * @param filename 文件名(全路径)
     * @return true-成功, false-失败
     */
    bool addFile(const std::string& fieldName, const std::string& filename);

    /**
     * @brief 获取缓冲区列表
     * @return 缓冲区列表
     */
    std::map<std::string, BufferInfo> getBufferMap() const;

    /**
     * @brief 添加缓冲区
     * @param fieldName 字段名
     * @param bufferName 缓冲区名称
     * @param buffer 缓冲区数据(需要保证表单数据上传结束前, 缓冲区数据一直在内存中存在)
     * @param bufferSize 缓冲区大小
     * @return true-成功, false-失败
     */
    bool addBuffer(const std::string& fieldName, const std::string& bufferName, const char* buffer, size_t bufferSize);

private:
    std::map<std::string, TextInfo> m_textMap; /* 文本列表 */
    std::map<std::string, std::string> m_fileMap; /* 文件列表 */
    std::map<std::string, BufferInfo> m_bufferMap; /* 缓冲区列表 */
};

using MultipartFormRequestDataPtr = std::shared_ptr<MultipartFormRequestData>;
} // namespace curlex
