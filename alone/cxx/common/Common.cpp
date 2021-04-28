/**********************************************************************
 * Author:	jaron.ho
 * Date:    2012-09-06
 * Brief:	common functions
 **********************************************************************/
#if defined(win32) || defined(_win32) || defined(WIN32) || defined(_WIN32) || \
    defined(win64) || defined(_win64) || defined(WIN64) || defined(_WIN64)
    #define _SYSTEM_WINDOWS_
#endif
#include "Common.h"
#include <fcntl.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#ifdef _SYSTEM_WINDOWS_
#include <direct.h>
#include <io.h>
#include <windows.h>
#pragma warning(disable: 4996)
#else
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#endif
/*********************************************************************/
bool Common::isDigit(char c) {
    return c >= '0' && c <= '9';
}
/*********************************************************************/
bool Common::isDigit(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    bool mask = false;
    int decimal = 0;
    for (size_t i = 0; i < str.size(); ++i) {
        if (0 == i && ('+' == str[i] || '-' == str[i])) {
            mask = true;
            continue;
        }
        if ('.' == str[i]) {
            ++decimal;
            if (decimal > 1) {
                return false;
            } else {
                continue;
            }
        }
        if (!isDigit(str[i])) {
            return false;
        }
    }
    if (mask && 1 == str.size()) {
        return false;
    }
    return true;
}
/*********************************************************************/
int Common::toInt(const std::string& str) {
    return atoi(str.c_str());
}
/*********************************************************************/
long Common::toLong(const std::string& str) {
    return atol(str.c_str());
}
/*********************************************************************/
long long Common::toLongLong(const std::string& str) {
    return strtoll(str.c_str(), NULL, 10);
}
/*********************************************************************/
double Common::toDouble(const std::string& str) {
    return atof(str.c_str());
}
/*********************************************************************/
std::string Common::toString(short u) {
    char buf[8];
    sprintf(buf, "%d", u);
    return buf;
}
/*********************************************************************/
std::string Common::toString(int n) {
    char buf[16];
    sprintf(buf, "%d", n);
    return buf;
}
/*********************************************************************/
std::string Common::toString(unsigned int un) {
    char buf[16];
    sprintf(buf, "%u", un);
    return buf;
}
/*********************************************************************/
std::string Common::toString(long l) {
    char buf[32];
    sprintf(buf, "%ld", l);
    return buf;
}
/*********************************************************************/
std::string Common::toString(unsigned long ul) {
    char buf[32];
    sprintf(buf, "%lu", ul);
    return buf;
}
/*********************************************************************/
std::string Common::toString(long long ll) {
    char buf[64];
    sprintf(buf, "%lld", ll);
    return buf;
}
/*********************************************************************/
std::string Common::toString(unsigned long long ull) {
    char buf[64];
    sprintf(buf, "%llu", ull);
    return buf;
}
/*********************************************************************/
std::string Common::toString(float f) {
    char buf[64];
    sprintf(buf, "%f", f);
    return buf;
}
/*********************************************************************/
std::string Common::toString(double d) {
    char buf[128];
    sprintf(buf, "%f", d);
    return buf;
}
/*********************************************************************/
std::string Common::toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}
/*********************************************************************/
std::string Common::toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}
/*********************************************************************/
std::string Common::newLineString(void) {
    return std::string("\n");
}
/*********************************************************************/
std::string Common::stringOfChar(unsigned int n, char ch) {
    return std::string(n, ch);
}
/*********************************************************************/
std::string Common::limitStringLength(std::string str, size_t len, const std::string& sufStr, const std::string& defStr) {
    if (str.empty()) {
        return defStr;
    }
    if (str.size() > len) {
        return str.substr(0, len) + sufStr;
    }
    return str;
}
/*********************************************************************/
std::wstring Common::limitWstringLength(std::wstring wstr, size_t len, const std::wstring& sufWstr, const std::wstring& defWstr) {
    if (wstr.empty()) {
        return defWstr;
    }
    if (wstr.size() > len) {
        return wstr.substr(0, len) + sufWstr;
    }
    return wstr;
}
/*********************************************************************/
std::vector<std::string> Common::splitString(std::string str, const std::string& pattern) {
    std::vector<std::string> result;
    if (str.empty() || pattern.empty()) {
        return result;
    }
    str.append(pattern);
    std::string::size_type pos;
    for (size_t i = 0; i < str.size(); ++i) {
        pos = str.find(pattern, i);
        if (pos < str.size()) {
            result.push_back(str.substr(i, pos - i));
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}
/*********************************************************************/
std::string Common::replaceString(std::string str, const std::string& rep, const std::string& dest) {
    if (str.empty() || rep.empty()) {
        return str;
    }
    std::string::size_type pos = 0;
    while (std::string::npos != (pos = str.find(rep, pos))) {
        str.replace(pos, rep.size(), dest);
        pos += dest.size();
    }
    return str;
}
/*********************************************************************/
size_t Common::findString(std::string source, std::string target, bool caseSensitive) {
    if (source.empty() || target.empty()) {
        return std::string::npos;
    }
    if (caseSensitive) {
        return source.find(target);
    }
    for (std::string::iterator iter = source.begin(); iter != source.end(); ++iter) {
        *iter = (char)tolower(*iter);
    }
    for (std::string::iterator iter = target.begin(); iter != target.end(); ++iter) {
        *iter = (char)tolower(*iter);
    }
    return source.find(target);
}
/*********************************************************************/
std::string Common::formatString(const char* format, ...) {
    char buf[512];
    memset(buf, 0, sizeof(buf));
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    return buf;
}
/*********************************************************************/
char* Common::wchar2char(const wchar_t* wstr) {
    char* buf = NULL;
    if (!wstr || 0 == wcslen(wstr)) {
        return buf;
    }
#ifdef _SYSTEM_WINDOWS_
    int len = WideCharToMultiByte(CP_ACP, 0, wstr, wcslen(wstr), NULL, 0, NULL, NULL);
    buf = (char*)malloc(sizeof(char) * (len + 1));
    WideCharToMultiByte(CP_ACP, 0, wstr, wcslen(wstr), buf, len, NULL, NULL);
    buf[len] = '\0';
#endif
    return buf;
}
/*********************************************************************/
wchar_t* Common::char2wchar(const char* str) {
    wchar_t* buf = NULL;
    if (!str || 0 == strlen(str)) {
        return buf;
    }
#ifdef _SYSTEM_WINDOWS_
    int len = MultiByteToWideChar(CP_ACP, 0, str, strlen(str), NULL, 0);
    buf = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_ACP, 0, str, strlen(str), buf, len);
    buf[len] = '\0';
#endif
    return buf;
}
/*********************************************************************/
std::string Common::wstring2string(const std::wstring& wstr, const char* locale /*= ""*/) {
    if (wstr.empty()) {
        return "";
    }
    std::string curLocale = setlocale(LC_ALL, NULL);
    char* localeName = setlocale(LC_ALL, locale);
    if (!localeName) {
        setlocale(LC_ALL, curLocale.c_str());
        return "";
    }
    const wchar_t* src = wstr.c_str();
    size_t destSize = 2 * wstr.size() + 1;
    char* dest = (char*)malloc(sizeof(char) * destSize);
    memset(dest, 0, destSize);
    wcstombs(dest, src, destSize);
    std::string result = dest;
    free(dest);
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}
/*********************************************************************/
std::wstring Common::string2wstring(const std::string& str, const char* locale /*= ""*/) {
    if (str.empty()) {
        return L"";
    }
    std::string curLocale = setlocale(LC_ALL, NULL);
    char* localeName = setlocale(LC_ALL, locale);
    if (!localeName) {
        setlocale(LC_ALL, curLocale.c_str());
        return L"";
    }
    const char* src = str.c_str();
    size_t destSize = str.size() + 1;
    wchar_t* dest = (wchar_t*)malloc(sizeof(wchar_t) * destSize);
    wmemset(dest, 0, destSize);
    mbstowcs(dest, src, destSize);
    std::wstring result = dest;
    free(dest);
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}
/*********************************************************************/
bool Common::isStringUTF8(const char* str) {
    unsigned int nBytes = 0;    /* utf8可用1-6个字节编码,ASCII用一个字节 */
    unsigned char chr = *str;
    bool bAllAscii = true;
    for (unsigned int i = 0; '\0' != str[i]; ++i) {
        chr = *(str + i);
        if (0 == nBytes && 0 != (chr & 0x80)) { /* 判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx  */
            bAllAscii = false;
        }
        if (0 == nBytes) {
            if (chr >= 0x80) {  /* 如果不是ASCII码,应该是多字节符,计算字节数 */
                if (chr >= 0xFC && chr <= 0xFD) {
                    nBytes = 6;
                } else if (chr >= 0xF8) {
                    nBytes = 5;
                } else if (chr >= 0xF0) {
                    nBytes = 4;
                } else if (chr >= 0xE0) {
                    nBytes = 3;
                } else if (chr >= 0xC0) {
                    nBytes = 2;
                } else {
                    return false;
                }
                nBytes--;
            }
        } else {
            if (0x80 != (chr & 0xC0)) { /* 多字节符的非首字节,应为 10xxxxxx */
                return false;
            }
            nBytes--;   /* 减到为零为止 */
        }
    }
    if (0 != nBytes) {  /* 违返UTF8编码规则 */
        return false;
    }
    if (bAllAscii) {    /* 如果全部都是ASCII, 也是utf8 */
        return true;
    }
    return true;
}
/*********************************************************************/
bool Common::isStringGBK(const char* str) {
    unsigned int nBytes = 0;    /* GBK可用1-2个字节编码,中文两个,英文一个 */
	unsigned char chr = *str;
	bool bAllAscii = true;  /* 如果全部都是ASCII */
	for (unsigned int i = 0; '\0' != str[i]; ++i) {
		chr = *(str + i);
		if (0 != (chr & 0x80) && 0 == nBytes) {  /* 判断是否ASCII编码,如果不是,说明有可能是GBK */
			bAllAscii = false;
		}
		if (0 == nBytes) {
			if (chr >= 0x80) {
				if (chr >= 0x81 && chr <= 0xFE) {
					nBytes = 2;
				} else {
					return false;
				}
				nBytes--;
			}
		} else{
			if (chr < 0x40 || chr>0xFE) {
				return false;
			}
			nBytes--;
		}
	}
	if (0 != nBytes) { /* 违返规则 */
		return false;
	}
	if (bAllAscii) { /* 如果全部都是ASCII,也是GBK */
		return true;
	}
	return true;
}
/*********************************************************************/
size_t Common::characterPlaceholder(unsigned char ch) {
    const unsigned char charsets[] = {0, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc};
    size_t i = sizeof(charsets);
    while (i > 0) {
        if (ch >= charsets[i - 1]) {
            return i;
        }
        --i;
    }
    return 1;
}
/*********************************************************************/
std::string Common::bytes2hex(const unsigned char* bytes, unsigned int num, const std::string& sep) {
    std::string hex;
    if (!bytes || 0 == num) {
        return hex;
    }
    char tmp[3] = { 0 };
    for (size_t i = 0; i < num; ++i) {
        memset(tmp, 0, 3);
        sprintf(tmp, "%02X", bytes[i]);
        hex += (0 == i ? "" : sep) + tmp;
    }
    return hex;
}
/*********************************************************************/
bool Common::createDir(const std::string& dirName) {
    if (dirName.empty()) {
        return false;
    }
    unsigned int dirLen = dirName.length();
    char* tmp = (char*)malloc(sizeof(char) * (dirLen + 1));
    memset(tmp, 0, dirLen + 1);
    for (unsigned int i = 0; i < dirLen; ++i) {
        tmp[i] = dirName[i];
        int flag = 0;
        if ('/' == tmp[i] || '\\' == tmp[i]) {
            flag = 1;
            tmp[i] = '/';
        } else if (dirLen - 1 == i) {
            flag = 1;
        }
        if (flag) {
            tmp[i + 1] = '\0';
#ifdef _SYSTEM_WINDOWS_
            if (0 != _access(tmp, 0)) {
                if (0 != _mkdir(tmp)) {
                    free(tmp);
                    return false;
                }
            }
#else
            if (0 != access(tmp, F_OK)) {
                if (0 != mkdir(tmp, S_IRWXU | S_IRWXG | S_IRWXO)) {
                    free(tmp);
                    return false;
                }
            }
#endif
        }
    }
    free(tmp);
    return true;
}
/*********************************************************************/
void Common::removeDir(const std::string& dirName) {
#ifdef _SYSTEM_WINDOWS_
    /* method 1 */
#if 0
    struct _finddata_t fileData;
    int handle = _findfirst((dirName + "/*.*").c_str(), &fileData);
    if (-1 == handle || !(_A_SUBDIR & fileData.attrib)) {
        return;
    }
    while (0 == _findnext(handle, &fileData)) {
        if (0 == strcmp(".", fileData.name) || 0 == strcmp("..", fileData.name)) {
            continue;
        }
        std::string subName = dirName + "/" + fileData.name;
        if (_A_SUBDIR & fileData.attrib) {
            removeDir(subName);
        } else {
            remove(subName.c_str());
        }
    }
    _findclose(handle);
    rmdir(dirName.c_str());
#endif
    /* method 2 */
    system(("rd /s /q \"" + dirName + "\"").c_str());
#else
    DIR* dir = opendir(dirName.c_str());
    if (!dir) {
        return;
    }
    struct dirent* dirp = NULL;
    while ((dirp = readdir(dir))) {
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name)) {
            continue;
        }
        std::string subName = dirName + "/" + dirp->d_name;
        DIR* subDir = opendir(subName.c_str());
        if (!subDir) {
            remove(subName.c_str());
        } else {
            closedir(subDir);
            removeDir(subName);
        }
    }
    closedir(dir);
    rmdir(dirName.c_str());
