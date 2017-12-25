/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile
**********************************************************************/
#include "logfile.h"
#include <stdlib.h>

logfile_st* logfile_open(const char* fileName, long maxSize) {
    logfile_st* lf;
    FILE* fp;
    if (NULL == fileName || 0 == strlen(fileName) || maxSize <= 0) {
        return NULL;
    }
    fp = fopen(fileName, "a+");
    if (NULL == fp) {
        return NULL;
    }
    lf = (logfile_st*)malloc(sizeof(logfile_st));
    if (NULL == lf) {
        fclose(fp);
        return NULL;
    }
    lf->fileptr = fp;
    lf->filename = malloc(strlen(fileName));
    if (NULL == lf->filename) {
        fclose(fp);
        free(lf);
        return NULL;
    }
    memcpy(lf->filename, fileName, strlen(fileName));
    lf->maxsize = maxSize;
    lf->enable = 1;
#if LOGFILE_THREAD_SAFETY
    pthread_mutex_init(&lf->mutex, NULL);
#endif
    return lf;
}

logfile_st* logfile_open_default(const char* fileName) {
    return logfile_open(fileName, DEF_LOGFILE_MAXSIZE);
}

int logfile_close(logfile_st* lf) {
    if (NULL == lf || NULL == lf->fileptr || NULL == lf->filename) {
        return 1;
    }
#if LOGFILE_THREAD_SAFETY
    pthread_mutex_destroy(&lf->mutex);
#endif
    fclose(lf->fileptr);
    lf->fileptr = NULL;
    free(lf->filename);
    lf->filename = NULL;
    free(lf);
    return 0;
}

const char* logfile_name(logfile_st* lf) {
    if (NULL == lf) {
        return NULL;
    }
    return lf->filename;
}

int logfile_isenable(logfile_st* lf) {
    if (NULL == lf || NULL == lf->fileptr || NULL == lf->filename) {
        return 0;
    }
    return lf->enable;
}

int logfile_enable(logfile_st* lf, int enable) {
    if (NULL == lf || NULL == lf->fileptr || NULL == lf->filename) {
        return 1;
    }
    lf->enable = enable > 0 ? 1 : 0;
    return 0;
}

int logfile_record(logfile_st* lf, const char* content) {
    if (NULL == lf || NULL == lf->fileptr || NULL == lf->filename) {
        return 1;
    }
    if (!lf->enable) {
        return 2;
    }
    if (NULL == content || 0 == strlen(content)) {
        return 3;
    }
    long fileSize = 0;
    FILE* fp = NULL;
#if LOGFILE_THREAD_SAFETY
    pthread_mutex_lock(&lf->mutex);
#endif
    fseek(lf->fileptr, 0, SEEK_END);
    fileSize = ftell(lf->fileptr);
    if (fileSize > 0) {
        if (fileSize + 1 + strlen(content) >= lf->maxsize) {
            return 4;
        } else {
            fwrite("\n", 1, 1, lf->fileptr);
        }
    }
    fwrite(content, strlen(content), 1, lf->fileptr);
    fflush(lf->fileptr);
#if LOGFILE_THREAD_SAFETY
    pthread_mutex_unlock(&lf->mutex);
#endif
    return 0;
}

int logfile_record_with_time(logfile_st* lf, const char* content) {
    if (NULL == lf || NULL == lf->fileptr || NULL == lf->filename) {
        return 1;
    }
    if (!lf->enable) {
        return 2;
    }
    if (NULL == content || 0 == strlen(content)) {
        return 3;
    }
    struct timeval tv;
    struct tm tm;
    char* buf = malloc(23 + strlen(content));
    int flag = 0;
    gettimeofday(&tv, NULL);
    tm = *localtime(&tv.tv_sec);
    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d] %s", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, content);
    flag = logfile_record(lf, buf);
    free(buf);
    return flag;
}

int logfile_record_with_tag(logfile_st* lf, const char* tag, const char* content) {
    if (NULL == lf || NULL == lf->fileptr || NULL == lf->filename) {
        return 1;
    }
    if (!lf->enable) {
        return 2;
    }
    if (NULL == tag || 0 == strlen(tag) || NULL == content || 0 == strlen(content)) {
        return 3;
    }
    char* buf = malloc(strlen(tag) + 4 + strlen(content));
    int flag = 0;
    sprintf(buf, "[%s] %s", tag, content);
    flag = logfile_record_with_time(lf, buf);
    free(buf);
    return flag;
}
