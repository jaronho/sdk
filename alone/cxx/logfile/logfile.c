/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile
**********************************************************************/
#include "logfile.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

logfile_st* logfile_open(const char* filename, size_t maxSize) {
    logfile_st* lf = NULL;
    FILE* fp = NULL;
    assert(filename && strlen(filename) > 0);
    assert(maxSize > 0);
    fp = fopen(filename, "a+");
    if (!fp) {
        return NULL;
    }
    lf = (logfile_st*)malloc(sizeof(logfile_st));
    if (!lf) {
        fclose(fp);
        return NULL;
    }
    lf->fileptr = fp;
    lf->filename = (char*)malloc(strlen(filename) + 1);
    if (!lf->filename) {
        fclose(fp);
        free(lf);
        return NULL;
    }
    sprintf(lf->filename, "%s", filename);
    lf->maxsize = maxSize;
    lf->enable = 1;
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_init(&lf->mutex, NULL);
#endif
    return lf;
}

void logfile_close(logfile_st* lf) {
    if (!lf) {
        return;
    }
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_destroy(&lf->mutex);
#endif
    if (lf->fileptr) {
        fclose(lf->fileptr);
        lf->fileptr = NULL;
    }
    if (lf->filename) {
        free(lf->filename);
        lf->filename = NULL;
    }
    free(lf);
    lf = NULL;
}

unsigned int logfile_clear(logfile_st* lf) {
    FILE* fp = NULL;
    if (!lf) {
        return 1;
    }
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_lock(&lf->mutex);
#endif
    if (lf->fileptr) {
        fclose(lf->fileptr);
        lf->fileptr = NULL;
    }
    if (lf->filename) {
        fp = fopen(lf->filename, "w+");
        if (fp) {
            fclose(fp);
        }
        fp = fopen(lf->filename, "a+");
        if (fp) {
            lf->fileptr = fp;
        }
    }
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_unlock(&lf->mutex);
#endif
    return 0;
}

const char* logfile_name(logfile_st* lf) {
    return lf ? lf->filename : NULL;
}

unsigned int logfile_isenable(logfile_st* lf) {
    return lf ? lf->enable : 0;
}

void logfile_enable(logfile_st* lf, unsigned int enable) {
    if (!lf) {
        return;
    }
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_lock(&lf->mutex);
#endif
    lf->enable = enable > 0 ? 1 : 0;
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_unlock(&lf->mutex);
#endif
}

unsigned int logfile_record(logfile_st* lf, const char* content, unsigned int newline) {
    unsigned long fileSize = 0;
    unsigned long contentLength = 0;
    if (!lf) {
        return 1;
    }
    if (!content) {
        return 2;
    }
    if (!lf->enable) {
        return 3;
    }
    if (!lf->fileptr) {
        return 4;
    }
    contentLength = strlen(content);
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_lock(&lf->mutex);
#endif
    fseek(lf->fileptr, 0, SEEK_END);
    fileSize = (unsigned long)ftell(lf->fileptr);
    if (fileSize > 0) {
        if (contentLength > lf->maxsize) {
            return 5;
        } else if (fileSize + 1 + contentLength >= lf->maxsize) {
            return 6;
        } else if (newline) {
            fwrite("\n", 1, 1, lf->fileptr);
        }
    }
    fwrite(content, contentLength, 1, lf->fileptr);
    fflush(lf->fileptr);
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_unlock(&lf->mutex);
#endif
    return 0;
}

unsigned int logfile_record_with_time(logfile_st* lf, const char* content) {
    time_t now;
    struct tm t;
    char date[32] = { 0 };
    char* buf = NULL;
    unsigned long contentLength = 0;
    unsigned int flag = 0;
    if (!lf) {
        return 1;
    }
    if (!content) {
        return 2;
    }
    if (!lf->enable) {
        return 3;
    }
    if (!lf->fileptr) {
        return 4;
    }
    contentLength = strlen(content);
    time(&now);
    t = *localtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &t);
    buf = (char*)malloc(1 + strlen(date) + 2 + contentLength + 1);
    sprintf(buf, "[%s] %s", date, content);
    flag = logfile_record(lf, buf, 1);
    free(buf);
    return flag;
}

unsigned int logfile_record_with_tag(logfile_st* lf, const char* tag, unsigned int withtime, const char* content) {
    unsigned long contentLength = 0;
    char* buf = NULL;
    unsigned int flag = 0;
    if (!lf) {
        return 1;
    }
    if (!content) {
        return 2;
    }
    if (!lf->enable) {
        return 3;
    }
    if (!lf->fileptr) {
        return 4;
    }
    contentLength = strlen(content);
    buf = (char*)malloc(1 + (tag ? strlen(tag) : 0) + 2 + contentLength + 1);
    sprintf(buf, "[%s] %s", tag, content);
    if (withtime) {
        flag = logfile_record_with_time(lf, buf);
    } else {
        flag = logfile_record(lf, buf, 1);
    }
    free(buf);
    return flag;
}
