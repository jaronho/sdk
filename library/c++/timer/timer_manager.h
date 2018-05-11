/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer manager
**********************************************************************/
#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_

#include "timer.h"

class TimerManager {
public:
    /*
     * Brief:	initialize
     * Param:	void
     * Return:	void
     */
    static TimerManager* getInstance(void);

    /*
     * Brief:	update timer manager, need to be called in main thread for loop
     * Param:	void
     * Return:	void
     */
    void update(void);

    /*
     * Brief:	start a custom timer
     * Param:	id - id
     *			interval - timer trigger interval(millisecond)
     *			count - timer trigger count, when <=0 it will be in loop
     *			run_handler - timer trigger callback
     *			over_handler - timer over callback
     * Return:	void
     */
    void run(const char* id, unsigned long interval, unsigned long count, tm_callback_run run_handler, tm_callback_over over_handler, void* param);

    /*
     * Brief:	start a loop timer
     * Param:	id - id
     *			interval - timer trigger interval(millisecond)
     *			run_handler - timer trigger callback
     * Return:	void
     */
    void runLoop(const char* id, unsigned long interval, tm_callback_run run_handler);

    /*
     * Brief:	start an once timer
     * Param:	id - id
     *			interval - timer trigger interval(millisecond)
     *			over_handler - timer over callback
     * Return:	void
     */
    void runOnce(const char* id, unsigned long interval, tm_callback_over over_handler);

    /*
     * Brief:	stop a timer
     * Param:	id - id
     * Return:	void
     */
    void stop(const char* id);

    /*
     * Brief:	clear all timer
     * Param:	void
     * Return:	void
     */
    void clear(void);
};

#endif // _TIMER_MANAGER_H_
