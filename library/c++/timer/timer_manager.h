/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer manager
**********************************************************************/
#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_

#include "timer.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void timermanager_run(const char* id, long interval, long count, tm_callback_run run_handler, tm_callback_over over_handler, void* param);

extern void timermanager_run_loop(long interval, tm_callback_run run_handler);

extern void timermanager_run_once(long interval, tm_callback_over over_handler);

extern void timermanager_stop(const char* id);

#ifdef __cplusplus
}
#endif

#endif // _TIMER_MANAGER_H_
