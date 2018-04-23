/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile helper
**********************************************************************/
#include "logfile_helper.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define EXT     ".log"

logfile_st* sLogFile = NULL;
char* sFilename = NULL;
unsigned int sOverride = 0;
unsigned int sMaxSize = 0;
unsigned int sCount = 1;

void logfilehelper_init(const char* name, unsigned int maxSize) {
    if (!name || 0 == strlen(name) || maxSize <= 0 || sLogFile) {
        return;
    }
    time_t now;
    struct tm t;
    char date[32] = { 0 };
    char* buf = NULL;
    if (strstr(name, ".")) {
        sFilename = malloc(strlen(name));
        sprintf(sFilename, "%s", name);
        sLogFile = logfile_open(sFilename, maxSize);
        sOverride = 1;
    } else {
        time(&now);
        t = *localtime(&now);
        strftime(date, sizeof(date), "%Y%m%d_%H%M%S", &t);
        sFilename = malloc(strlen(name) + strlen(date));
        sprintf(sFilename, "%s%s", name, date);
        buf = malloc(strlen(sFilename) + strlen(EXT));
        sprintf(buf, "%s%s", sFilename, EXT);
        sLogFile = logfile_open(buf, maxSize);
        free(buf);
        sOverride = 0;
    }
    sMaxSize = maxSize;
    ++sCount;
}

unsigned int logfilehelper_isenable(void) {
    return logfile_isenable(sLogFile);
}

void logfilehelper_enable(unsigned int enable) {
    logfile_enable(sLogFile, enable);
}

void logfilehelper_record(const char* tag, unsigned int withtime, const char* content) {
    if (!sLogFile) {
        return;
    }
    unsigned int flag = 0;
    char* buf = NULL;
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
        if (sOverride) {
            logfile_clear(sLogFile);
        } else {
            logfile_close(sLogFile);
            buf = malloc(strlen(sFilename) + 4 + strlen(EXT));
            sprintf(buf, "%s-%03d%s", sFilename, sCount, EXT);
            sLogFile = logfile_open(buf, sMaxSize);
            free(buf);
            if (!sLogFile) {
                free(sFilename);
                sFilename = NULL;
                sOverride = 0;
                sMaxSize = 0;
                sCount = 1;
                return;
            }
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
