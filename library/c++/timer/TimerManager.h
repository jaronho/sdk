/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer manager
**********************************************************************/
#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_

#include <functional>
#include "timer.h"

/* 定时器触发回调,返回值:无 */
#define TIMER_TRIGGER_CALLBACK std::function<void(timer_st* tm, unsigned long runCount, void* param)>
/* 定时器结束回调,返回值:无 */
#define TIMER_OVER_CALLBACK std::function<void(timer_st* tm, void* param)>

class TimerManager {
public:
    /*
     * Brief:	get current system time (current from 1970-01-01 00:00:00)
     * Param:	void
     * Return:	double (seconds)
     */
    static double getTime(void);

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
     * Param:	id - timer id
     *			interval - timer trigger interval(millisecond)
     *			count - timer trigger count, when <=0 it will be in loop
     *			triggerCallback - timer trigger callback
     *			overCallback - timer over callback
     * Return:	void
     */
    void run(const char* id, unsigned long interval, unsigned long count, TIMER_TRIGGER_CALLBACK triggerCallback, TIMER_OVER_CALLBACK overCallback, void* param = NULL);

    /*
     * Brief:	start a loop timer
     * Param:	id - id
     *			interval - timer trigger interval(millisecond)
     *			triggerCallback - timer trigger callback
     * Return:	void
     */
    void runLoop(const char* id, unsigned long interval, TIMER_TRIGGER_CALLBACK triggerCallback);

    /*
     * Brief:	start an once timer
     * Param:	id - id
     *			interval - timer trigger interval(millisecond)
     *			overCallback - timer over callback
     * Return:	void
     */
    void runOnce(const char* id, unsigned long interval, TIMER_OVER_CALLBACK overCallback);

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

/*
auto handleTimerTrigger1 = [](timer_st* tm, unsigned long runCount, void* param)->void {
    printf("------- handleTimerTrigger1 --------- %f\n", TimerManager::getTime());
};

auto handleTimerTrigger2 = [](timer_st* tm, unsigned long runCount, void* param)->void {
    printf("------- handleTimerTrigger2 --------- %f\n", TimerManager::getTime());
};

auto handleTimerOver1 = [](timer_st* tm, void* param)->void {
    printf("------- handleTimerOver1 --------- %f\n", TimerManager::getTime());
};

auto handleTimerOver3 = [](timer_st* tm, void* param)->void {
    printf("------- handleTimerOver3 --------- %f\n", TimerManager::getTime());
    TimerManager::getInstance()->stop("timer_2");
};

int main() {
    TimerManager::getInstance()->run("timer_1", 1000, 15, handleTimerTrigger1, handleTimerOver1, NULL);
    TimerManager::getInstance()->runLoop("timer_2", 2000, handleTimerTrigger2);
    TimerManager::getInstance()->runOnce("timer_3", 5000, handleTimerOver3);
    while (1) {
        Sleep(1);
        TimerManager::getInstance()->update();
    }
    return 0;
}
*/
