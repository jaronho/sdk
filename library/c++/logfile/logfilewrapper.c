/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile wrapper
**********************************************************************/
#include "logfilewrapper.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

logfilewrapper_st* logfilewrapper_init(const char* basename, const char* extname, unsigned int maxSize, unsigned int override) {
    if (!basename || 0 == strlen(basename) || !extname || 0 == strlen(extname) || maxSize <= 0) {
        return NULL;
    }
    char* filename = NULL;
    logfilewrapper_st* wrapper = (logfilewrapper_st*)malloc(sizeof(logfilewrapper_st));
    wrapper->basename = malloc(strlen(basename));
    sprintf(wrapper->basename, "%s", basename);
    if (strstr(extname, ".")) {
        wrapper->extname = malloc(strlen(extname));
        sprintf(wrapper->extname, "%s", extname);
        filename = malloc(strlen(basename) + strlen(extname));
        sprintf(filename, "%s%s", basename, extname);
    } else {
        wrapper->extname = malloc(strlen(extname) + 1);
        sprintf(wrapper->extname, ".%s", extname);
        filename = malloc(strlen(basename) + strlen(extname) + 1);
        sprintf(filename, "%s.%s", basename, extname);
    }
    wrapper->logfile = logfile_open(filename, maxSize);
    free(filename);
    if (!wrapper->logfile) {
        free(wrapper);
        return NULL;
    }
    wrapper->override = override;
    wrapper->count = 1;
    return wrapper;
}

unsigned int logfilewrapper_isenable(logfilewrapper_st* wrapper) {
    if (wrapper) {
        return logfile_isenable(wrapper->logfile);
    }
    return 0;
}

void logfilewrapper_enable(logfilewrapper_st* wrapper, unsigned int enable) {
    if (wrapper) {
        logfile_enable(wrapper->logfile, enable);
    }
}

unsigned int logfilewrapper_record(logfilewrapper_st* wrapper, const char* tag, unsigned int withtime, const char* content) {
    if (!wrapper || !wrapper->logfile) {
        return 1;
    }
    if (!content || 0 == strlen(content)) {
        return 2;
    }
    unsigned int flag = 0;
    char* filename = NULL;
    if (!tag || 0 == strlen(tag)) {
        if (withtime) {
            flag = logfile_record_with_time(wrapper->logfile, content);
        } else {
            flag = logfile_record(wrapper->logfile, content, 0);
        }
    } else {
        flag = logfile_record_with_tag(wrapper->logfile, tag, withtime, content);
    }
    if (5 == flag) {
        if (wrapper->override) {
            logfile_clear(wrapper->logfile);
        } else {
            logfile_close(wrapper->logfile);
            if (strstr(wrapper->extname, ".")) {
                filename = malloc(strlen(wrapper->basename) + strlen(wrapper->extname));
                sprintf(filename, "%s-%03d%s", wrapper->basename, wrapper->count + 1, wrapper->extname);
            } else {
                filename = malloc(strlen(wrapper->basename) + strlen(wrapper->extname) + 1);
                sprintf(filename, "%s-%03d.%s", wrapper->basename, wrapper->count + 1, wrapper->extname);
            }
            wrapper->logfile = logfile_open(filename, wrapper->logfile->maxsize);
            free(filename);
            if (!wrapper->logfile) {
                free(wrapper->basename);
                wrapper->basename = NULL;
                free(wrapper->extname);
                wrapper->extname = NULL;
                free(wrapper);
                wrapper = NULL;
                return 3;
            }
        }
        ++wrapper->count;
        if (!tag || 0 == strlen(tag)) {
            if (withtime) {
                logfile_record_with_time(wrapper->logfile, content);
            } else {
                logfile_record(wrapper->logfile, content, 0);
            }
        } else {
            logfile_record_with_tag(wrapper->logfile, tag, withtime, content);
        }
    }
    return 0;
}
