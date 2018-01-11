/******************************************************************************
* Author: jaron.ho
* Date:   2012-09-06
* Brief:  common functions
******************************************************************************/
#if defined(win32) || defined(_win32) || defined(WIN32) || defined(_WIN32) || \
    defined(win64) || defined(_win64) || defined(WIN64) || defined(_WIN64)
    #define _SYSTEM_WINDOWS_
#endif
#include "Common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <locale.h>
#include <algorithm>
#ifdef _SYSTEM_WINDOWS_
#include <direct.h>
#include <windows.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#endif
//--------------------------------------------------------------------------
bool Common::isDigit(char c) {
    return c >= '0' && c <= '9';
}
//--------------------------------------------------------------------------
bool Common::isDigit(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    bool mask = false;
    int decimal = 0;
    for (size_t i=0; i<str.size(); ++i) {
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
//--------------------------------------------------------------------------
int Common::toInt(const std::string& str) {
    return atoi(str.c_str());
}
//--------------------------------------------------------------------------
long Common::toLong(const std::string& str) {
    return atol(str.c_str());
}
//--------------------------------------------------------------------------
long long Common::toLongLong(const std::string& str) {
    return _atoi64(str.c_str());
}
//--------------------------------------------------------------------------
double Common::toDouble(const std::string& str) {
    return atof(str.c_str());
}
//--------------------------------------------------------------------------
std::string Common::toString(short u) {
    char buf[8];
    sprintf(buf, "%d", u);
    return buf;
}
//--------------------------------------------------------------------------
std::string Common::toString(int n) {
    char buf[16];
    sprintf(buf, "%d", n);
    return buf;
}
//--------------------------------------------------------------------------
std::string Common::toString(long l) {
    char buf[32];
    sprintf(buf, "%ld", l);
    return buf;
}
//--------------------------------------------------------------------------
std::string Common::toString(long long ll) {
    char buf[64];
    sprintf(buf, "%lld", ll);
    return buf;
}
//--------------------------------------------------------------------------
std::string Common::toString(float f) {
    char buf[64];
    sprintf(buf, "%f", f);
    return buf;
}
//--------------------------------------------------------------------------
std::string Common::toString(double d) {
    char buf[128];
    sprintf(buf, "%f", d);
    return buf;
}
//--------------------------------------------------------------------------
std::string Common::toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}
//--------------------------------------------------------------------------
std::string Common::toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}
//--------------------------------------------------------------------------
std::string Common::newLineString(void) {
    return std::string("\r\n");
}
//--------------------------------------------------------------------------
std::string Common::stringOfChar(unsigned int n, char ch) {
    return std::string(n, ch);
}
//--------------------------------------------------------------------------
std::string Common::limitStringLength(std::string str, size_t len, const std::string& sufStr, const std::string& defStr) {
    if (str.empty()) {
        return defStr;
    }
    if (str.size() > len) {
        return str.substr(0, len) + sufStr;
    }
    return str;
}
//--------------------------------------------------------------------------
std::wstring Common::limitWstringLength(std::wstring wstr, size_t len, const std::wstring& sufWstr, const std::wstring& defWstr) {
    if (wstr.empty()) {
        return defWstr;
    }
    if (wstr.size() > len) {
        return wstr.substr(0, len) + sufWstr;
    }
    return wstr;
}
//--------------------------------------------------------------------------
std::vector<std::string> Common::splitString(std::string str, const std::string& pattern) {
    std::vector<std::string> result;
    if (str.empty() || pattern.empty()) {
        return result;
    }
    str.append(pattern);
    std::string::size_type pos;
    for (size_t i=0; i<str.size(); ++i) {
        pos = str.find(pattern, i);
        if (pos < str.size()) {
            result.push_back(str.substr(i, pos - i));
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}
//--------------------------------------------------------------------------
std::string Common::replaceString(std::string str, const std::string& src, const std::string& dest) {
    if (str.empty() || src.empty()) {
        return str;
    }
    std::string::size_type pos = 0;
    while (std::string::npos != (pos = str.find(src, pos))) {
        str.replace(pos, src.size(), dest);
        pos += dest.size();
    }
    return str;
}
//--------------------------------------------------------------------------
std::string Common::formatString(const char* format, ...) {
    char buf[512];
    memset(buf, 0, sizeof(buf));

    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    return buf;
}
//--------------------------------------------------------------------------
std::string Common::wstring2string(const std::wstring& wstr, const char* locale) {
    if (wstr.empty()) {
        return "";
    }
    std::string curLocale = setlocale(LC_ALL, NULL);
    char* localeName = setlocale(LC_ALL, locale);
    if (NULL == localeName) {
        setlocale(LC_ALL, curLocale.c_str());
        return "";
    }
    const wchar_t* src = wstr.c_str();
    size_t destSize = 2 * wstr.size() + 1;
    char* dest = new char[destSize];
    memset(dest, 0, destSize);
    wcstombs(dest, src, destSize);
    std::string result = dest;
    delete []dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}
//--------------------------------------------------------------------------
std::wstring Common::string2wstring(const std::string& str, const char* locale) {
    if (str.empty()) {
        return L"";
    }
    std::string curLocale = setlocale(LC_ALL, NULL);
    char* localeName = setlocale(LC_ALL, locale);
    if (NULL == localeName) {
        setlocale(LC_ALL, curLocale.c_str());
        return L"";
    }
    const char* src = str.c_str();
    size_t destSize = str.size() + 1;
    wchar_t* dest = new wchar_t[destSize];
    wmemset(dest, 0, destSize);
    mbstowcs(dest, src, destSize);
    std::wstring result = dest;
    delete []dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}
//--------------------------------------------------------------------------
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
//--------------------------------------------------------------------------
bool Common::createDir(const std::string& dirName) {
#ifdef _SYSTEM_WINDOWS_
    // method 1:
    return 0 == _mkdir(dirName.c_str());
    // method 2:
    /*if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(dirName.c_str())) {
        CreateDirectoryA(dirName.c_str(), 0);
        return true;
    }
    return false;*/
#else
    return 0 == mkdir(dirName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
}
//--------------------------------------------------------------------------
void Common::removeDir(const std::string& dirName) {
#ifdef _SYSTEM_WINDOWS_
    // method 1:
    /*struct _finddata_t fileData;
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
    rmdir(dirName.c_str());*/
    // method 2:
    std::string command = "rd /s /q \"" + dirName + "\"";
    system(command.c_str());
#else
    DIR* dir = opendir(dirName.c_str());
    if (NULL == dir) {
        return;
    }
    struct dirent* dirp = NULL;
    while (NULL != (dirp = readdir(dir))) {
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name)) {
            continue;
        }
        std::string subName = dirName + "/" + dirp->d_name;
        DIR* subDir = opendir(subName.c_str());
        if (NULL == subDir) {
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
//--------------------------------------------------------------------------
bool Common::existFile(const std::string& filePath) {
    FILE* fp = fopen(filePath.c_str(), "r");
    if (NULL == fp) {
        return false;
    }
    fclose(fp);
    return true;
}
//--------------------------------------------------------------------------
bool Common::createFile(const std::string& filePath) {
    FILE* fp = fopen(filePath.c_str(), "wb");
    if (NULL == fp) {
        return false;
    }
    fclose(fp);
    return true;
}
//--------------------------------------------------------------------------
bool Common::removeFile(const std::string& filePath) {
    return 0 == remove(filePath.c_str());
}
//--------------------------------------------------------------------------
bool Common::renameFile(const std::string& oldFileName, const std::string& newFileName, bool forceRename /*= false*/) {
    if (oldFileName.empty() || newFileName.empty()) {
        return false;
    }
    if (forceRename) {
        remove(newFileName.c_str());
    }
    return 0 == rename(oldFileName.c_str(), newFileName.c_str());
}
//--------------------------------------------------------------------------
long Common::calcFileSize(const std::string& filePath) {
    FILE* fp = fopen(filePath.c_str(), "r");
    if (NULL == fp) {
        return -1;
    }
#ifdef _SYSTEM_WINDOWS_
    long size = _filelength(_fileno(fp));
#else
    long size = filelength(fileno(fp));
#endif
    fclose(fp);
    return size;
}
//--------------------------------------------------------------------------
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
    }
    std::vector<std::string> infos;
    infos.push_back(dirname);
    infos.push_back(filename);
    infos.push_back(basename);
    infos.push_back(extname);
    return infos;
}
//--------------------------------------------------------------------------
unsigned char* Common::getFileData(const std::string& filePath, long* fileSize, bool isText) {
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (NULL == fp) {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    *fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (0 == *fileSize) {
        return NULL;
    }
    unsigned char* buffer = new unsigned char[isText ? *fileSize + 1 : *fileSize];
    *fileSize = fread(buffer, sizeof(unsigned char), *fileSize, fp);
    if (isText) {
        *(buffer + *fileSize) = '\0';	// set EOF
    }
    fclose(fp);
    return buffer;
}
//--------------------------------------------------------------------------
std::vector<std::string> Common::getFileDataEx(const std::string& filePath) {
    std::string fileString = "";
    long fileSize = 0;
    unsigned char* fileData = getFileData(filePath, &fileSize);
    if (fileData) {
        fileString = (char*)fileData;
        delete []fileData;
    }
    return splitString(fileString, newLineString());
}
//--------------------------------------------------------------------------
bool Common::writeDataToFile(const unsigned char* data, long dataSize, const std::string& filePath) {
    if (NULL == data || 0 == dataSize) {
        return false;
    }
    FILE* fp = fopen(filePath.c_str(), "wb");
    if (NULL == fp) {
        return false;
    }
    fwrite(data, dataSize, sizeof(unsigned char), fp);
    fflush(fp);
    fclose(fp);
    return true;
}
//--------------------------------------------------------------------------
bool Common::copyFile(const std::string& srcFilePath, const std::string& destFilePath) {
    if (srcFilePath.empty() || destFilePath.empty() || srcFilePath == destFilePath) {
        return false;
    }
    long fileSize = 0;
    unsigned char* fileData = getFileData(srcFilePath, &fileSize);
    if (0 == fileSize || NULL == fileData) {
        return false;
    }
    return writeDataToFile(fileData, fileSize, destFilePath);
}
//--------------------------------------------------------------------------
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
//--------------------------------------------------------------------------
bool Common::isAbsolutePath(std::string path, OSType os) {
    switch (os) {
    case OST_WINDOWS: {
            if ((path.size() >= 2) && ((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z')) && (':' == path[1])) {
                return true;
            }
        }
        break;
    case OST_ANDORID: {
            // On Android, there are two situations for full path.
            // 1.Files in APK, e.g. assets/path/path/file.png
            // 2.Files not in APK, e.g. /data/data/org.cocos2dx.hellocpp/cache/path/path/file.png, or /sdcard/path/path/file.png.
            // So these two situations need to be checked on Android.
            if ((path.size() >= 1) && ('/' == path[0] || 0 == path.find("assets/"))) {
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
//--------------------------------------------------------------------------
std::string Common::getFullPath(std::string path, OSType os) {
    if (isAbsolutePath(path, os)) {
        return path;
    }
    // get current dir
#ifdef _SYSTEM_WINDOWS_
    char* buffer = _getcwd(NULL, 0);
#else
    char* buffer = getcwd(NULL, 0);
#endif
    if (NULL == buffer) {
        return path;
    }
    std::string currentPath(buffer);
    free(buffer);
    return revisalPath(currentPath + '/' + path);
}
//--------------------------------------------------------------------------
void Common::searchFile(std::string dirName, const std::vector<std::string>& extList, std::vector<std::string>& fileList, bool recursive /*= true*/) {
#ifdef _SYSTEM_WINDOWS_
    _finddata_t fileData;
    int handle = _findfirst((dirName + "/*.*").c_str(), &fileData);
    if (-1 == handle || !(_A_SUBDIR & fileData.attrib)) {
        return;
    }
    dirName = revisalPath(dirName);
    while (0  == _findnext(handle, &fileData)) {
        if (0 == strcmp(".", fileData.name) || 0 == strcmp("..", fileData.name)) {
            continue;
        }
        std::string subName = dirName + "/" + fileData.name;
        // is sub directory
        if (_A_SUBDIR & fileData.attrib) {
            if (recursive) {
                searchFile(subName, extList, fileList, true);
            }
            continue;
        }
        // all file type
        if (extList.empty()) {
            fileList.push_back(subName);
            continue;
        }
        // specific file type
        std::string::size_type index = subName.find_last_of(".");
        if (std::string::npos == index) {
            continue;
        }
        std::string ext = subName.substr(index, subName.size() - index);
        for (size_t i=0; i<extList.size(); ++i) {
            if (extList[i] == ext) {
                fileList.push_back(subName);
            }
        }
    }
    _findclose(handle);
#else
    DIR* dir = opendir(dirName.c_str());
    if (NULL == dir) {
        return;
    }
    dirName = revisalPath(dirName);
    struct dirent* dirp = NULL;
    while (NULL != (dirp = readdir(dir))) {
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name)) {
            continue;
        }
        std::string subName = dirName + "/" + dirp->d_name;
        DIR* subDir = opendir(subName.c_str());
        if (NULL == subDir) {
            if (extList.empty()) {
                fileList.push_back(subName);
                continue;
            }
            std::string::size_type index = subName.find_last_of(".");
            if (std::string::npos == index) {
                continue;
            }
            std::string ext = subName.substr(index, subName.size() - index);
            for (size_t i=0; i<extList.size(); ++i) {
                if (extList[i] == ext) {
                    fileList.push_back(subName);
                }
            }
            continue;
        }
        closedir(subDir);
        subDir = NULL;
        if (recursive) {
            searchFile(subName, extList, fileList, true);
        }
    }
    closedir(dir);
#endif
}
//--------------------------------------------------------------------------
void Common::searchDir(std::string dirName, std::vector<std::string>& dirList, bool recursive /*= true*/) {
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
        if (_A_SUBDIR & fileData.attrib) {	// is sub directory
            std::string subDirName = dirName + "/" + fileData.name;
            dirList.push_back(subDirName);
            if (recursive) {
                searchDir(subDirName, dirList, true);
            }
        }
    }
    _findclose(handle);
#else
    DIR* dir = opendir(dirName.c_str());
    if (NULL == dir) {
        return;
    }
    dirName = revisalPath(dirName);
    struct dirent* dirp = NULL;
    while (NULL != (dirp = readdir(dir))) {
        if (0 == strcmp(".", dirp->d_name) || 0 == strcmp("..", dirp->d_name)) {
            continue;
        }
        std::string subDirName = dirName + "/" + dirp->d_name;
        DIR* subDir = opendir(subDirName.c_str());
        if (NULL == subDir) {
            continue;
        }
        closedir(subDir);
        subDir = NULL;
        dirList.push_back(subDirName);
        if (recursive) {
            searchDir(subDirName, dirList, true);
        }
    }
    closedir(dir);
#endif
}
//--------------------------------------------------------------------------
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
//--------------------------------------------------------------------------
struct tm Common::timeToDate(long seconds /*= 0*/) {
    time_t nowtime = seconds > 0 ? seconds : time(NULL);
    struct tm* timeinfo = localtime(&nowtime);
    timeinfo->tm_year += 1900;      /* [1900, ...) */
    timeinfo->tm_mon += 1;          /* [1, 12] */
    if (0 == timeinfo->tm_wday) {   /* [1, 7] */
        timeinfo->tm_wday = 7;
    }
    timeinfo->tm_yday += 1;         /* [1, 366] */
    return *timeinfo;
}
//--------------------------------------------------------------------------
long Common::dateToTime(int y /*= 1970*/, int m /*= 1*/, int d /*= 1*/, int h /*= 8*/, int n /*= 0*/, int s /*= 0*/) {
    if (y < 1900 || m > 12 || m < 1 || d > 31 || d < 1 || h > 23 || h < 0 || n > 59 || n < 0 || s > 59 || s < 0) {
        return 0;
    }
    tm date;
    date.tm_year = y - 1900;
    date.tm_mon = m - 1;
    date.tm_mday = d;
    date.tm_hour = h;
    date.tm_min = n;
    date.tm_sec = s;
    return (long)mktime(&date);
}
//--------------------------------------------------------------------------
