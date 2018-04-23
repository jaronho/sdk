#include "logfile.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

logfile_st* logfile_open(const char* filename, unsigned int maxSize) {
    logfile_st* lf = NULL;
    FILE* fp = NULL;
    unsigned int nameLength = filename ? strlen(filename) : 0;
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

unsigned int logfile_close(logfile_st* lf) {
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

unsigned int logfile_clear(logfile_st* lf) {
    FILE* fp = NULL;
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_lock(&lf->mutex);
#endif
    fclose(lf->fileptr);
    lf->fileptr = NULL;
    fp = fopen(lf->filename, "w+");
    if (!fp) {
        return 2;
    }
    fclose(fp);
    fp = fopen(lf->filename, "a+");
    if (fp) {
        lf->fileptr = fp;
    }
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_unlock(&lf->mutex);
#endif
    return 0;
}

const char* logfile_name(logfile_st* lf) {
    if (!lf) {
        return NULL;
    }
    return lf->filename;
}

unsigned int logfile_isenable(logfile_st* lf) {
    if (!lf || !lf->fileptr || !lf->filename) {
        return 0;
    }
    return lf->enable;
}

unsigned int logfile_enable(logfile_st* lf, unsigned int enable) {
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
    lf->enable = enable > 0 ? 1 : 0;
    return 0;
}

unsigned int logfile_record(logfile_st* lf, const char* content, unsigned int newline) {
    unsigned int fileSize = 0;
    unsigned int contentLength = 0;
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
    if (!lf->enable) {
        return 2;
    }
    contentLength = content ? strlen(content) : 0;
    if (0 == contentLength) {
        return 3;
    }
#ifdef LOGFILE_THREAD_SAFETY
    pthread_mutex_lock(&lf->mutex);
#endif
    fseek(lf->fileptr, 0, SEEK_END);
    fileSize = ftell(lf->fileptr);
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

unsigned int logfile_record_with_time(logfile_st* lf, const char* content) {
    time_t now;
    struct tm t;
    char date[32] = { 0 };
    char* buf = NULL;
    unsigned int contentLength = 0;
    unsigned int flag = 0;
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
    if (!lf->enable) {
        return 2;
    }
    contentLength = content ? strlen(content) : 0;
    if (0 == contentLength) {
        return 3;
    }
    time(&now);
    t = *localtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &t);
    buf = malloc(strlen(date) + 4 + contentLength);
    sprintf(buf, "[%s] %s", date, content);
    flag = logfile_record(lf, buf, 1);
    free(buf);
    return flag;
}

unsigned int logfile_record_with_tag(logfile_st* lf, const char* tag, unsigned int withtime, const char* content) {
    unsigned int tagLength = 0;
    unsigned int contentLength = 0;
    char* buf = NULL;
    unsigned int flag = 0;
    if (!lf || !lf->fileptr || !lf->filename) {
        return 1;
    }
    if (!lf->enable) {
        return 2;
    }
    tagLength = tag ? strlen(tag) : 0;
    if (0 == tagLength) {
        return 3;
    }
    contentLength = content ? strlen(content) : 0;
    if (0 == contentLength) {
        return 3;
    }
    buf = malloc(tagLength + 4 + contentLength);
    sprintf(buf, "[%s] %s", tag, content);
    if (withtime) {
        flag = logfile_record_with_time(lf, buf);
    } else {
        flag = logfile_record(lf, buf, 1);
    }
    free(buf);
    return flag;
}
