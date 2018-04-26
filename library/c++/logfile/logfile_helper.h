/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile helper
**********************************************************************/
#ifndef _LOGFILE_HELPER_H_
#define _LOGFILE_HELPER_H_

#include "logfile.h"

#define LOGFILE_DEFAULT_MAXSIZE     1024*1024*10L

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct logrecord_st {
    logfile_st* logfile;
    char* basename;
    char* extname;
    unsigned int override;
    unsigned int count;
} logrecord_st;

/*
 * Brief:	init logfile helper
 * Param:	basename - file base name, e.g. "Demo" or "demo_"
 *          extname - file extend name, e.g. ".log" or ".err"
 *          maxSize - file max size
 *          override - when file reach max size, 0.create new file and write, 1. clear file and override
 * Return:	logrecord_st*
 */
extern logrecord_st* logfilehelper_init(const char* basename, const char* extname, unsigned int maxSize, unsigned int override);

/*
 * Brief:	get is logfile helper enable
 * Param:	logrecord - a log record
 * Return:	0.disable
 *          1.enable
 */
extern unsigned int logfilehelper_isenable(logrecord_st* logrecord);

/*
 * Brief:	set logfile helper enable
 * Param:	logrecord - a log record
 *          enable - 0.false, 1.true
 * Return:	void
 */
extern void logfilehelper_enable(logrecord_st* logrecord, unsigned int enable);

/*
 * Brief:	record log to file
 * Param:	logrecord - a log record
 *          tag - record tag
 *          withtime - with time, 0.false, 1.true
 *          content - record content
 * Return:	0.ok
 *          1.logrecord is invalid
 *          2.content is null or empty
 *          3.file reach max size, create new file fail
 */
extern unsigned int logfilehelper_record(logrecord_st* logrecord, const char* tag, unsigned int withtime, const char* content);

#ifdef __cplusplus
}
#endif

#endif	// _LOGFILE_HELPER_H_