#endif
}
/*********************************************************************/
bool Common::existFile(const std::string& filePath) {
    FILE* fp = fopen(filePath.c_str(), "r");
    if (!fp) {
        return false;
    }
    fclose(fp);
    return true;
}
/*********************************************************************/
bool Common::createFile(const std::string& filePath) {
    FILE* fp = fopen(filePath.c_str(), "wb");
    if (!fp) {
        return false;
    }
    fclose(fp);
    return true;
}
/*********************************************************************/
bool Common::removeFile(const std::string& filePath) {
    return 0 == remove(filePath.c_str());
}
/*********************************************************************/
bool Common::renameFile(const std::string& oldFileName, const std::string& newFileName, bool forceRename /*= false*/) {
    if (oldFileName.empty() || newFileName.empty()) {
        return false;
    }
    if (forceRename) {
        remove(newFileName.c_str());
    }
    return 0 == rename(oldFileName.c_str(), newFileName.c_str());
}
/*********************************************************************/
long Common::calcFileSize(const std::string& filePath) {
    FILE* fp = fopen(filePath.c_str(), "r");
    if (!fp) {
        return -1;
    }
#ifdef _SYSTEM_WINDOWS_
    long size = _filelength(_fileno(fp));
#else
    long size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
#endif
    fclose(fp);
    return size;
}
/*********************************************************************/
std::vector<std::string> Common::stripFileInfo(const std::string& filePath) {
    std::string dirname = "", filename = filePath, basename = "", extname = "";
    size_t pos = filePath.find_last_of("/\\");
    if (pos < filePath.size()) {
        dirname = filePath.substr(0, pos + 1);
        filename = filePath.substr(pos + 1, filePath.size() - 1);
    }
    pos = filename.find_last_of(".");
    if (pos < filename.size()) {
        basename = filename.substr(0, pos);
        extname = filename.substr(pos, filename.size() - 1);
    } else {
        basename = filename;
    }
    std::vector<std::string> infos;
    infos.push_back(dirname);
    infos.push_back(filename);
    infos.push_back(basename);
    infos.push_back(extname);
    return infos;
}
/*********************************************************************/
unsigned char* Common::getFileData(const std::string& filePath, long* fileSize, bool isText) {
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp) {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    *fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (0 == *fileSize) {
        fclose(fp);
        return NULL;
    }
    unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * (isText ? *fileSize + 1 : *fileSize));
    *fileSize = fread(buffer, sizeof(unsigned char), *fileSize, fp);
    if (isText) {
        *(buffer + *fileSize) = '\0';	// set EOF
    }
    fclose(fp);
    return buffer;
}
/*********************************************************************/
std::vector<std::string> Common::getFileDataEx(const std::string& filePath) {
    std::string fileString = "";
    long fileSize = 0;
    unsigned char* fileData = getFileData(filePath, &fileSize);
    if (fileData) {
        fileString = (char*)fileData;
        free(fileData);
    }
    return splitString(fileString, newLineString());
}
/*********************************************************************/
bool Common::writeDataToFile(const unsigned char* data, long dataSize, const std::string& filePath) {
    FILE* fp = fopen(filePath.c_str(), "wb");
    if (!fp) {
        return false;
    }
    if (data && dataSize > 0) {
        fwrite(data, dataSize, sizeof(unsigned char), fp);
        fflush(fp);
    }
    fclose(fp);
    return true;
}
/*********************************************************************/
unsigned char* Common::readFileBytes(FILE* fp, const char* filePath, long offset, unsigned int num) {
    int needClose = 0;
    if (!fp) {
        if (!filePath || 0 == strlen(filePath)) {
            return NULL;
        }
        fp = fopen(filePath, "rb");
        if (!fp) {
            return NULL;
        }
        needClose = 1;
    }
    unsigned char* bytes = (unsigned char*)malloc(sizeof(unsigned char) * num);
    memset(bytes, 0, sizeof(unsigned char) * num);
    fseek(fp, offset, SEEK_SET);
    fread(bytes, sizeof(unsigned char), num, fp);
    if (1 == needClose) {
        fclose(fp);
    }
    return bytes;
}
/*********************************************************************/
int Common::copyFile(const char* srcFilePath, const char* destFilePath) {
    if (!srcFilePath || 0 == strlen(srcFilePath)) {
        return 1;
    }
    if (!destFilePath || 0 == strlen(destFilePath)) {
        return 2;
    }
    if (0 == strcmp(srcFilePath, destFilePath)) {
        return 3;
    }
    FILE* fileSrc = fopen(srcFilePath, "r");
    if (!fileSrc) {
        return 4;
    }
    FILE* fileDest = fopen(destFilePath, "w");
    if (!fileDest){
        fclose(fileSrc);
        return 5;
    }
    char buffer[1024] = { 0 };
    unsigned long len = 0;
    while ((len = fread(buffer, 1, 1024, fileSrc)) > 0) {
        fwrite(buffer, 1, len, fileDest);
    }
    fclose(fileSrc);
    fclose(fileDest);
    return 0;
}
/*********************************************************************/
std::string Common::revisalPath(std::string path) {
    if (path.empty()) {
        return path;
    }
    path = replaceString(path, "\\", "/");
    path = replaceString(path, "//", "/");
    if ('/' == path.at(path.size() - 1)) {
        return path.substr(0, path.size() - 1);
    }
    return path;
}
/*********************************************************************/
bool Common::isAbsolutePath(std::string path, OSType os) {
    switch (os) {
    case OST_WINDOWS: {
            if (path.size() >= 2 && ((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z')) && (':' == path[1])) {
                return true;
            }
        }
        break;
    case OST_ANDORID: {
            /*
             * On Android, there are two situations for full path.
             * 1.Files in APK, e.g. assets/path/path/file.png
             * 2.Files not in APK, e.g. /data/data/org.cocos2dx.hellocpp/cache/path/path/file.png, or /sdcard/path/path/file.png.
             * So these two situations need to be checked on Android.
             */
            if (path.size() >= 1 && ('/' == path[0] || 0 == path.find("assets/"))) {
                return true;
            }
        }
        break;
    case OST_IOS: {
            if (path.size() >= 1 && '/' == path[0]) {
                return true;
            }
        }
        break;
    default:
        break;
    }
    return false;
}
/*********************************************************************/
std::string Common::getFullPath(std::string path, OSType os) {
    if (isAbsolutePath(path, os)) {
        return path;
    }
    /* get current dir */
#ifdef _SYSTEM_WINDOWS_
    char* buffer = _getcwd(NULL, 0);
#else
    char* buffer = getcwd(NULL, 0);
#endif
    if (!buffer) {
        return path;
    }
    std::string currentPath(buffer);
    free(buffer);
    return revisalPath(currentPath + '/' + path);
}
/*********************************************************************/
std::string Common::getCurrentDir(void) {
#ifdef _SYSTEM_WINDOWS_
    char* buffer = _getcwd(NULL, 0);
#else
    char* buffer = getcwd(NULL, 0);
#endif
    if (!buffer) {
        return "";
    }
    std::string currentDir(buffer);
    free(buffer);
    return currentDir + "/";
}
/*********************************************************************/
std::string Common::getParentDir(std::string dir /*= ""*/) {
    if (dir.empty()) {
#ifdef _SYSTEM_WINDOWS_
        char* buffer = _getcwd(NULL, 0);
#else
        char* buffer = getcwd(NULL, 0);
#endif
        if (!buffer) {
            return "";
        }
        dir = buffer;
        free(buffer);
    }
    if (dir.length() > 0 && '/' == dir[dir.length() - 1]) {
        dir.pop_back();
    }
    std::string::size_type pos = dir.find_last_of('/');
    if (std::string::npos == pos) {
        return dir;
    }
    return dir.substr(0, pos + 1);
}
/*********************************************************************/
void Common::traverse(std::string dirName,
                      std::function<void(const std::string& name,
                                         long createTime,
                                         long writeTime,
                                         long accessTime)> folderCallback,
                      std::function<void(const std::string& name,
                                         long createTime,
                                         long writeTime,
                                         long accessTime,
                                         unsigned long size)> fileCallback,
                      bool recursive /*= true*/) {
    dirName = revisalPath(dirName);
#ifdef _SYSTEM_WINDOWS_
    _finddata_t fileData;
    int handle = _findfirst((dirName + "/*.*").c_str(), &fileData);
    if (-1 == handle || !(_A_SUBDIR & fileData.attrib)) {
        return;
    }
    dirName = revisalPath(dirName);
    while (0 == _findnext(handle, &fileData)) {
        if (0 == strcmp(".", fileData.name) || 0 == strcmp("..", fileData.name)) {
            continue;
        }
        std::string subName = dirName + "/" + fileData.name;
        if (_A_SUBDIR & fileData.attrib) {	/* is sub directory */
            if (folderCallback) {
                folderCallback(subName, (long)(fileData.time_create), (long)(fileData.time_write), (long)(fileData.time_access));
            }
        } else {
            if (fileCallback) {
                fileCallback(subName, (long)(fileData.time_create), (long)(fileData.time_write), (long)(fileData.time_access), fileData.size);
            }
        }
        if (recursive) {
            traverse(subName, folderCallback, fileCallback, true);
        }
    }
    _findclose(handle);
#else
    DIR* dir = opendir(dirName.c_str());
    if (!dir) {
        return;
    }
    struct dirent* dirp = NULL;
    while ((dirp = readdir(dir))) {
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name)) {
            continue;
        }
        std::string subName = dirName + "/" + dirp->d_name;
        struct stat subStat;
        if (0 != stat(subName.c_str(), &subStat)) {
            continue;
        }
        DIR* subDir = opendir(subName.c_str());
        if (subDir) {   /* is sub directory */
            closedir(subDir);
            if (folderCallback) {
                folderCallback(subName, subStat.st_ctime, subStat.st_mtime, subStat.st_atime);
            }
        } else {
            if (fileCallback) {
                fileCallback(subName, subStat.st_ctime, subStat.st_mtime, subStat.st_atime, subStat.st_size);
            }
        }
        if (recursive) {
            traverse(subName, folderCallback, fileCallback, true);
        }
    }
    closedir(dir);
#endif
}
/*********************************************************************/
void Common::traverseFile(std::string dirName,
                          const std::vector<std::string>& extList,
                          std::function<void(const std::string& name,
                                             long createTime,
                                             long writeTime,
                                             long accessTime,
                                             unsigned long size)> callback,
                          bool recursive /*= true*/) {
    if (!callback) {
        return;
    }
    traverse(dirName,
             NULL,
             [&](const std::string& name,
             long createTime,
             long writeTime,
             long accessTime,
             unsigned long size)->void {
        if (extList.empty()) {
            callback(name, createTime, writeTime, accessTime, size);
            return;
        }
        std::string::size_type index = name.find_last_of(".");
        if (std::string::npos == index) {
            return;
        }
        std::string ext = name.substr(index, name.size() - index);
        for (size_t i = 0; i < extList.size(); ++i) {
            if (extList[i] == ext) {
                callback(name, createTime, writeTime, accessTime, size);
            }
        }
    }, recursive);
}
/*********************************************************************/
int Common::isIPv4(const char* ip) {
    int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
    char endCh = 0;
    if (!ip || 0 == strlen(ip)) {
        return 0;
    }
    if (4 == sscanf(ip, "%d.%d.%d.%d%c", &p1, &p2, &p3, &p4, &endCh)) {
        if ((p1 >= 0 && p1 <= 255) && (p2 >= 0 && p2 <= 255) && (p3 >= 0 && p3 <= 255) && (p4 >= 0 && p4 <= 255)) {
            return 1;
        }
    }
    return 0;
}
/*********************************************************************/
int Common::isIPv4Inner(const char* ip) {
    if (!ip || 0 == strlen(ip)) {
        return 1;
    }
    if (0 == strcmp("127.0.0.1", ip)) {
        return 0;
    }
    int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
    char endCh = 0;
    if (4 != sscanf(ip, "%d.%d.%d.%d%c", &p1, &p2, &p3, &p4, &endCh)) {
        return 1;
    }
    if ((p1 < 0 || p1 > 255) || (p2 < 0 || p2 > 255) || (p3 < 0 || p3 > 255) || (p4 < 0 || p4 > 255)) {
        return 1;
    }
    unsigned long long ipNum = (unsigned long long)p1 * (256 * 256 * 256) + p2 * (256 * 256) + p3 * (256) + p4;
    /*
     * private IP:
     *      A:  10.0.0.0    - 10.255.255.255
     *      B:  172.16.0.0  - 172.31.255.255
     *      C:  192.168.0.0 - 192.168.255.255
     *      127.0.0.1
     */
    static const unsigned long long A1 = (unsigned long long)10 * (256 * 256 * 256);  /* 10.0.0.0 */
    static const unsigned long long A2 = (unsigned long long)10 * (256 * 256 * 256) + 255 * (256 * 256) + 255 * (256) + 255;  /* 10.255.255.255 */
    static const unsigned long long B1 = (unsigned long long)172 * (256 * 256 * 256) + 16 * (256 * 256);  /* 172.16.0.0 */
    static const unsigned long long B2 = (unsigned long long)172 * (256 * 256 * 256) + 31 * (256 * 256) + 255 * (256) + 255;  /* 172.31.255.255 */
    static const unsigned long long C1 = (unsigned long long)192 * (256 * 256 * 256) + 168 * (256 * 256); /* 192.168.0.0 */
    static const unsigned long long C2 = (unsigned long long)192 * (256 * 256 * 256) + 168 * (256 * 256) + 255 * (256) + 255; /* 192.168.255.255 */
    if ((ipNum >= A1 && ipNum <= A2) || (ipNum >= B1 && ipNum <= B2) || (ipNum >= C1 && ipNum <= C2)) {
        return 0;
    }
    return 2;
}
/*********************************************************************/
std::string Common::calcNetAddress(const char* ip, const char* netmask) {
    if (0 == isIPv4(ip) || 0 == isIPv4(netmask)) {
        return "";
    }
    int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
    sscanf(ip, "%d.%d.%d.%d", &p1, &p2, &p3, &p4);
    int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
    sscanf(netmask, "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
    int v1 = p1 & n1, v2 = p2 & n2, v3 = p3 & n3, v4 = p4 & n4;
    char address[16] = { 0 };
    sprintf(address, "%d.%d.%d.%d", v1, v2, v3, v4);
    return address;
}
/*********************************************************************/
std::string Common::calcHostAddress(const char* ip, const char* netmask) {
    if (0 == isIPv4(ip) || 0 == isIPv4(netmask)) {
        return "";
    }
    int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
    sscanf(ip, "%d.%d.%d.%d", &p1, &p2, &p3, &p4);
    int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
    sscanf(netmask, "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
    int v1 = p1 & (~n1), v2 = p2 & (~n2), v3 = p3 & (~n3), v4 = p4 & (~n4);
    char address[16] = { 0 };
    sprintf(address, "%d.%d.%d.%d", v1, v2, v3, v4);
    return address;
}
/*********************************************************************/
std::string Common::calcBroadcastAddress(const char* ip) {
    if (0 == isIPv4(ip)) {
        return "";
    }
    int p1 = 0, p2 = 0, p3 = 0;
    sscanf(ip, "%d.%d.%d", &p1, &p2, &p3);
    char address[16] = { 0 };
    sprintf(address, "%d.%d.%d.255", p1, p2, p3);
    return address;
}
/*********************************************************************/
double Common::getTime(void) {
#ifdef _SYSTEM_WINDOWS_
    FILETIME ft;
    double t;
    GetSystemTimeAsFileTime(&ft);
    /* Windows file time (time since January 1, 1601 (UTC)) */
    t = ft.dwLowDateTime/1.0e7 + ft.dwHighDateTime*(4294967296.0/1.0e7);
    /* convert to Unix Epoch time (time since January 1, 1970 (UTC)) */
    return (t - 11644473600.0);
#else
    struct timeval v;
    gettimeofday(&v, (struct timezone*)NULL);
    /* Unix Epoch time (time since January 1, 1970 (UTC)) */
    return v.tv_sec + v.tv_usec/1.0e6;
#endif
}
/*********************************************************************/
void Common::getDate(int* year, int* mon, int* mday, int* hour /*= NULL*/, int* min /*= NULL*/, int* sec /*= NULL*/, int* wday /*= NULL*/, int* yday /*= NULL*/) {
    time_t t = time(NULL);
    struct tm* date = localtime(&t);
    if (year) {
        *year = 1900 + date->tm_year;
    }
    if (mon) {
        *mon = 1 + date->tm_mon;
    }
    if (mday) {
        *mday = date->tm_mday;
    }
    if (hour) {
        *hour = date->tm_hour;
    }
    if (min) {
        *min = date->tm_min;
    }
    if (sec) {
        *sec = date->tm_sec;
    }
    if (wday) {
        *wday = 0 == date->tm_wday ? 7 : date->tm_wday;
    }
    if (yday) {
        *yday = 1 + date->tm_yday;
    }
}
/*********************************************************************/
struct tm Common::timeToDate(long seconds /*= 0*/) {
    time_t t = seconds > 0 ? seconds : time(NULL);
    struct tm* date = localtime(&t);
    return *date;
}
/*********************************************************************/
long Common::dateToTime(int year /*= 1970*/, int mon /*= 1*/, int mday /*= 1*/, int hour /*= 8*/, int min /*= 0*/, int sec /*= 0*/) {
    if (year < 1900 || mon > 12 || mon < 1 || mday > 31 || mday < 1 || hour > 23 || hour < 0 || min > 59 || min < 0 || sec > 59 || sec < 0) {
        return 0;
    }
    tm date;
    date.tm_year = year - 1900;
    date.tm_mon = mon - 1;
    date.tm_mday = mday;
    date.tm_hour = hour;
    date.tm_min = min;
    date.tm_sec = sec;
    return (long)mktime(&date);
}
/*********************************************************************/
bool Common::setSystemTime(int year, int mon, int mday, int hour, int min, int sec) {
    if (year < 1900 || mon > 12 || mon < 1 || mday > 31 || mday < 1 || hour > 23 || hour < 0 || min > 59 || min < 0 || sec > 59 || sec < 0) {
        return false;
    }
    tm date;
    date.tm_year = year - 1900;
    date.tm_mon = mon - 1;
    date.tm_mday = mday;
    date.tm_hour = hour;
    date.tm_min = min;
    date.tm_sec = sec;
    time_t t = mktime(&date);
    struct timeval tv;
    tv.tv_sec = t;
    tv.tv_usec = 0;
    return 0 == settimeofday(&tv, NULL);
}
/*********************************************************************/
void Common::sleepSeconds(unsigned long seconds) {
    int ret;
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    do {
#ifdef _SYSTEM_WINDOWS_
        ret = 0;
#else
        ret = select(0, 0, 0, 0, &tv);
#endif
        if (tv.tv_sec <= 0 && tv.tv_usec <= 0) {
            break;
        }
    } while (ret < 0 && EINTR == errno);
}
/*********************************************************************/
void Common::sleepMilliseconds(unsigned long milliseconds) {
    int ret;
    struct timeval tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    do {
#ifdef _SYSTEM_WINDOWS_
        ret = 0;
#else
        ret = select(0, 0, 0, 0, &tv);
#endif
        if (tv.tv_sec <= 0 && tv.tv_usec <= 0) {
            break;
        }
    } while (ret < 0 && EINTR == errno);
}
/*********************************************************************/
void Common::sleepMicroseconds(unsigned long microseconds) {
    int ret;
    struct timeval tv;
    tv.tv_sec = microseconds / 1000000;
    tv.tv_usec = microseconds % 1000000;
    do {
#ifdef _SYSTEM_WINDOWS_
        ret = 0;
#else
        ret = select(0, 0, 0, 0, &tv);
#endif
        if (tv.tv_sec <= 0 && tv.tv_usec <= 0) {
            break;
        }
    } while (ret < 0 && EINTR == errno);
}
/*********************************************************************/
int Common::tryToLockFile(int fd, int lock) {
#ifdef _SYSTEM_WINDOWS_
    return 0;
#else
    struct flock fl;
    fl.l_type = (1 == lock) ? F_WRLCK : F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl);
#endif
}
/*********************************************************************/
int Common::startProcessByName(const char* filename, const char* procName) {
#ifdef _SYSTEM_WINDOWS_
    return 0;
#else
    if (!filename || 0 == strlen(filename)) {
        return 0;
    }
    if (!procName || 0 == strlen(procName)) {
        return 0;
    }
    if (0 != access(filename, F_OK | R_OK | X_OK)) {                           /* 文件必须具有可读,可执行权限 */
        return 0;
    }
    pid_t firstPid = fork();                                                   /* 创建一代子进程 */
    if (firstPid < 0) {                                                        /* 一代子进程创建失败 */
        return 0;
    } else if (0 == firstPid) {                                                /* 创建成功,此处是一代子进程的代码 */
        pid_t secondPid = fork();                                              /* 创建二代孙进程 */
        if (secondPid < 0) {                                                   /* 二代孙进程创建失败 */
            return 0;
        } else if (0 == secondPid) {                                           /* 创建成功,此处是二代孙进程的代码 */
            if (-1 != execl(filename, procName, (char*)NULL)) {                /* 在子进程中执行该程序 */
                return 1;                                                      /* 执行完毕直接退出 */
            }
            return 0;
        }
        exit(1);                                                               /* 创建成功,此处是一代子进程的代码 */
    } else {                                                                   /* 创建成功,此处是父进程的代码 */
        if (waitpid(firstPid, NULL, 0) != firstPid) {                          /* 父进程必须为一代子进程收尸 */
        }
    }
    return 1;
#endif
}
/*********************************************************************/
int Common::searchProcessByName(const char* filename, void(*callback)(const char* fullPath, int pid)) {
#ifdef _SYSTEM_WINDOWS_
    return 0;
#else
    unsigned int nameNum = strlen(filename);
    if (!filename || 0 == nameNum) {
        return 0;
    }
    DIR* dir = opendir("/proc");
    if (!dir) {
        return 0;
    }
    char exePath[64];
    char fullPath[256];
    unsigned int pathNum;
    int matchCount = 0;
    struct dirent* dirp = NULL;
    while ((dirp = readdir(dir))) {
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name) || 0 == atoi(dirp->d_name)) {
            continue;
        }
        if (DT_DIR != dirp->d_type) {
            continue;
        }
        memset(exePath, 0, sizeof(exePath));
        sprintf(exePath, "/proc/%s/exe", dirp->d_name);
        pathNum = readlink(exePath, fullPath, sizeof(fullPath) - 1);
        if (pathNum <= 0 || pathNum >= sizeof(fullPath) - 1) {
            continue;
        }
        fullPath[pathNum] = '\0';
        if (nameNum > pathNum) {
            continue;
        }
        int matched = 1;
        for (unsigned int i = 1; i <= nameNum; ++i) {
            if (fullPath[pathNum - i] != filename[nameNum - i]) {
                matched = 0;
                break;
            }
        }
        if (matched) {
            ++matchCount;
            if (callback) {
                callback(fullPath, atoi(dirp->d_name));
            }
        }
    }
    closedir(dir);
    return matchCount;
