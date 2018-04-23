/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile helper
**********************************************************************/
#include "logfile_helper.h"
#include <stdlib.h>

#define EXT     ".log"

logfile_st* sLogFile = NULL;
char* sBaseName = NULL;
long sMaxSize = 0;
int sCount = 1;

void logfilehelper_init(const char* prefix, long maxSize) {
    if (sLogFile || maxSize <= 0) {
        return;
    }
    struct timeval tv;
    struct tm tm;
    char* buf;
    sBaseName = malloc((prefix ? strlen(prefix) : 0) + 16);
    gettimeofday(&tv, NULL);
    tm = *localtime(&tv.tv_sec);
    sprintf(sBaseName, "%s%04d%02d%02d_%02d%02d%02d", prefix ? prefix : "", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    buf = malloc(strlen(sBaseName) + strlen(EXT));
    sprintf(buf, "%s%s", sBaseName, EXT);
    sLogFile = logfile_open(buf, maxSize);
    sMaxSize = maxSize;
    ++sCount;
    free(buf);
}

int logfilehelper_isenable(void) {
    return logfile_isenable(sLogFile);
}

void logfilehelper_enable(int enable) {
    logfile_enable(sLogFile, enable);
}

void logfilehelper_record(const char* tag, int withtime, const char* content) {
    if (!sLogFile) {
        return;
    }
    int flag = 0;
    char* buf;
    if (!tag || 0 == strlen(tag)) {
        if (withtime) {
            flag = logfile_record_with_time(sLogFile, content);
        } else {
            flag = logfile_record(sLogFile, content, 0);
        }
    } else {
        flag = logfile_record_with_tag(sLogFile, tag, withtime, content);
    }
    if (5 == flag) {
        logfile_close(sLogFile);
        buf = malloc(strlen(sBaseName) + 4 + strlen(EXT));
        sprintf(buf, "%s-%03d%s", sBaseName, sCount, EXT);
        sLogFile = logfile_open(buf, sMaxSize);
        if (sLogFile) {
            free(sBaseName);
            sBaseName = NULL;
            sMaxSize = 0;
            sCount = 1;
            return;
        }
        ++sCount;
        if (!tag || 0 == strlen(tag)) {
            if (withtime) {
                logfile_record_with_time(sLogFile, content);
            } else {
                logfile_record(sLogFile, content, 0);
            }
        } else {
            logfile_record_with_tag(sLogFile, tag, withtime, content);
        }
    }
}
