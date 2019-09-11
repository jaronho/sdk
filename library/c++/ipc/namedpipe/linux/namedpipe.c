#include "namedpipe.h"
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef NULL
#define NULL    0
#endif

namedpipe_st* namedpipe_open(const char* name, int rdFlag) {
    namedpipe_st* np = NULL;
    int fd = -1;
    assert(name && strlen(name) > 0);
    assert(0 == rdFlag || 1 == rdFlag);
    if (-1 == access(name, F_OK)) {
        if (0 != mkfifo(name, 0777)) {
            printf("Could not create fifo '%s'\n", name);
            return NULL;
        }
    }
    fd = open(name, 0 == rdFlag ? (O_RDONLY | O_NONBLOCK) : O_WRONLY);
    if (-1 == fd) {
        printf("Could not open fifo '%s' for %s\n", name, 0 == rdFlag ? "read" : "write");
        return NULL;
    }
    np = (namedpipe_st*)malloc(sizeof(namedpipe_st));
    if (!np) {
        close(fd);
        return NULL;
    }
    np->name = (char*)malloc(strlen(name) + 1);
    if (!np->name) {
        close(fd);
        free(np);
        return NULL;
    }
    sprintf(np->name, "%s", name);
    np->fd = fd;
    np->rd_flag = rdFlag;
    return np;
}

void namedpipe_close(namedpipe_st* np) {
    if (!np) {
        return;
    }
    if (-1 != np->fd) {
        close(np->fd);
    }
    if (np->name) {
        free(np->name);
        np->name = NULL;
    }
    free(np);
    np = NULL;
}

const char* namedpipe_name(namedpipe_st* np) {
    return np ? np->name : NULL;
}

int namedpipe_fd(namedpipe_st* np) {
    return np ? np->fd : -1;
}

int namedpipe_read(namedpipe_st* np, void* buffer, unsigned int count) {
    if (!np) {
        return 1;
    }
    if (-1 == np->fd) {
        return 2;
    }
    if (!buffer || count <= 0) {
        return 3;
    }
    if (read(np->fd, buffer, count) <= 0) {
        return 4;
    }
    return 0;
}

int namedpipe_write(namedpipe_st* np, const void* buffer, unsigned int count) {
    if (!np) {
        return 1;
    }
    if (-1 == np->fd) {
        return 2;
    }
    if (!buffer || count <= 0) {
        return 3;
    }
    if (write(np->fd, buffer, count) <= 0) {
        return 4;
    }
    return 0;
}
