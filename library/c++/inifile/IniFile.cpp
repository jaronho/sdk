/**********************************************************************
* Author:	jaron.ho
* Date:		2018-04-03
* Brief:	ini file reader/writter
**********************************************************************/
#include "IniFile.h"
#include <algorithm>

void IniFile::trimLeft(std::string& str, char c /*= ' '*/) {
    int len = str.length();
    int i = 0;
    while (str[i] == c && '\0' != str[i]) {
        ++i;
    }
    if (0 != i) {
        str = std::string(str, i, len - i);
    }
}

void IniFile::trimRight(std::string& str, char c /*= ' '*/) {
    int len = str.length();
    int i = 0;
    for (i = len - 1; i >= 0; --i) {
        if (str[i] != c) {
            break;
        }
    }
    str = std::string(str, 0, i + 1);
}

void IniFile::trimLeftRightSpace(std::string& str) {
    int len = str.length();
    int i = 0;
    while (isspace(str[i]) && '\0' != str[i]) {
        ++i;
    }
    if (0 != i) {
        str = std::string(str, i, len - i);
    }
    len = str.length();
    for (i = len - 1; i >= 0; --i) {
        if (!isspace(str[i])) {
            break;
        }
    }
    str = std::string(str, 0, i + 1);
}

int IniFile::getLine(FILE* fp, std::string& line, int lineSize /*= 128*/) {
    int plen = 0;
    int buf_size = lineSize * sizeof(char);
    char* buf = (char*)malloc(buf_size);
    char* pbuf = NULL;
    char* p = buf;
    if (NULL == buf) {
        fprintf(stderr, "no enough memory!\n");
        exit(-1);
    }
    memset(buf, 0, buf_size);
    int total_size = buf_size;
    while (NULL != fgets(p, buf_size, fp)) {
        plen = strlen(p);
        if (plen > 0 && '\n' != p[plen - 1] && !feof(fp)) {
            total_size = strlen(buf) + buf_size;
            pbuf = (char*)realloc(buf, total_size);
            if (NULL == pbuf) {
                free(buf);
                fprintf(stderr, "no enough memory!\n");
                exit(-1);
            }
            buf = pbuf;
            p = buf + strlen(buf);
            continue;
        } else {
            break;
        }
    }
    char* tmp = buf;
    if (strlen(tmp) >= 3 && ('\xEF' == tmp[0] && '\xBB' == tmp[1] && '\xBF' == tmp[2])) { /* skip BOM */
        tmp = tmp + 3;
    }
    line = tmp;
    free(buf);
    buf = NULL;
    return line.length();
}

bool IniFile::getKeyAndValue(const std::string& content, std::string& key, std::string& value, char c /*= '='*/) {
    int len = content.length();
    int i = 0;
    while (i < len && content[i] != c) {
        ++i;
    }
    if (i >= 0 && i < len) {
        key = std::string(content.c_str(), i);
        value = std::string(content.c_str() + i + 1, len - i - 1);
        trimLeftRightSpace(key);
        trimLeftRightSpace(value);
        return true;
    }
    return false;
}

IniFile::IniFile(void) {
    mFlags.push_back("#");
    mFlags.push_back(";");
}

IniFile::~IniFile(void) {
    release();
    mFlags.clear();
}

