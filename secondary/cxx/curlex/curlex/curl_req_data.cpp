#include "curl_req_data.h"

#include <string.h>

namespace curlex
{
RawRequestData::RawRequestData(const unsigned char* bytes, size_t count)
{
    if (bytes && count > 0)
    {
        m_bytes = new unsigned char[count];
        memcpy(m_bytes, bytes, count);
        m_byteCount = count;
    }
}

RawRequestData::~RawRequestData()
{
    if (m_bytes)
    {
        delete[] m_bytes;
        m_bytes = nullptr;
    }
}

RequestData::Type RawRequestData::getType()
{
    return Type::RAW;
}

std::string RawRequestData::toString()
{
    std::string str;
    str.append("{").append("\n");
    str.append("    \"count\": ").append(std::to_string(m_byteCount)).append(",").append("\n");
    str.append("    \"data\": \"").append(std::string(m_bytes ? (char*)m_bytes : "")).append("\"").append("\n");
    str.append("}");
    return str;
}

const unsigned char* RawRequestData::getBytes()
{
    return m_bytes;
}

size_t RawRequestData::getByteCount()
{
    return m_byteCount;
}

FormRequestData::FormRequestData(const std::string& data) : m_data(data) {}

RequestData::Type FormRequestData::getType()
{
    return Type::FORM;
}

std::string FormRequestData::toString()
{
    return m_data;
}

std::string FormRequestData::getData()
{
    return m_data;
}

MultipartFormRequestData::MultipartFormRequestData() {}

RequestData::Type MultipartFormRequestData::getType()
{
    return Type::MULTIPART_FORM;
}

std::string MultipartFormRequestData::toString()
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

std::map<std::string, MultipartFormRequestData::TextInfo> MultipartFormRequestData::getTextMap()
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

std::map<std::string, std::string> MultipartFormRequestData::getFileMap()
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
    FILE* fp;
    if (0 != fopen_s(&fp, filename.c_str(), "r"))
#else
    FILE* fp = fopen(filename.c_str(), "r");
    if (!fp)
#endif
    {
        return false;
    }
    m_fileMap.insert(std::make_pair(fieldName, filename));
    return true;
}
} // namespace curlex
