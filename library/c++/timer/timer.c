/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer
**********************************************************************/
#include "timer.h"
#include <stdlib.h>
#include <math.h>

#ifndef NULL
#define NULL	0
#endif

timer_st* create_timer(long interval, long count, tm_callback_run run_handler, tm_callback_over over_handler, void* param) {
	if (interval <= 0) {
		return NULL;
	}
	timer_st* tm = (timer_st*)malloc(sizeof(timer_st));
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

void update_timer(timer_st* tm, long current_time) {
	if (NULL == tm || !tm->running) {
		return;
	}
	if (tm->is_pause || current_time < tm->start_time) {
		tm->start_time = current_time;
		return;
	}
	if (tm->total_count <= 0 || tm->current_count < tm->total_count) {
		long deltaTime = abs(current_time - tm->start_time);
		if (deltaTime >= tm->interval) {
			long runCount = (long)(deltaTime / tm->interval);
			tm->current_count = tm->current_count + runCount;
			tm->start_time = current_time;
			if (NULL != tm->run_handler) {
				tm->run_handler(tm, runCount, tm->param);
			}
		}
	} else {
		stop_timer(tm, 1);
	}
}

void start_timer(timer_st* tm, long current_time, int execute_flag) {
	if (NULL == tm || tm->running) {
		return;
	}
	tm->running = 1;
	tm->is_pause = 0;
	tm->current_count = 0;
	tm->start_time = current_time;
	if (NULL != tm->run_handler && execute_flag) {
		tm->run_handler(tm, 0, tm->param);
	}
}

void stop_timer(timer_st* tm, int execute_flag) {
	if (NULL == tm || !tm->running) {
		return;
	}
	tm->running = 0;
	tm->is_pause = 1;
	if (NULL != tm->over_handler && execute_flag) {
		tm->over_handler(tm, tm->param);
	}
}

void resume_timer(timer_st* tm) {
	if (NULL == tm) {
		return;
	}
	tm->is_pause = 0;
}

void pause_timer(timer_st* tm) {
	if (NULL == tm) {
		return;
	}
	tm->is_pause = 1;
}

long get_timer_interval(timer_st* tm) {
	if (NULL == tm) {
		return 0;
	}
	return tm->interval;
}

void set_timer_interval(timer_st* tm, long interval) {
	if (NULL == tm) {
		return;
	}
	tm->interval = interval;
}

long get_timer_total_count(timer_st* tm) {
	if (NULL == tm) {
		return 0;
	}
	return tm->total_count;
}

void set_timer_total_count(timer_st* tm, long count) {
	if (NULL == tm) {
		return;
	}
	tm->total_count = count;
}

long get_timer_current_count(timer_st* tm) {
	if (NULL == tm) {
		return 0;
	}
	return tm->current_count;
}

int is_timer_running(timer_st* tm) {
	if (NULL == tm) {
		return 0;
	}
	return tm->running;
}

void set_timer_run_handler(timer_st* tm, tm_callback_run run_handler) {
	if (NULL == tm) {
		return;
	}
	tm->run_handler = run_handler;
}

void set_timer_over_handler(timer_st* tm, tm_callback_over over_handler) {
	if (NULL == tm) {
		return;
	}
	tm->over_handler = over_handler;
}

void* get_timer_param(timer_st* tm) {
	if (NULL == tm) {
		return NULL;
	}
	return tm->param;
}

void set_timer_param(timer_st* tm, void* param) {
	if (NULL == tm) {
		return;
	}
	tm->param = param;
}
