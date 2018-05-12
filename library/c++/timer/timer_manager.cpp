/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer manager
**********************************************************************/
#include "timer_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <map>
#include <thread>
#include <mutex>
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#endif

static std::map<std::string, timer_st*> sTimerMap;
static std::mutex sTimerMapmutex;
static TimerManager* mInstance = NULL;

double TimerManager::getTime(void) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    FILETIME ft;
    double t;
    GetSystemTimeAsFileTime(&ft);
    /* Windows file time (time since January 1, 1601 (UTC)) */
    t = ft.dwLowDateTime / 1.0e7 + ft.dwHighDateTime*(4294967296.0 / 1.0e7);
    /* convert to Unix Epoch time (time since January 1, 1970 (UTC)) */
    return (t - 11644473600.0);
#else
    struct timeval v;
    gettimeofday(&v, (struct timezone*)NULL);
    /* Unix Epoch time (time since January 1, 1970 (UTC)) */
    return v.tv_sec + v.tv_usec / 1.0e6;
#endif
}

TimerManager* TimerManager::getInstance(void) {
    if (!mInstance) {
        mInstance = new TimerManager();
    }
    return mInstance;
}

void TimerManager::update(void) {
    sTimerMapmutex.lock();
    unsigned long long curr_millseconds = (unsigned long long)(getTime() * 1000);
    std::map<std::string, timer_st*>::iterator iter = sTimerMap.begin();
    while (sTimerMap.end() != iter) {
        int ret = update_timer(iter->second, curr_millseconds);
        if (1 == ret || 4 == ret) {
            sTimerMap.erase(iter);
            if (iter->second) {
                free(iter->second);
            }
        } else {
            ++iter;
        }
    }
    sTimerMapmutex.unlock();
}

void TimerManager::run(const char* id, unsigned long interval, unsigned long count, timer_callback_run run_handler, timer_callback_over over_handler, void* param) {
	if (NULL == id || 0 == strlen(id)) {
		return;
	}
    sTimerMapmutex.lock();
    std::map<std::string, timer_st*>::iterator iter = sTimerMap.find(id);
    if (sTimerMap.end() != iter) {
        sTimerMap.erase(iter);
        stop_timer(iter->second, 1);
        free(iter->second);
    }
    timer_st* tm = create_timer(interval, count, run_handler, over_handler, param);
	if (tm) {
        sTimerMap[id] = tm;
        start_timer(tm, (unsigned long long)(getTime() * 1000), 0);
	}
    sTimerMapmutex.unlock();
}

void TimerManager::runLoop(const char* id, unsigned long interval, timer_callback_run run_handler) {
	run(id, interval, 0, run_handler, NULL, NULL);
}

void TimerManager::runOnce(const char* id, unsigned long interval, timer_callback_over over_handler) {
	run(id, interval, 1, NULL, over_handler, NULL);
}

void TimerManager::stop(const char* id) {
	if (NULL == id || 0 == strlen(id)) {
		return;
	}
    sTimerMapmutex.lock();
    //std::map<std::string, timer_st*>::iterator iter = sTimerMap.find(id);
    //if (sTimerMap.end() != iter) {
    //    sTimerMap.erase(iter);
    //    stop_timer(iter->second, 1);
    //    free(iter->second);
    //}
    sTimerMapmutex.unlock();
}

void TimerManager::clear(void) {
    sTimerMapmutex.lock();
    std::map<std::string, timer_st*>::iterator iter = sTimerMap.begin();
    for (; sTimerMap.end() != iter; ++iter) {
        free(iter->second);
    }
    sTimerMap.clear();
    sTimerMapmutex.unlock();
}
