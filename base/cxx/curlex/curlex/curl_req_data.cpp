#include "curl_req_data.h"

#include <string.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace curlex
{
RawRequestData::RawRequestData(const char* bytes, size_t count)
{
    if (bytes && count > 0)
    {
        m_bytes.insert(m_bytes.end(), bytes, bytes + count);
    }
}

RequestData::Type RawRequestData::getType() const
{
    return Type::raw;
}

std::string RawRequestData::toString() const
{
    std::string str;
    str.append("{").append("\n");
    str.append("    \"count\": ").append(std::to_string(m_bytes.size())).append(",").append("\n");
    str.append("    \"data\": \"").append(std::string(m_bytes.begin(), m_bytes.end())).append("\"").append("\n");
    str.append("}");
    return str;
}

std::vector<char> RawRequestData::getBytes() const
{
    return m_bytes;
}

FormRequestData::FormRequestData(const std::string& data) : m_data(data) {}

RequestData::Type FormRequestData::getType() const
{
    return Type::form;
}

std::string FormRequestData::toString() const
{
    return m_data;
}

std::string FormRequestData::getData() const
{
    return m_data;
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
} // namespace curlex