#endif
}
/*********************************************************************/
unsigned long long Common::generateUID(void) {
    static int sY = 0, sM = 0, sD = 0, sH = 0, sMM = 0, sS = 0, sIdx = 1;
    time_t t = time(nullptr);
    struct tm* dt = localtime(&t);
    int year = 1900 + dt->tm_year, mon = 1 + dt->tm_mon, mday = dt->tm_mday, hour = dt->tm_hour, min = dt->tm_min, sec = dt->tm_sec;
    if (year == sY && mon == sM && mday == sD && hour == sH && min == sMM && sec == sS) {
        sIdx++;
    } else {
        sY = year; sM = mon; sD = mday; sH = hour; sMM = min; sS = sec; sIdx = 1;
    }
    return (unsigned long long)sY * 10000000000000 + (unsigned long long)sM * 100000000000 + (unsigned long long)sD * 1000000000 + 
           (unsigned long long)sH * 10000000 + (unsigned long long)sMM * 100000 + (unsigned long long)sS * 1000 + (unsigned long long)sIdx;
}
/*********************************************************************/
std::string Common::generateFilename(const std::string& extname) {
    char filename[64] = { 0 };
    sprintf(filename, "%llu%s", generateUID(), extname.empty() ? "" : ('.' == extname.at(0) ? extname.c_str() : ('.' + extname).c_str()));
    return filename;
}
/*********************************************************************/
std::vector<std::string> Common::shellCmd(const std::string& cmd, unsigned int sleepMillisecondWhenOk) {
    std::vector<std::string> results;
    if (cmd.empty()) {
        return results;
    }
    FILE* stream = NULL;
#ifdef _SYSTEM_WINDOWS_
    stream = _popen(cmd.c_str(), "r");
#else
    stream = popen(cmd.c_str(), "r");
#endif
    if (!stream) {
        return results;
    }
    char line[1024] = { 0 };
    while (memset(line, 0, sizeof(line)) && fgets(line, sizeof(line) - 1, stream)) {
        line[strlen(line) - 1] = '\0';
        if (strlen(line) > 0) {
            results.push_back(line);
        }
    }
#ifdef _SYSTEM_WINDOWS_
    _pclose(stream);
#else
    pclose(stream);
#endif
    if (results.empty() && sleepMillisecondWhenOk > 0) {
        usleep(sleepMillisecondWhenOk * 1000);
    }
    return results;
}
/*********************************************************************/
int Common::randomNetPort(void) {
    static const int PORT_START = 1025;
    static const int PORT_END = 65534;
    time_t tm;
    srand((unsigned int)time(&tm));
    return (rand() % (PORT_END - PORT_START + 1) + PORT_START);
}
/*********************************************************************/
