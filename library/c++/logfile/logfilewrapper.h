/**********************************************************************
* Author:	jaron.ho
* Date:		2017-12-25
* Brief:	logfile wrapper
**********************************************************************/
#ifndef _LOGFILE_WRAPPER_H_
#define _LOGFILE_WRAPPER_H_

#include "logfile.h"

#define LOGFILE_DEFAULT_MAXSIZE     1024*1024*10L

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct logfilewrapper_st {
    logfile_st* logfile;
    char* basename;
    char* extname;
    unsigned int override;
} logfilewrapper_st;

/*
 * Brief:	init logfile wraper
 * Param:	basename - file base name, e.g. "demo" or "/home/workspace/demo"
 *          extname - file extend name, e.g. ".log" or ".err"
 *          maxSize - file max size
 *          override - when file reach max size, 0.create new file and write, 1. clear file and override
 * Return:	logfilewrapper_st*
 */
extern logfilewrapper_st* logfilewrapper_init(const char* basename, const char* extname, size_t maxSize, unsigned int override);

/*
 * Brief:	close logfile wraper
 * Param:	wrapper - a logfile wrapper
 * Return:	logfilewrapper_st*
 */
extern void logfilewrapper_close(logfilewrapper_st* wrapper);

/*
 * Brief:	get is logfile wrapper enable
 * Param:	wrapper - a logfile wrapper
 * Return:	0.disable
 *          1.enable
 */
extern unsigned int logfilewrapper_isenable(logfilewrapper_st* wrapper);

/*
 * Brief:	set logfile wrapper enable
 * Param:	wrapper - a logfile wrapper
 *          enable - 0.false, 1.true
 * Return:	void
 */
extern void logfilewrapper_enable(logfilewrapper_st* wrapper, unsigned int enable);

/*
 * Brief:	record log to file
 * Param:	wrapper - a logfile wrapper
 *          tag - record tag
 *          withtime - with time, 0.false, 1.true
 *          content - record content
 * Return:	0.ok
 *          1.disabled
 *          2.content size large max
 *          3.file reach max size, create new file fail
 */
extern unsigned int logfilewrapper_record(logfilewrapper_st* wrapper, const char* tag, unsigned int withtime, const char* content);

/*
 * Brief:	record log to file, if basename and extname not equal wrapper's, will create new log file
 * Param:	wrapper - a logfile wrapper
 *          tag - record tag
 *          withtime - with time, 0.false, 1.true
 *          content - record content
 *          basename - file base name, allow NULL, e.g. "demo" or "/home/workspace/demo"
 *          extname - file extend name, allow NULL, e.g. ".log" or ".err"
 * Return:	0.ok
 *          1.disabled
 *          2.content size large max
 *          3.file reach max size, create new file fail
 */
extern unsigned int logfilewrapper_record_ex(logfilewrapper_st* wrapper, const char* tag, unsigned int withtime, const char* content, const char* basename, const char* extname);

#ifdef __cplusplus
}
#endif

#endif	// _LOGFILE_WRAPPER_H_
