/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile wrapper
**********************************************************************/
#include "logfilewrapper.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

logfilewrapper_st* logfilewrapper_open(const char* basename, const char* extname, unsigned long maxSize, unsigned int override) {
    char* filename = NULL;
    logfilewrapper_st* wrapper = NULL;
    assert(basename && strlen(basename) > 0);
    assert(extname && strlen(extname) > 0);
    assert(maxSize > 0);
    if (strstr(extname, ".")) {
        filename = (char*)malloc(strlen(basename) + strlen(extname) + 1);
        sprintf(filename, "%s%s", basename, extname);
    } else {
        filename = (char*)malloc(strlen(basename) + 1 + strlen(extname) + 1);
        sprintf(filename, "%s.%s", basename, extname);
    }
    wrapper = (logfilewrapper_st*)malloc(sizeof(logfilewrapper_st));
    wrapper->logfile = logfile_open(filename, maxSize);
    free(filename);
    if (!wrapper->logfile) {
        free(wrapper);
        return NULL;
    }
    wrapper->basename = basename;
    wrapper->extname = extname;
    wrapper->override = override;
    return wrapper;
}

void logfilewrapper_close(logfilewrapper_st* wrapper) {
    assert(wrapper);
    logfile_close(wrapper->logfile);
    wrapper->logfile = NULL;
    free(wrapper);
    wrapper = NULL;
}

unsigned int logfilewrapper_isenable(logfilewrapper_st* wrapper) {
    assert(wrapper);
    return logfile_isenable(wrapper->logfile);
}

void logfilewrapper_enable(logfilewrapper_st* wrapper, unsigned int enable) {
    assert(wrapper);
    logfile_enable(wrapper->logfile, enable);
}

unsigned int logfilewrapper_record(logfilewrapper_st* wrapper, const char* tag, unsigned int withtime, const char* content) {
    unsigned int flag = 0;
    char* filename = NULL;
    unsigned long maxsize = 0;
    time_t now;
    struct tm t;
    char date[16] = { 0 };
    char* oldFilename = NULL;
    assert(wrapper);
    assert(content);
    if (!tag || 0 == strlen(tag)) {
        if (withtime) {
            flag = logfile_record_with_time(wrapper->logfile, content);
        } else {
            flag = logfile_record(wrapper->logfile, content, 0);
        }
    } else {
        flag = logfile_record_with_tag(wrapper->logfile, tag, withtime, content);
    }
    if (6 == flag) {
        if (wrapper->override) {
            logfile_clear(wrapper->logfile);
        } else {
            filename = (char*)malloc(strlen(wrapper->logfile->filename) + 1);
            sprintf(filename, "%s", wrapper->logfile->filename);
            maxsize = wrapper->logfile->maxsize;
            time(&now);
            t = *localtime(&now);
            strftime(date, sizeof(date), "%Y%m%d%H%M%S", &t);
            if (strstr(wrapper->extname, ".")) {
                oldFilename = (char*)malloc(strlen(wrapper->basename) + 1 + strlen(date) + strlen(wrapper->extname) + 1);
                sprintf(oldFilename, "%s_%s%s", wrapper->basename, date, wrapper->extname);
            } else {
                oldFilename = (char*)malloc(strlen(wrapper->basename) + 1 + strlen(date) + 1 + strlen(wrapper->extname) + 1);
                sprintf(oldFilename, "%s_%s.%s", wrapper->basename, date, wrapper->extname);
            }
            rename(filename, oldFilename);
            free(oldFilename);
            logfile_close(wrapper->logfile);
            wrapper->logfile = logfile_open(filename, maxsize);
            free(filename);
            if (!wrapper->logfile) {
                free(wrapper);
                wrapper = NULL;
                return flag;
            }
        }
        if (!tag || 0 == strlen(tag)) {
            if (withtime) {
                flag = logfile_record_with_time(wrapper->logfile, content);
            } else {
                flag = logfile_record(wrapper->logfile, content, 0);
            }
        } else {
            flag = logfile_record_with_tag(wrapper->logfile, tag, withtime, content);
        }
    }
    return flag;
}