int IniFile::open(const std::string& fileName, int lineLength /*= 128*/) {
    if ("" == fileName) {
        return 1;
    }
    release();
    mFileName = fileName;
    FILE* fp = fopen(fileName.c_str(), "r");
    if (NULL == fp) {
        return 2;
    }
    IniSection* section = new IniSection();
    mSections[""] = section;
    std::string line;
    std::string comment;
    while (getLine(fp, line, lineLength) > 0) {
        trimRight(line, '\r');
        trimRight(line, '\n');
        trimLeftRightSpace(line);
        if (!isComment(line)) {
            /*
             * 针对 “value=1 #测试” 这种后面有注释的语句
             * 重新分割line，并添加注释到commnet
             * 注意：这种情况保存后会变成
             * #测试
             * value=1
             */
            std::string subline;
            std::string tmp = line;
            for (size_t i = 0, sz = mFlags.size(); i < sz; ++i) {
                subline = line.substr(0, line.find(mFlags[i]));
                line = subline;
            }
            comment += tmp.substr(line.length());
        }
        trimLeftRightSpace(line);
        if (line.length() <= 0) {
            continue;
        }
        if ('[' == line[0]) {
            section = NULL;
            int index = line.find_first_of(']');
            if (-1 == index) {
                fclose(fp);
                fprintf(stderr, "没有找到匹配的]\n");
                return 3;
            }
            int len = index - 1;
            if (len <= 0) {
                fclose(fp);
                fprintf(stderr, "段为空\n");
                return 4;
            }
            std::string s(line, 1, len);
            if (NULL != getSection(s.c_str())) {
                fclose(fp);
                fprintf(stderr, "此段已存在: %s\n", s.c_str());
                return 5;
            }
            section = new IniSection();
            mSections[s] = section;
            section->name = s;
            section->comment = comment;
            comment = "";
        } else if (isComment(line)) {
            if (comment != "") {
                comment += '\n' + line ;
            } else {
                comment = line;
            }
        } else {
            std::string key, value;
            if (getKeyAndValue(line, key, value)) {
                IniItem item;
                item.key = key;
                item.value = value;
                item.comment = comment;
                section->items.push_back(item);
            } else {
                fclose(fp);
                fprintf(stderr, "解析参数失败: %s\n", line.c_str());
                return 6;
            }
            comment = "";
        }
    }
    fclose(fp);
    return 0;
}

int IniFile::save(const std::string& fileName /*= ""*/) {
    if ("" == fileName && "" == mFileName) {
        return 1;
    }
    std::string data = "";
    for (std::map<std::string, IniSection*>::iterator sect = mSections.begin(); mSections.end() != sect; ++sect) {
        IniSection* section = sect->second;
        if ("" != section->comment) {
            data += section->comment;
            data += '\n';
        }
        if ("" != sect->first) {
            data += std::string("[") + sect->first + std::string("]");
            data += '\n';
        }
        for (std::vector<IniItem>::iterator item = section->items.begin(); section->items.end() != item; ++item) {
            if ("" != item->comment) {
                data += item->comment;
                data += '\n';
            }
            data += item->key + "=" + item->value;
            data += '\n';
        }
    }
    FILE* fp = fopen("" == fileName ? mFileName.c_str() : fileName.c_str(), "w");
    if (NULL == fp) {
        return 2;
    }
    fwrite(data.c_str(), 1, data.length(), fp);
    fflush(fp);
    fclose(fp);
    return 0;
}

bool IniFile::hasSection(const std::string& section) {
    return NULL != getSection(section);
}

bool IniFile::hasKey(const std::string& section, const std::string& key) {
    IniSection* sect = getSection(section);
    if (NULL != sect) {
        for (std::vector<IniItem>::iterator iter = sect->items.begin(); sect->items.end() != iter; ++iter) {
            if (key == iter->key) {
                return true;
            }
        }
    }
    return false;
}

void IniFile::deleteSection(const std::string& section) {
    IniSection* sect = getSection(section);
    if (NULL != sect) {
        mSections.erase(section);
        delete sect;
    }
}

void IniFile::deleteKey(const std::string& section, const std::string& key) {
    IniSection* sect = getSection(section);
    if (NULL != sect) {
        for (std::vector<IniItem>::iterator iter = sect->items.begin(); sect->items.end() != iter; ++iter) {
            if (key == iter->key) {
                sect->items.erase(iter);
                break;
            }
        }
    }
}

std::vector<std::string> IniFile::getCommentFlags(void) {
    return mFlags;
}

void IniFile::setCommentFlags(const std::vector<std::string>& flags) {
    if (!flags.empty()) {
        mFlags = flags;
    }
}

int IniFile::getSectionComment(const std::string& section, std::string& comment) {
    comment = "";
    IniSection* sect = getSection(section);
    if (NULL == sect) {
        return 1;
    }
    comment = sect->comment;
    return 0;
}

int IniFile::setSectionComment(const std::string& section, const std::string& comment) {
    IniSection* sect = getSection(section);
    if (NULL == sect) {
        return 1;
    }
    sect->comment = comment;
    return 0;
}

int IniFile::getValue(const std::string& section, const std::string& key, std::string& value, std::string& comment) {
    IniSection* sect = getSection(section);
    if (NULL != sect) {
        for (std::vector<IniItem>::iterator iter = sect->items.begin(); sect->items.end() != iter; ++iter) {
            if (key == iter->key) {
                value = iter->value;
                comment = iter->comment;
                return 0;
            }
        }
    }
    return 1;
}

