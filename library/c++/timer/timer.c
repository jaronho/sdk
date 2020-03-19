/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer
**********************************************************************/
#include "timer.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

timer_st* create_timer(unsigned long interval, unsigned long count, timer_callback_run run_handler, timer_callback_over over_handler, const char* id, void* param) {
    timer_st* tm = NULL;
    if (interval <= 0) {
		return NULL;
	}
	tm = (timer_st*)malloc(sizeof(timer_st));
	tm->interval = interval;
	tm->total_count = count;
	tm->current_count = 0;
	tm->start_time = 0;
	tm->running = 0;
	tm->is_pause = 0;
	tm->run_handler = run_handler;
	tm->over_handler = over_handler;
    memset(tm->id, 0, sizeof(tm->id));
    if (id && strlen(id) > 0) {
        if (strlen(id) > sizeof(tm->id)) {
            free(tm);
            printf("id [%s] length [%d] is greater than the maximum length [%d] \n", id, strlen(id), sizeof(tm->id));
            return NULL;
        }
        sprintf(tm->id, "%s", id);
    }
	tm->param = param;
	return tm;
}

int update_timer(timer_st* tm, unsigned long long current_time) {
    unsigned long long deltaTime = 0;
    unsigned int runCount = 0;
	if (!tm) {
		return 1;
	}
	if (!tm->running) {
		return 2;
	}
	if (tm->is_pause || current_time < tm->start_time) {
		tm->start_time = current_time;
		return 3;
	}
    if (tm->total_count <= 0 || tm->current_count < tm->total_count) {
        deltaTime = (unsigned long long)abs((unsigned int)(current_time - tm->start_time));
        if (deltaTime >= tm->interval) {
            runCount = (unsigned int)(deltaTime / tm->interval);
            tm->current_count = tm->current_count + runCount;
            tm->start_time = current_time;
            if (tm->run_handler) {
                tm->run_handler(tm, runCount);
            }
        }
    } else {
        stop_timer(tm, 1);
        return 4;
    }
	return 0;
}

void start_timer(timer_st* tm, unsigned long long current_time, unsigned int execute_flag) {
	if (!tm || tm->running) {
		return;
	}
	tm->running = 1;
	tm->is_pause = 0;
	tm->current_count = 0;
	tm->start_time = current_time;
	if (tm->run_handler && execute_flag) {
		tm->run_handler(tm, 1);
	}
}

void stop_timer(timer_st* tm, unsigned int execute_flag) {
	if (!tm || !tm->running) {
		return;
	}
	tm->running = 0;
	tm->is_pause = 1;
	if (tm->over_handler && execute_flag) {
		tm->over_handler(tm);
	}
}

void resume_timer(timer_st* tm) {
	if (!tm) {
		return;
	}
	tm->is_pause = 0;
}

void pause_timer(timer_st* tm) {
    if (!tm) {
		return;
	}
	tm->is_pause = 1;
}

const char* get_timer_id(timer_st* tm) {
    if (!tm) {
        return NULL;
    }
    return tm->id;
}

unsigned long get_timer_interval(timer_st* tm) {
    if (!tm) {
		return 0;
	}
	return tm->interval;
}

void set_timer_interval(timer_st* tm, unsigned long interval) {
    if (!tm) {
		return;
	}
	tm->interval = interval;
}

unsigned long get_timer_total_count(timer_st* tm) {
    if (!tm) {
		return 0;
	}
	return tm->total_count;
}

void set_timer_total_count(timer_st* tm, unsigned long count) {
    if (!tm) {
		return;
	}
	tm->total_count = count;
}

unsigned long get_timer_current_count(timer_st* tm) {
    if (!tm) {
		return 0;
	}
	return tm->current_count;
}

unsigned int is_timer_running(timer_st* tm) {
    if (!tm) {
		return 0;
	}
	return tm->running;
}

void set_timer_run_handler(timer_st* tm, timer_callback_run run_handler) {
    if (!tm) {
		return;
	}
	tm->run_handler = run_handler;
}

void set_timer_over_handler(timer_st* tm, timer_callback_over over_handler) {
    if (!tm) {
		return;
	}
	tm->over_handler = over_handler;
}

void* get_timer_param(timer_st* tm) {
    if (!tm) {
		return NULL;
	}
	return tm->param;
}

void set_timer_param(timer_st* tm, void* param) {
    if (!tm) {
		return;
	}
	tm->param = param;
}
