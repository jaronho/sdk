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

struct timer_st;

typedef void (*timer_callback_run)(struct timer_st* tm, unsigned long runCount);
typedef void (*timer_callback_over)(struct timer_st* tm);

typedef struct timer_st {
    unsigned long interval;					// interval duration in milliseconds
    unsigned long total_count;				// number of intervals, if count <= 0, timer will repeat forever
    unsigned long current_count;			// current interval count
    unsigned long long start_time;			// start time for the current interval in milliseconds
    unsigned int running;					// status of the timer
    unsigned int is_pause;					// is timer paused
    timer_callback_run run_handler;		    // called when current count changed
    timer_callback_over over_handler;		// called when timer is complete
    char id[128];                           // id
    void* param;						    // parameter
} timer_st;

/*
 * Brief:	create a timer
 * Param:	interval - interval duration in milliseconds
 *			count - number of intervals, if count <= 0, timer will repeat forever
 *			run_handler - called when current count changed
 *			over_handler - called when timer is complete
 *          id - id
 *			param - parameter
 * Return:	timer_st*
 */
extern timer_st* create_timer(unsigned long interval, unsigned long count, timer_callback_run run_handler, timer_callback_over over_handler, const char* id, void* param);

/*
 * Brief:	update a timer
 * Param:	tm - timer
 *			current_time - current time in milliseconds
 * Return:	int, 0.running, 1.tm is null, 2.tm not running, 3.tm is pause or not trigger, 4.tm is over
 */
extern int update_timer(timer_st* tm, unsigned long long current_time);

/*
 * Brief:	start a timer
 * Param:	tm - timer
 *			current_time - current time in milliseconds
 *			execute_flag - if execute run handler immediately
 * Return:	void
 */
extern void start_timer(timer_st* tm, unsigned long long current_time, unsigned int execute_flag);

/*
 * Brief:	stop a timer
 * Param:	tm - timer
 *			execute_flag - if execute over handler immediately
 * Return:	void
 */
extern void stop_timer(timer_st* tm, unsigned int execute_flag);

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
 * Brief:	get timer unique id
 * Param:	tm - timer
 * Return:	const char*
 */
extern const char* get_timer_id(timer_st* tm);

/*
 * Brief:	get timer interval
 * Param:	tm - timer
 * Return:	long
 */
extern unsigned long get_timer_interval(timer_st* tm);

/*
 * Brief:	set timer interval
 * Param:	tm - timer
 *			interval - interval duration in milliseconds
 * Return:	void
 */
extern void set_timer_interval(timer_st* tm, unsigned long interval);

/*
 * Brief:	get timer total count
 * Param:	tm - timer
 * Return:	long
 */
extern unsigned long get_timer_total_count(timer_st* tm);

/*
 * Brief:	set timer total count
 * Param:	tm - timer
 *			count - total count
 * Return:	void
 */
extern void set_timer_total_count(timer_st* tm, unsigned long count);

/*
 * Brief:	get timer current count
 * Param:	tm - timer
 * Return:	long
 */
extern unsigned long get_timer_current_count(timer_st* tm);

/*
 * Brief:	check if timer is running
 * Param:	tm - timer
 * Return:	int, 0.Not running, 1.Running
 */
extern unsigned int is_timer_running(timer_st* tm);

/*
 * Brief:	set timer run handler
 * Param:	tm - timer
 *			run_handler - run handler
 * Return:	void
 */
extern void set_timer_run_handler(timer_st* tm, timer_callback_run run_handler);

/*
 * Brief:	set timer over handler
 * Param:	tm - timer
 *			over_handler - over handler
 * Return:	void
 */
extern void set_timer_over_handler(timer_st* tm, timer_callback_over over_handler);

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
