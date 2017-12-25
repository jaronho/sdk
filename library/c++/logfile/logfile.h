/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile
**********************************************************************/
#ifndef _LOGFILE_H_
#define _LOGFILE_H_

#include <stdio.h>

#define LOGFILE_THREAD_SAFETY	1	/* 1.thread safe, 0.no thread safe */
#define DEF_LOGFILE_MAXSIZE     1024*1024*4L

#if LOGFILE_THREAD_SAFETY
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct logfile_st {
    FILE* fileptr;
    char* filename;
    long maxsize;
    int enable;
#if LOGFILE_THREAD_SAFETY
    pthread_mutex_t mutex;
#endif
} logfile_st;

/*
 * Brief:	open a logfile
 * Param:	fileName - log file name
 *			maxSize - file max size
 * Return:	logfile_st*
 */
extern logfile_st* logfile_open(const char* fileName, long maxSize);

/*
 * Brief:	open a logfile, default max size = DEF_LOGFILE_MAXSIZE
 * Param:	fileName - log file name
 * Return:	logfile_st*
 */
extern logfile_st* logfile_open_default(const char* fileName);

/*
 * Brief:	close a logfile
 * Param:	lf - a log file
 * Return:	int, 0.ok, 1.lf is NULL
 */
extern int logfile_close(logfile_st* lf);

/*
 * Brief:	get a logfile name
 * Param:	lf - a log file
 * Return:	const char*
 */
extern const char* logfile_name(logfile_st* lf);

/*
 * Brief:	get logfile enable status
 * Param:	lf - a log file
 * Return:	int, 0.disable, 1.enable
 */
extern int logfile_isenable(logfile_st* lf);

/*
 * Brief:	set logfile enable status
 * Param:	lf - a log file
 *          enable - 0.false, 1.true
 * Return:	int, 0.ok, 1.lf is NULL
 */
extern int logfile_enable(logfile_st* lf, int enable);

/*
 * Brief:	record to log file
 * Param:	lf - a log file
 *          content - record content
 * Return:	int, 0.ok, 1.lf is NULL, 2.lf is disable, 3.contnet is NULL/empty, 4.file size reach max
 */
extern int logfile_record(logfile_st* lf, const char* content);

/*
 * Brief:	record to log file with time
 * Param:	lf - a log file
 *          content - record content
 * Return:	int, 0.ok, 1.lf is NULL, 2.lf is disable, 3.contnet is NULL/empty, 4.file size reach max
 */
extern int logfile_record_with_time(logfile_st* lf, const char* content);

/*
 * Brief:	record to log file with time and tag
 * Param:	lf - a log file
 *          tag - record tag
 *          content - record content
 * Return:	int, 0.ok, 1.lf is NULL, 2.lf is disable, 3.tag/contnet is NULL/empty, 4.file size reach max
 */
extern int logfile_record_with_tag(logfile_st* lf, const char* tag, const char* content);

#ifdef __cplusplus
}
#endif

#endif	// _LOGFILE_H_
