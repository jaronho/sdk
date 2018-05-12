/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-15
* Brief:	timer manager
**********************************************************************/
#include "TimerManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <map>
#include <list>
#include <thread>
#include <mutex>
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#endif

class TimerWrapper {
public:
    ~TimerWrapper(void) {
        if (tm) {
            free(tm);
        }
    }
public:
    timer_st* tm;
    TIMER_TRIGGER_CALLBACK triggerCallback;
    TIMER_OVER_CALLBACK overCallback;
};

static std::map<std::string, TimerWrapper*> sTimerWrapperMap;
static std::list<std::string> sAddIdList;
static std::list<TimerWrapper*> sAddTimerWrapperList;
static std::mutex sAddListMutex;
static std::list<std::string> sStopIdList;
static std::mutex sStopIdListMutex;
static bool sClearFlag = false;
static std::mutex sClearFlagMutex;
static TimerManager* mInstance = NULL;

static void timerCallbackRun(timer_st* tm, unsigned long runCount, void* param) {
    std::map<std::string, TimerWrapper*>::iterator iter = sTimerWrapperMap.begin();
    for (; sTimerWrapperMap.end() != iter; ++iter) {
        if (get_timer_id(tm) == get_timer_id(iter->second->tm)) {
            if (iter->second->triggerCallback) {
                iter->second->triggerCallback(tm, runCount, param);
            }
            break;
        }
    }
}

static void timerCallbackOver(timer_st* tm, void* param) {
    std::map<std::string, TimerWrapper*>::iterator iter = sTimerWrapperMap.begin();
    for (; sTimerWrapperMap.end() != iter; ++iter) {
        if (get_timer_id(tm) == get_timer_id(iter->second->tm)) {
            if (iter->second->overCallback) {
                iter->second->overCallback(tm, param);
            }
            break;
        }
    }
}

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
    /* add list */
    sAddListMutex.lock();
    while (sAddIdList.size() > 0) {
        std::string id = *(sAddIdList.begin());
        sAddIdList.pop_front();
        TimerWrapper* wrapper = *(sAddTimerWrapperList.begin());
        sAddTimerWrapperList.pop_front();
        std::map<std::string, TimerWrapper*>::iterator iter = sTimerWrapperMap.find(id);
        if (sTimerWrapperMap.end() != iter) {
            sTimerWrapperMap.erase(iter);
            free(iter->second);
        }
        sTimerWrapperMap[id] = wrapper;
        start_timer(wrapper->tm, (unsigned long long)(getTime() * 1000), 0);
    }
    sAddListMutex.unlock();
    /* stop id list */
    sStopIdListMutex.lock();
    while (sStopIdList.size() > 0) {
        std::string id = *(sStopIdList.begin());
        sStopIdList.pop_front();
        std::map<std::string, TimerWrapper*>::iterator iter = sTimerWrapperMap.find(id);
        if (sTimerWrapperMap.end() != iter) {
            sTimerWrapperMap.erase(iter);
            free(iter->second);
        }
    }
    sStopIdListMutex.unlock();
    /* clear list */
    sClearFlagMutex.lock();
    if (sClearFlag) {
        std::map<std::string, TimerWrapper*>::iterator iter = sTimerWrapperMap.begin();
        for (; sTimerWrapperMap.end() != iter; ++iter) {
            free(iter->second);
        }
        sTimerWrapperMap.clear();
        sClearFlag = false;
    }
    sClearFlagMutex.unlock();
    /* update list */
    std::map<std::string, TimerWrapper*>::iterator iter = sTimerWrapperMap.begin();
    while (sTimerWrapperMap.end() != iter) {
        int ret = update_timer(iter->second->tm, (unsigned long long)(getTime() * 1000));
        if (1 == ret || 4 == ret) {
            sTimerWrapperMap.erase(iter);
            free(iter->second);
        } else {
            ++iter;
        }
    }
}

void TimerManager::run(const char* id, unsigned long interval, unsigned long count, TIMER_TRIGGER_CALLBACK triggerCallback, TIMER_OVER_CALLBACK overCallback, void* param /*= NULL*/) {
	if (!id || 0 == strlen(id)) {
		return;
	}
    sAddListMutex.lock();
    timer_st* tm = create_timer(interval, count, timerCallbackRun, timerCallbackOver, param);
    if (tm) {
        TimerWrapper* wrapper = (TimerWrapper*)malloc(sizeof(TimerWrapper));
        wrapper->tm = tm;
        wrapper->triggerCallback = triggerCallback;
        wrapper->overCallback = overCallback;
        sAddIdList.push_back(id);
        sAddTimerWrapperList.push_back(wrapper);
    }
    sAddListMutex.unlock();
}

void TimerManager::runLoop(const char* id, unsigned long interval, TIMER_TRIGGER_CALLBACK triggerCallback) {
	run(id, interval, 0, triggerCallback, NULL, NULL);
}

void TimerManager::runOnce(const char* id, unsigned long interval, TIMER_OVER_CALLBACK overCallback) {
	run(id, interval, 1, NULL, overCallback, NULL);
}

void TimerManager::stop(const char* id) {
	if (!id || 0 == strlen(id)) {
		return;
	}
    sStopIdListMutex.lock();
    sStopIdList.push_back(id);
    sStopIdListMutex.unlock();
}

void TimerManager::clear(void) {
    sClearFlagMutex.lock();
    sClearFlag = true;
    sClearFlagMutex.unlock();
}
