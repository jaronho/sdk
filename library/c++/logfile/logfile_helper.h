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

/*
 * Brief:	init logfile helper
 * Param:	prefix - file prefix name, e.g. "hud_"
 *          maxSize - file max size
 * Return:	void
 */
extern void logfilehelper_init(const char* prefix, long maxSize);

/*
 * Brief:	get is logfile helper enable
 * Param:	void
 * Return:	0.disable
 *          1.enable
 */
extern int logfilehelper_isenable(void);

/*
 * Brief:	set logfile helper enable
 * Param:	enable - 0.false, 1.true
 * Return:	void
 */
extern void logfilehelper_enable(int enable);

/*
 * Brief:	record log to file
 * Param:	tag - record tag
 *          content - record content
 * Return:	void
 */
extern void logfilehelper_record(const char* tag, const char* content);

#ifdef __cplusplus
}
#endif

#endif	// _LOGFILE_HELPER_H_
