/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer manager
**********************************************************************/
#include "timer_manager.h"
#include "../hashmap/hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <pthread.h>

static hashmap_st* timer_hashmap = NULL;
static pthread_t update_thread_t;

static long get_milliseconds() {
	static struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static void* update_thread_func(void* arg) {
	long curr_millseconds;
	timer_st* tm;
	int ret;
	for (;;) {
		if (NULL == timer_hashmap) {
			continue;
		}
		curr_millseconds = get_milliseconds();
		hashmap_element_iterator_st iter = HASHMAP_ITERATOR(timer_hashmap);
		tm = (timer_st*)hashmap_iterate_value( &iter);
		while (NULL != tm) {
			ret = update_timer(tm, curr_millseconds);
			if (1 == ret || 3 == ret) {
				tm = (timer_st*)hashmap_remove(timer_hashmap, hashmap_iterate_key(&iter));
				if (NULL != tm) {
					free(tm);
				}
			}
			tm = (timer_st*)hashmap_iterate_value(&iter);
		}
	}
	return (void*)0;
}

static void start_update_thread() {
	pthread_attr_t tattr;
	if (0 != pthread_attr_init(&tattr)) {
		return;
	}
	if (0 != pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED)) {
		return;
	}
	if (0 != pthread_attr_setinheritsched(&tattr, PTHREAD_INHERIT_SCHED)) {
		return;
	}
	if (0 != pthread_create(&update_thread_t, &tattr, update_thread_func, (void*)NULL)) {
		return;
	}
}

void timermanager_init(int timer_capacity) {
    if (timer_capacity <= 0) {
        return;
    }
    if (NULL == timer_hashmap) {
        timer_hashmap = hashmap_create(timer_capacity);
        start_update_thread();
    }
}

void timermanager_run(const char* id, long interval, long count, tm_callback_run run_handler, tm_callback_over over_handler, void* param) {
	timer_st* tm;
	if (NULL == id || 0 == strlen(id)) {
		return;
	}
	if (NULL == timer_hashmap) {
        return;
	}
	tm = (timer_st*)hashmap_remove(timer_hashmap, id);
	if (NULL != tm) {
		free(tm);
	}
	tm = create_timer(interval, count, run_handler, over_handler, param);
	if (NULL == tm) {
		return;
	}
	if (hashmap_put(timer_hashmap, id, (void*)tm)) {
		free(tm);
		return;
	}
	start_timer(tm, get_milliseconds(), 0);
}

void timermanager_run_loop(const char* id, long interval, tm_callback_run run_handler) {
	timermanager_run(id, interval, 0, run_handler, NULL, NULL);
}

void timermanager_run_once(const char* id, long interval, tm_callback_over over_handler) {
	timermanager_run(id, interval, 1, NULL, over_handler, NULL);
}

void timermanager_stop(const char* id) {
	timer_st* tm;
	if (NULL == id || 0 == strlen(id)) {
		return;
	}
	if (NULL == timer_hashmap) {
		return;
	}
	tm = (timer_st*)hashmap_remove(timer_hashmap, id);
	if (NULL == tm) {
		return;
	}
	stop_timer(tm, 1);
	free(tm);
}

void timermanager_clear() {
    if (NULL == timer_hashmap) {
        return;
    }
    hashmap_clear(timer_hashmap, 1);
}
