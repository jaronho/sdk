/**********************************************************************
* Author:	jaron.ho
* Date:		2019-08-25
* Brief:	日志模块
**********************************************************************/
#include "Logger.h"
#include <fstream>
#include <iostream>
#include <mutex>
#ifdef __linux
#include <syslog.h>
#endif

void writeLog(const std::string& path, const std::string& filename, const std::string& content) {
    std::cout << content << std::endl;
    static std::mutex sMutex;
    static std::ofstream sFstream;
    static std::string sFullFilePath;
    sMutex.lock();
    std::string fullFilePath = path + filename;
    if (sFullFilePath != fullFilePath) {
        if (sFstream.is_open()) {
            sFstream.close();
        }
        sFstream.open(fullFilePath, std::ios_base::out | std::ios_base::app);
    }
    if (sFstream.is_open()) {
        sFstream << content << std::endl;
        sFstream.flush();
    }
    sMutex.unlock();
}

Logger::Logger(void) {
    mSysLogOpened = false;
    mPath = "";
}

Logger::~Logger(void) {
    if (mSysLogOpened) {
#ifdef __linux
        closelog();
#endif
    }
}

Logger* Logger::getInstance(void) {
    static Logger* instance = nullptr;
    if (nullptr == instance) {
        instance = new Logger();
    }
    return instance;
}

std::string Logger::header(std::string& filename, const std::string& tag, bool withTag, bool withTime) {
    time_t now;
    time(&now);
    struct tm t = *localtime(&now);
    char file[13] = { 0 };
    strftime(file, sizeof(file), "%Y%m%d.log", &t);
    char date[22] = { 0 };
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &t);
    std::string hdr;
    if (withTime) {
        hdr += "[";
        hdr += date;
        hdr += "]";
    }
    if (withTag) {
        hdr += "[";
        hdr += tag;
        hdr += "]";
    }
    filename = file;
    return hdr;
}

void Logger::openSyslog(const std::string& ident) {
    if (mSysLogOpened) {
        return;
    }
    mSysLogOpened = true;
#ifdef __linux
    openlog(ident.c_str(), LOG_PID | LOG_CONS, LOG_USER);
#endif
}

void Logger::writeSyslog(const std::string& content) {
    if (!mSysLogOpened) {
        return;
    }
#ifdef __linux
    syslog(LOG_DEBUG, "%s", content.c_str());
#endif
}

void Logger::setPath(const std::string& path) {
    mPath = path;
}

void Logger::setFilename(const std::string& filename) {
    mFilename = filename;
}

std::string Logger::print(const std::string& content, const std::string& tag, bool withTag, bool withTime) {
    std::string filename;
    std::string hdr = header(filename, tag, withTag, withTime);
    writeLog(mPath, mFilename.empty() ? filename : mFilename, hdr + " " + content);
    return hdr;
}

std::string Logger::printSyslog(const std::string& content, const std::string& tag) {
    std::string filename;
    std::string hdr = header(filename, tag, true, true);
    std::string buff = hdr + " " + content;
    writeLog(mPath, mFilename.empty() ? filename : mFilename, buff);
    writeSyslog(tag + " " + content);
    return hdr;
}