int IniFile::setValue(const std::string& section, const std::string& key, const std::string& value, const std::string& comment /*= ""*/, bool forceComment /*= false*/) {
    IniSection* sect = getSection(section);
    std::string comt = comment;
    if ("" != comt) {
        comt = mFlags[0] + comt;
    }
    if (NULL == sect) {
        sect = new IniSection();
        if (NULL == sect) {
            fprintf(stderr, "no enough memory!\n");
            exit(-1);
        }
        sect->name = section;
        mSections[section] = sect;
    }
    for (std::vector<IniItem>::iterator iter = sect->items.begin(); sect->items.end() != iter; ++iter) {
        if (iter->key == key) {
            iter->value = value;
            if (forceComment) {
                iter->comment = comt;
            }
            return 0;
        }
    }
    IniItem item;
    item.key = key;
    item.value = value;
    if (forceComment) {
        item.comment = comt;
    }
    sect->items.push_back(item);
    return 0;
}

int IniFile::getValue(const std::string& section, const std::string& key, std::string& value) {
    std::string comment;
    return getValue(section, key, value, comment);
}

int IniFile::getInt(const std::string& section, const std::string& key, int defaultValue /*= 0*/) {
    std::string value, comment;
    if (0 == getValue(section, key, value, comment)) {
        return atoi(value.c_str());
    }
    return defaultValue;
}

void IniFile::setInt(const std::string& section, const std::string& key, int value) {
    char str[16] = {0};
    sprintf(str, "%d", value);
    setValue(section, key, str);
}

long IniFile::getLong(const std::string& section, const std::string& key, long defaultValue /*= 0*/) {
    std::string value, comment;
    if (0 == getValue(section, key, value, comment)) {
        return atol(value.c_str());
    }
    return defaultValue;
}

void IniFile::setLong(const std::string& section, const std::string& key, long value) {
    char str[32] = {0};
    sprintf(str, "%ld", value);
    setValue(section, key, str);
}

float IniFile::getFloat(const std::string& section, const std::string& key, float defaultValue /*= 0.0f*/) {
	return (float)getDouble(section, key, (double)defaultValue);
}

void IniFile::setFloat(const std::string& section, const std::string& key, float value) {
	setDouble(section, key, (double)value);
}

double IniFile::getDouble(const std::string& section, const std::string& key, double defaultValue /*= 0.0f*/) {
    std::string value, comment;
    if (0 == getValue(section, key, value, comment)) {
        return atof(value.c_str());
    }
    return defaultValue;
}

void IniFile::setDouble(const std::string& section, const std::string& key, double value) {
    char str[32] = {0};
    sprintf(str, "%f", value);
    setValue(section, key, str);
}

bool IniFile::getBool(const std::string& section, const std::string& key, bool defaultValue /*= false*/) {
    std::string value, comment;
    if (0 == getValue(section, key, value, comment)) {
        std::transform(value.begin(), value.end(), value.begin(), tolower);
        return "true" == value;
    }
    return defaultValue;
}

void IniFile::setBool(const std::string& section, const std::string& key, bool value) {
    setValue(section, key, value ? "true" : "false");
}

std::string IniFile::getString(const std::string& section, const std::string& key, const std::string& defaultValue /*= ""*/) {
    std::string value, comment;
    if (0 == getValue(section, key, value, comment)) {
        return value;
    }
    return defaultValue;
}

void IniFile::setString(const std::string& section, const std::string& key, const std::string& value) {
    setValue(section, key, value);
}

void IniFile::release(void) {
    mFileName = "";
    for (std::map<std::string, IniSection*>::iterator iter = mSections.begin(); mSections.end() != iter; ++iter) {
        delete iter->second;
    }
    mSections.clear();
}

IniSection* IniFile::getSection(const std::string& section /*= ""*/) {
    std::map<std::string, IniSection*>::iterator iter = mSections.find(section);
    if (mSections.end() != iter) {
        return iter->second;
    }
    return NULL;
}

bool IniFile::isComment(const std::string& str) {
    bool ret = false;
    for (size_t i = 0, sz = mFlags.size(); i < sz; ++i) {
        size_t j = 0, len = mFlags[i].length();
        if (str.length() < len) {
            continue;
        }
        for (j = 0; j < len; ++j) {
            if (str[j] != mFlags[i][j]) {
                break;
            }
        }
        if (j == len) {
            ret = true;
            break;
        }
    }
    return ret;
}
