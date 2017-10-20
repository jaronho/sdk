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
static pthread_mutex_t hashmap_mutex;

static long get_milliseconds() {
	static struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

void timermanager_init(int timer_capacity) {
    if (timer_capacity <= 0) {
        return;
    }
    if (NULL == timer_hashmap) {
        timer_hashmap = hashmap_create(timer_capacity);
		pthread_mutex_init(&hashmap_mutex, NULL);
    }
}

void timermanager_update(void) {
	long curr_millseconds;
	timer_st* tm;
	int ret;
	if (NULL == timer_hashmap) {
		return;
	}
	pthread_mutex_lock(&hashmap_mutex);
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
	pthread_mutex_unlock(&hashmap_mutex);
}

void timermanager_run(const char* id, long interval, long count, tm_callback_run run_handler, tm_callback_over over_handler, void* param) {
	timer_st* tm;
	if (NULL == id || 0 == strlen(id)) {
		return;
	}
	if (NULL == timer_hashmap) {
        return;
	}
	pthread_mutex_lock(&hashmap_mutex);
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
	pthread_mutex_unlock(&hashmap_mutex);
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
	pthread_mutex_lock(&hashmap_mutex);
	tm = (timer_st*)hashmap_remove(timer_hashmap, id);
	if (NULL == tm) {
		return;
	}
	stop_timer(tm, 1);
	free(tm);
	pthread_mutex_unlock(&hashmap_mutex);
}

void timermanager_clear(void) {
    if (NULL == timer_hashmap) {
        return;
    }
	pthread_mutex_lock(&hashmap_mutex);
    hashmap_clear(timer_hashmap, 1);
	pthread_mutex_unlock(&hashmap_mutex);
}
