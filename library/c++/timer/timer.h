/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer
**********************************************************************/
#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef __cplusplus
extern "C"
{
#endif

struct _timer_st_;

typedef void (*tm_callback_run)(struct _timer_st_*, long, void*);
typedef void (*tm_callback_over)(struct _timer_st_*, void*);

typedef struct _timer_st_ {
	long interval;						// interval duration in milliseconds
	long total_count;					// number of intervals, if count <= 0, timer will repeat forever
	long current_count;					// current interval count
	long start_time;					// start time for the current interval in milliseconds
	int running;						// status of the timer
	int is_pause;						// is timer paused
	tm_callback_run run_handler;		// called when current count changed
	tm_callback_over over_handler;		// called when timer is complete
	void* param;						// parameter
} timer_st;

/*
 * Brief:	create a timer
 * Param:	interval - interval duration in milliseconds
 *			count - number of intervals, if count <= 0, timer will repeat forever
 *			run_handler - called when current count changed
 *			over_handler - called when timer is complete
 *			param - parameter
 * Return:	timer_st*
 */
extern timer_st* create_timer(long interval, long count, tm_callback_run run_handler, tm_callback_over over_handler, void* param);

/*
 * Brief:	update a timer
 * Param:	tm - timer
 *			current_time - current time in milliseconds
 * Return:	void
 */
extern void update_timer(timer_st* tm, long current_time);

/*
 * Brief:	start a timer
 * Param:	tm - timer
 *			current_time - current time in milliseconds
 *			execute_flag - if execute run handler immediately
 * Return:	void
 */
extern void start_timer(timer_st* tm, long current_time, int execute_flag);

/*
 * Brief:	stop a timer
 * Param:	tm - timer
 *			execute_flag - if execute over handler immediately
 * Return:	void
 */
extern void stop_timer(timer_st* tm, int execute_flag);

/*
 * Brief:	resume a timer
 * Param:	tm - timer
 * Return:	void
 */
extern void resume_timer(timer_st* tm);

/*
 * Brief:	pause a timer
 * Param:	tm - timer
 * Return:	void
 */
extern void pause_timer(timer_st* tm);

/*
 * Brief:	get timer interval
 * Param:	tm - timer
 * Return:	long
 */
extern long get_timer_interval(timer_st* tm);

/*
 * Brief:	set timer interval
 * Param:	tm - timer
 *			interval - interval duration in milliseconds
 * Return:	void
 */
extern void set_timer_interval(timer_st* tm, long interval);

/*
 * Brief:	get timer total count
 * Param:	tm - timer
 * Return:	long
 */
extern long get_timer_total_count(timer_st* tm);

/*
 * Brief:	set timer total count
 * Param:	tm - timer
 *			count - total count
 * Return:	void
 */
extern void set_timer_total_count(timer_st* tm, long count);

/*
 * Brief:	get timer current count
 * Param:	tm - timer
 * Return:	long
 */
extern long get_timer_current_count(timer_st* tm);

/*
 * Brief:	check if timer is running
 * Param:	tm - timer
 * Return:	int, 0.Not running, 1.Running
 */
extern int is_timer_running(timer_st* tm);

/*
 * Brief:	set timer run handler
 * Param:	tm - timer
 *			run_handler - run handler
 * Return:	void
 */
extern void set_timer_run_handler(timer_st* tm, tm_callback_run run_handler);

/*
 * Brief:	set timer over handler
 * Param:	tm - timer
 *			over_handler - over handler
 * Return:	void
 */
extern void set_timer_over_handler(timer_st* tm, tm_callback_over over_handler);

/*
 * Brief:	get timer parameter
 * Param:	tm - timer
 * Return:	void*
 */
extern void* get_timer_param(timer_st* tm);

/*
 * Brief:	set timer parameter
 * Param:	tm - timer
 *			param - parameter
 * Return:	void
 */
extern void set_timer_param(timer_st* tm, void* param);

#ifdef __cplusplus
}
#endif

#endif // _TIMER_H_
