#include "curl_req_data.h"

#include <string.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace curlex
{
RawRequestData::RawRequestData(const char* bytes, size_t count, bool chunk)
{
    if (bytes && count > 0)
    {
        m_bytes = bytes;
        m_byteCount = count;
    }
    m_chunk = chunk;
}

RequestData::Type RawRequestData::getType() const
{
    return Type::raw;
}

std::string RawRequestData::toString() const
{
    std::string str;
    if (m_bytes && m_byteCount > 0)
    {
        str.append("{").append("\n");
        str.append("    \"count\": ").append(std::to_string(m_byteCount)).append(",").append("\n");
        str.append("    \"data\": \"").append(m_bytes, m_byteCount).append("\"").append("\n");
        str.append("}");
    }
    return str;
}

const char* RawRequestData::getBytes(size_t& byteCount) const
{
    byteCount = m_byteCount;
    return m_bytes;
}

bool RawRequestData::isChunk() const
{
    return m_chunk;
}

FormRequestData::FormRequestData(const std::map<std::string, std::string>& fieldMap) : m_fieldMap(fieldMap) {}

RequestData::Type FormRequestData::getType() const
{
    return Type::form;
}

std::string FormRequestData::toString() const
{
    std::string str;
    for (auto iter = m_fieldMap.begin(); m_fieldMap.end() != iter; ++iter)
    {
        if (!str.empty())
        {
            str.append("&");
        }
        str.append(iter->first).append("=").append(iter->second);
    }
    return str;
}

std::map<std::string, std::string> FormRequestData::getFieldMap() const
{
    return m_fieldMap;
}

MultipartFormRequestData::MultipartFormRequestData() {}

RequestData::Type MultipartFormRequestData::getType() const
{
    return Type::multipart_form;
}

std::string MultipartFormRequestData::toString() const
{
    std::string str;
    str.append("{").append("\n");
    str.append("    \"texts\":").append("\n");
    str.append("    {").append("\n");
    for (auto iter = m_textMap.begin(); m_textMap.end() != iter;)
    {
        str.append("        \"").append(iter->first).append("\": \"").append(iter->second.text).append("\"");
        ++iter;
        if (m_textMap.end() != iter)
        {
            str.append(",");
        }
        str.append("\n");
    }
    str.append("    },").append("\n");
    str.append("    \"files\":").append("\n");
    str.append("    {").append("\n");
    for (auto iter = m_fileMap.begin(); m_fileMap.end() != iter;)
    {
        str.append("        \"").append(iter->first).append("\": \"").append(iter->second).append("\"");
        ++iter;
        if (m_fileMap.end() != iter)
        {
            str.append(",");
        }
        str.append("\n");
    }
    str.append("    }").append("\n");
    str.append("}");
    return str;
}

std::map<std::string, MultipartFormRequestData::TextInfo> MultipartFormRequestData::getTextMap() const
{
    return m_textMap;
}

bool MultipartFormRequestData::addText(const std::string& fieldName, const std::string& text, const std::string& contentType)
{
    if (fieldName.empty())
    {
        return false;
    }
    if (m_textMap.end() != m_textMap.find(fieldName))
    {
        return false;
    }
    TextInfo ti;
    ti.text = text;
    ti.contentType = contentType;
    m_textMap.insert(std::make_pair(fieldName, ti));
    return true;
}

std::map<std::string, std::string> MultipartFormRequestData::getFileMap() const
{
    return m_fileMap;
}

bool MultipartFormRequestData::addFile(const std::string& fieldName, const std::string& filename)
{
    if (fieldName.empty() || filename.empty())
    {
        return false;
    }
    if (m_fileMap.end() != m_fileMap.find(fieldName))
    {
        return false;
    }
#ifdef _WIN32
    if (0 != _access(filename.c_str(), 0))
    {
        return false;
    }
#else
    if (0 != access(filename.c_str(), F_OK))
    {
        return false;
    }
#endif
    m_fileMap.insert(std::make_pair(fieldName, filename));
    return true;
}

std::map<std::string, MultipartFormRequestData::BufferInfo> MultipartFormRequestData::getBufferMap() const
{
    return m_bufferMap;
}

bool MultipartFormRequestData::addBuffer(const std::string& fieldName, const std::string& bufferName, const char* buffer, size_t bufferSize)
{
    if (fieldName.empty() || !buffer || 0 == bufferSize)
    {
        return false;
    }
    BufferInfo info;
    info.name = bufferName;
    info.buffer = buffer;
    info.bufferSize = bufferSize;
    m_bufferMap.insert(std::make_pair(fieldName, info));
    return true;
}
} // namespace curlex
