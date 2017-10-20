/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer manager
**********************************************************************/
#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_

#include "timer.h"

#define TIMER_DEFAULT_CAPACITY	1024

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Brief:	init timer manager
 * Param:	timer_capacity - the max timer count
 * Return:	void
 */
extern void timermanager_init(int timer_capacity);

/*
 * Brief:	update timer manager, need to be called in main thread for loop
 * Param:	void
 * Return:	void
 */
extern void timermanager_update(void);

/*
 * Brief:	start a custom timer
 * Param:	id - id
 *			interval - timer trigger interval(millisecond)
 *			count - timer trigger count, when <=0 it will be in loop
 *			run_handler - timer trigger callback
 *			over_handler - timer over callback
 * Return:	void
 */
extern void timermanager_run(const char* id, long interval, long count, tm_callback_run run_handler, tm_callback_over over_handler, void* param);

/*
 * Brief:	start a loop timer
 * Param:	id - id
 *			interval - timer trigger interval(millisecond)
 *			run_handler - timer trigger callback
 * Return:	void
 */
extern void timermanager_run_loop(const char* id, long interval, tm_callback_run run_handler);

/*
 * Brief:	start an once timer
 * Param:	id - id
 *			interval - timer trigger interval(millisecond)
 *			over_handler - timer over callback
 * Return:	void
 */
extern void timermanager_run_once(const char* id, long interval, tm_callback_over over_handler);

/*
 * Brief:	stop a timer
 * Param:	id - id
 * Return:	void
 */
extern void timermanager_stop(const char* id);

/*
 * Brief:	clear all timer
 * Param:	void
 * Return:	void
 */
extern void timermanager_clear(void);

#ifdef __cplusplus
}
#endif

#endif // _TIMER_MANAGER_H_
