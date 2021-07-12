#include "utils.h"

namespace utilitiy
{
std::string Utils::replaceString(std::string str, const std::string& rep, const std::string& dest)
{
    if (str.empty() || rep.empty())
    {
        return str;
    }
    std::string::size_type pos = 0;
    while (std::string::npos != (pos = str.find(rep, pos)))
    {
        str.replace(pos, rep.size(), dest);
        pos += dest.size();
    }
    return str;
}

std::vector<std::string> Utils::splitString(std::string str, const std::string& pattern)
{
    std::vector<std::string> result;
    if (str.empty() || pattern.empty())
    {
        return result;
    }
    str.append(pattern);
    std::string::size_type pos;
    for (size_t i = 0; i < str.size(); ++i)
    {
        pos = str.find(pattern, i);
        if (pos < str.size())
        {
            result.push_back(str.substr(i, pos - i));
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}
} // namespace utilitiy
