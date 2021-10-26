#include "multipart_form_data.h"

namespace nsocket
{
namespace http
{
/**
 * @brief ��ȡָ����־���ַ����еĽ���λ��(�����ִ�Сд)
 */
static size_t getMarkEndPos(const std::string& str, const std::string& mark)
{
    std::string tmp;
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (tmp.size() == mark.size())
        {
            /* �����ִ�Сд */
            if (std::equal(tmp.begin(), tmp.end(), mark.begin(), [](char a, char b) { return tolower(a) == tolower(b); }))
            {
                return i;
            }
            return 0;
        }
        else
        {
            tmp.push_back(str[i]);
        }
    }
    return 0;
}

MultipartFormData::MultipartFormData(const std::string& boundary)
{
    if (!boundary.empty())
    {
        m_boundary = "--" + boundary; /* ʵ�ʵı߽�����Ҫ��2��'-' */
    }
}

int MultipartFormData::parse(const unsigned char* data, int length, const TEXT_CALLBACK& textCb, const FILE_CALLBACK& fileCb)
{
    if (!data || length <= 0 || m_boundary.empty())
    {
        return 0;
    }
    int totalUsed = 0;
    while (totalUsed < length)
    {
        const unsigned char* remainData = data + totalUsed;
        int remainLen = length - totalUsed;
        int used = 0;
        switch (m_parseStep)
        {
        case ParseStep::BOUNDARY:
            if ((used = parseBoundary(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::CONTENT_DISPOSITION:
            if ((used = parseContentDisposition(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::CONTENT_TYPE:
            if ((used = parseContentType(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::EMPTY_LINE:
            if ((used = parseEmptyLine(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::CONTENT:
            if ((used = parseContent(remainData, remainLen, textCb, fileCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::ENDING:
            used = remainLen;
            break;
        }
        totalUsed += used;
    }
    return totalUsed;
}

int MultipartFormData::parseBoundary(const unsigned char* data, int length)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if ('\r' == ch)
        {
            if (SepFlag::NONE == m_sepFlag)
            {
                m_sepFlag = SepFlag::R;
            }
            else
            {
                return 0;
            }
        }
        else if ('\n' == ch)
        {
            if (SepFlag::R == m_sepFlag)
            {
                if (0 == m_nowLine.compare(m_boundary)) /* ���ֱ߽��� */
                {
                    m_sepFlag = SepFlag::NONE;
                    m_parseStep = ParseStep::CONTENT_DISPOSITION;
                    m_nowLine.clear();
                    m_name.clear();
                    m_filename.clear();
                    m_contentType.clear();
                    m_fileOffset = 0;
                    return (used + 1);
                }
            }
            return 0;
        }
        else
        {
            if (SepFlag::NONE == m_sepFlag)
            {
                m_nowLine.push_back(ch);
                if (0 == m_nowLine.compare(m_boundary + "--")) /* ���ֱ����� */
                {
                    m_parseStep = ParseStep::ENDING;
                    m_nowLine.clear();
                    m_name.clear();
                    m_filename.clear();
                    m_contentType.clear();
                    m_fileOffset = 0;
                    return (used + 1);
                }
            }
            else
            {
                return 0;
            }
        }
    }
    return used;
}

int MultipartFormData::parseContentDisposition(const unsigned char* data, int length)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if ('\r' == ch)
        {
            if (SepFlag::NONE == m_sepFlag)
            {
                m_sepFlag = SepFlag::R;
            }
            else
            {
                return 0;
            }
        }
        else if ('\n' == ch)
        {
            if (SepFlag::R == m_sepFlag)
            {
                if (parseNameAndFilename()) /* ���ֺ��ļ��������ɹ� */
                {
                    m_sepFlag = SepFlag::NONE;
                    m_parseStep = ParseStep::CONTENT_TYPE;
                    m_nowLine.clear();
                    return (used + 1);
                }
            }
            return 0;
        }
        else
        {
            if (SepFlag::NONE == m_sepFlag)
            {
                m_nowLine.push_back(ch);
            }
            else
            {
                return 0;
            }
        }
    }
    return used;
}

bool MultipartFormData::parseNameAndFilename()
{
    size_t pos = getMarkEndPos(m_nowLine, "Content-Disposition: form-data;");
    if (pos <= 0)
    {
        return false;
    }
    auto sec = m_nowLine.substr(pos + 1);
    if (sec.empty() || '"' != sec[sec.size() - 1])
    {
        return false;
    }
    static const std::string NAME_TAG = "name=\"";
    static const std::string FILENAME_TAG = "; filename=\"";
    pos = sec.rfind(FILENAME_TAG);
    if (std::string::npos != pos) /* ���ļ��� */
    {
        m_filename = sec.substr(pos + FILENAME_TAG.size(), sec.size() - pos - FILENAME_TAG.size() - 1);
        sec = sec.substr(0, pos);
    }
    pos = sec.find(NAME_TAG);
    if (std::string::npos == pos)
    {
        return false;
    }
    m_name = sec.substr(pos + NAME_TAG.size(), sec.size() - pos - NAME_TAG.size() - 1);
    return true;
}

int MultipartFormData::parseContentType(const unsigned char* data, int length)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if ('\r' == ch)
        {
            if (SepFlag::NONE == m_sepFlag)
            {
                m_sepFlag = SepFlag::R;
            }
            else
            {
                return 0;
            }
        }
        else if ('\n' == ch)
        {
            if (SepFlag::R == m_sepFlag)
            {
                if (!m_nowLine.empty())
                {
                    size_t pos = getMarkEndPos(m_nowLine, "Content-Type:");
                    if (pos <= 0)
                    {
                        return 0;
                    }
                    for (size_t i = pos + 1; i < m_nowLine.size(); ++i)
                    {
                        m_contentType.push_back(m_nowLine[i]);
                    }
                }
                m_sepFlag = SepFlag::NONE;
                m_parseStep = !m_nowLine.empty() ? ParseStep::EMPTY_LINE : ParseStep::CONTENT;
                m_nowLine.clear();
                return (used + 1);
            }
            return 0;
        }
        else
        {
            if (SepFlag::NONE == m_sepFlag)
            {
                m_nowLine.push_back(ch);
            }
            else
            {
                return 0;
            }
        }
    }
    return used;
}

int MultipartFormData::parseEmptyLine(const unsigned char* data, int length)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if ('\r' == ch)
        {
            if (SepFlag::NONE == m_sepFlag)
            {
                m_sepFlag = SepFlag::R;
            }
            else
            {
                return 0;
            }
        }
        else if ('\n' == ch)
        {
            if (SepFlag::R == m_sepFlag)
            {
                m_sepFlag = SepFlag::NONE;
                m_parseStep = ParseStep::CONTENT;
                return (used + 1);
            }
            return 0;
        }
        else
        {
            return 0;
        }
    }
    return used;
};

int MultipartFormData::parseContent(const unsigned char* data, int length, const TEXT_CALLBACK& textCb, const FILE_CALLBACK& fileCb)
{
    if (m_filename.empty())
    {
        return parseTextContent(data, length, textCb);
    }
    return parseFileContent(data, length, fileCb);
}

int MultipartFormData::parseTextContent(const unsigned char* data, int length, const TEXT_CALLBACK& textCb)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if ('\r' == ch)
        {
            if (SepFlag::NONE == m_sepFlag)
            {
                m_sepFlag = SepFlag::R;
            }
            else
            {
                return 0;
            }
        }
        else if ('\n' == ch)
        {
            if (SepFlag::R == m_sepFlag)
            {
                if (textCb)
                {
                    textCb(m_name, m_contentType, m_nowLine);
                }
                m_sepFlag = SepFlag::NONE;
                m_parseStep = ParseStep::BOUNDARY;
                m_nowLine.clear();
                return (used + 1);
            }
            return 0;
        }
        else
        {
            if (SepFlag::NONE == m_sepFlag)
            {
                m_nowLine.push_back(ch);
            }
            else
            {
                return 0;
            }
        }
    }
    return used;
}

int MultipartFormData::parseFileContent(const unsigned char* data, int length, const FILE_CALLBACK& fileCb)
{
    int used = 0;
    int rpos = 0;
    std::string prevLine = m_nowLine;
    m_nowLine.clear();
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if ('\r' == ch)
        {
            handlePrevLine(prevLine, fileCb);
            m_sepFlag = SepFlag::R;
            m_nowLine.clear();
            rpos = used;
        }
        else if ('\n' == ch)
        {
            if (SepFlag::R == m_sepFlag)
            {
                m_sepFlag = SepFlag::RN;
            }
            else
            {
                handlePrevLine(prevLine, fileCb);
                m_sepFlag = SepFlag::NONE;
                m_nowLine.clear();
            }
        }
        else
        {
            if (SepFlag::RN == m_sepFlag) /* ����ƴ�ӱ߽��� */
            {
                m_nowLine.push_back(ch);
                if (prevLine.size() + m_nowLine.size() == m_boundary.size()) /* ƥ���˱߽��߳��� */
                {
                    if (0 == (prevLine + m_nowLine).compare(m_boundary)) /* �߽��� */
                    {
                        if (fileCb)
                        {
                            fileCb(m_name, m_filename, m_contentType, m_fileOffset, data, rpos, true); /* �ļ����ս��� */
                        }
                        m_sepFlag = SepFlag::NONE;
                        m_parseStep = ParseStep::BOUNDARY;
                        m_nowLine.clear();
                        return rpos + 2;
                    }
                    /* �Ǳ߽��� */
                    handlePrevLine(prevLine, fileCb);
                    m_sepFlag = SepFlag::NONE;
                    m_nowLine.clear();
                }
            }
            else
            {
                m_sepFlag = SepFlag::NONE;
            }
        }
    }
    if (m_nowLine.empty()) /* �����ڿ��ɵı߽���, ���������ݶ����ļ����� */
    {
        if (fileCb)
        {
            fileCb(m_name, m_filename, m_contentType, m_fileOffset, data, used, false);
        }
        m_fileOffset += used;
    }
    else /* �����б߽���, ���ȴ���ȷ�����ļ����ݿ� */
    {
        if (rpos > 0)
        {
            if (fileCb)
            {
                fileCb(m_name, m_filename, m_contentType, m_fileOffset, data, rpos, false);
            }
            m_fileOffset += rpos;
        }
    }
    return used;
}

void MultipartFormData::handlePrevLine(std::string& prevLine, const FILE_CALLBACK& fileCb)
{
    if (!prevLine.empty())
    {
        /* ǰһ����Ҫƴ����"\r\n" */
        prevLine.insert(prevLine.begin(), '\n');
        prevLine.insert(prevLine.begin(), '\r');
        if (fileCb)
        {
            fileCb(m_name, m_filename, m_contentType, m_fileOffset, (const unsigned char*)prevLine.data(), prevLine.size(), false);
        }
        m_fileOffset += prevLine.size();
        prevLine.clear(); /* ǰһ�������Ѵ���, ��Ҫ��� */
    }
}
} // namespace http
} // namespace nsocket
