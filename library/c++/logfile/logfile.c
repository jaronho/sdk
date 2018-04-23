#include "logfile.h"
#include <stdlib.h>

logfile_st* logfile_open(const char* filename, long maxSize) {
    logfile_st* lf;
    FILE* fp;
    long nameLength = filename ? strlen(filename) : 0;
    if (0 == nameLength || maxSize <= 0) {
        return NULL;
    }
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
    lf->filename = malloc(nameLength);
    if (!lf->filename) {
        fclose(fp);
        free(lf);
        return NULL;
    }
    memcpy(lf->filename, filename, nameLength);
    lf->maxsize = maxSize;
    lf->enable = 1;
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_init(&lf->mutex, NULL);
#endif
    return lf;
}

logfile_st* logfile_open_default(const char* filename) {
    return logfile_open(filename, 1024*1024*4L);
}

int logfile_close(logfile_st* lf) {
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
#ifdef LOGFILE_THREAD_SAFETY
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
    if (!lf) {
        return NULL;
    }
    return lf->filename;
}

int logfile_isenable(logfile_st* lf) {
    if (!lf || !lf->fileptr || !lf->filename) {
        return 0;
    }
    return lf->enable;
}

int logfile_enable(logfile_st* lf, int enable) {
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
    lf->enable = enable > 0 ? 1 : 0;
    return 0;
}

int logfile_record(logfile_st* lf, const char* content, int newline) {
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
    if (!lf->enable) {
        return 2;
    }
    long contentLength = content ? strlen(content) : 0;
    if (0 == contentLength) {
        return 3;
    }
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_lock(&lf->mutex);
#endif
    fseek(lf->fileptr, 0, SEEK_END);
    long fileSize = ftell(lf->fileptr);
    if (fileSize > 0) {
        if (contentLength > lf->maxsize) {
            return 4;
        } else if (fileSize + 1 + contentLength >= lf->maxsize) {
            return 5;
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

int logfile_record_with_time(logfile_st* lf, const char* content) {
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
    if (!lf->enable) {
        return 2;
    }
    long contentLength = content ? strlen(content) : 0;
    if (0 == contentLength) {
        return 3;
    }
    struct timeval tv;
    struct tm tm;
    char* buf = malloc(23 + contentLength);
    int flag = 0;
    gettimeofday(&tv, NULL);
    tm = *localtime(&tv.tv_sec);
    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d] %s", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, content);
    flag = logfile_record(lf, buf, 1);
    free(buf);
    return flag;
}

int logfile_record_with_tag(logfile_st* lf, const char* tag, int withtime, const char* content) {
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
    if (!lf->enable) {
        return 2;
    }
    long tagLength = tag ? strlen(tag) : 0;
    if (0 == tagLength) {
        return 3;
    }
    long contentLength = content ? strlen(content) : 0;
    if (0 == contentLength) {
        return 3;
    }
    char* buf = malloc(tagLength + 4 + contentLength);
    int flag = 0;
    sprintf(buf, "[%s] %s", tag, content);
    if (withtime) {
        flag = logfile_record_with_time(lf, buf, 1);
    } else {
        flag = logfile_record(lf, buf, 1)
    }
    free(buf);
    return flag;
}
