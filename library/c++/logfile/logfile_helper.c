/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile helper
**********************************************************************/
#include "logfile_helper.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

logrecord_st* logfilehelper_init(const char* basename, const char* extname, unsigned int maxSize, unsigned int override) {
    if (!basename || 0 == strlen(basename) || !extname || 0 == strlen(extname) || maxSize <= 0) {
        return NULL;
    }
    char* filename = NULL;
    logrecord_st* logrecord = (logrecord_st*)malloc(sizeof(logrecord_st));
    logrecord->basename = malloc(strlen(basename));
    sprintf(logrecord->basename, "%s", basename);
    if (strstr(extname, ".")) {
        logrecord->extname = malloc(strlen(extname));
        sprintf(logrecord->extname, "%s", extname);
        filename = malloc(strlen(basename) + strlen(extname));
        sprintf(filename, "%s%s", basename, extname);
    } else {
        logrecord->extname = malloc(strlen(extname) + 1);
        sprintf(logrecord->extname, ".%s", extname);
        filename = malloc(strlen(basename) + strlen(extname) + 1);
        sprintf(filename, "%s.%s", basename, extname);
    }
    logrecord->logfile = logfile_open(filename, maxSize);
    free(filename);
    if (!logrecord->logfile) {
        free(logrecord);
        return NULL;
    }
    logrecord->override = override;
    logrecord->count = 1;
    return logrecord;
}

unsigned int logfilehelper_isenable(logrecord_st* logrecord) {
    if (logrecord) {
        return logfile_isenable(logrecord->logfile);
    }
    return 0;
}

void logfilehelper_enable(logrecord_st* logrecord, unsigned int enable) {
    if (logrecord) {
        logfile_enable(logrecord->logfile, enable);
    }
}

unsigned int logfilehelper_record(logrecord_st* logrecord, const char* tag, unsigned int withtime, const char* content) {
    if (!logrecord || !logrecord->logfile) {
        return 1;
    }
    if (!content || 0 == strlen(content)) {
        return 2;
    }
    unsigned int flag = 0;
    char* filename = NULL;
    if (!tag || 0 == strlen(tag)) {
        if (withtime) {
            flag = logfile_record_with_time(logrecord->logfile, content);
        } else {
            flag = logfile_record(logrecord->logfile, content, 0);
        }
    } else {
        flag = logfile_record_with_tag(logrecord->logfile, tag, withtime, content);
    }
    if (5 == flag) {
        if (logrecord->override) {
            logfile_clear(logrecord->logfile);
        } else {
            logfile_close(logrecord->logfile);
            if (strstr(logrecord->extname, ".")) {
                filename = malloc(strlen(logrecord->basename) + strlen(logrecord->extname));
                sprintf(filename, "%s-%03d%s", logrecord->basename, logrecord->count + 1, logrecord->extname);
            } else {
                filename = malloc(strlen(logrecord->basename) + strlen(logrecord->extname) + 1);
                sprintf(filename, "%s-%03d.%s", logrecord->basename, logrecord->count + 1, logrecord->extname);
            }
            logrecord->logfile = logfile_open(filename, logrecord->logfile->maxsize);
            free(filename);
            if (!logrecord->logfile) {
                free(logrecord->basename);
                logrecord->basename = NULL;
                free(logrecord->extname);
                logrecord->extname = NULL;
                free(logrecord);
                logrecord = NULL;
                return 3;
            }
        }
        ++logrecord->count;
        if (!tag || 0 == strlen(tag)) {
            if (withtime) {
                logfile_record_with_time(logrecord->logfile, content);
            } else {
                logfile_record(logrecord->logfile, content, 0);
            }
        } else {
            logfile_record_with_tag(logrecord->logfile, tag, withtime, content);
        }
    }
    return 0;
}
