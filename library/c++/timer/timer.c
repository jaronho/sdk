/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer
**********************************************************************/
#include "timer.h"
#include <stdlib.h>
#include <math.h>

timer_st* create_timer(unsigned long interval, unsigned long count, timer_callback_run run_handler, timer_callback_over over_handler, void* param) {
	if (interval <= 0) {
		return NULL;
	}
    static unsigned int s_id = 0;
	timer_st* tm = (timer_st*)malloc(sizeof(timer_st));
    tm->id = ++s_id;
	tm->interval = interval;
	tm->total_count = count;
	tm->current_count = 0;
	tm->start_time = 0;
	tm->running = 0;
	tm->is_pause = 0;
	tm->run_handler = run_handler;
	tm->over_handler = over_handler;
	tm->param = param;
	return tm;
}

int update_timer(timer_st* tm, unsigned long long current_time) {
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
        unsigned long long deltaTime = (unsigned long long)abs((unsigned int)(current_time - tm->start_time));
        if (deltaTime >= tm->interval) {
            unsigned int runCount = (unsigned int)(deltaTime / tm->interval);
            tm->current_count = tm->current_count + runCount;
            tm->start_time = current_time;
            if (tm->run_handler) {
                tm->run_handler(tm, runCount, tm->param);
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
		tm->run_handler(tm, 0, tm->param);
	}
}

void stop_timer(timer_st* tm, unsigned int execute_flag) {
	if (!tm || !tm->running) {
		return;
	}
	tm->running = 0;
	tm->is_pause = 1;
	if (tm->over_handler && execute_flag) {
		tm->over_handler(tm, tm->param);
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

unsigned long get_timer_id(timer_st* tm) {
    if (!tm) {
        return 0;
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
