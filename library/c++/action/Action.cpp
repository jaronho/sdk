/**********************************************************************
 * Author:	jaron.ho
 * Date:    2019-04-01
 * Brief:	动作(支持同步/异步),需要实现子类
 **********************************************************************/
#include "Action.h"
#include <unistd.h>
#include <list>
#include <map>
#include <mutex>
#include <thread>

static std::mutex sAsyncProcessMutex;
static std::list<Action*> sAsyncProcessList;
static std::mutex sDelayProcessMutex;
static std::map<unsigned long long, Action*> sDelayProcessMap;
static std::mutex sFinishMutex;
static std::list<Action*> sFinishList;

static unsigned long long generateActionId(void) {
    static int sY = 0, sM = 0, sD = 0, sH = 0, sMM = 0, sS = 0, sIdx = 1;
    time_t t = time(nullptr);
    struct tm* dt = localtime(&t);
    int year = 1900 + dt->tm_year, mon = 1 + dt->tm_mon, mday = dt->tm_mday, hour = dt->tm_hour, min = dt->tm_min, sec = dt->tm_sec;
    if (year == sY && mon == sM && mday == sD && hour == sH && min == sMM && sec == sS) {
        sIdx++;
    } else {
        sY = year; sM = mon; sD = mday; sH = hour; sMM = min; sS = sec; sIdx = 1;
    }
    return (unsigned long long)sY * 10000000000000 + (unsigned long long)sM * 100000000000 + (unsigned long long)sD * 1000000000 +
           (unsigned long long)sH * 10000000 + (unsigned long long)sMM * 100000 + (unsigned long long)sS * 1000 + (unsigned long long)sIdx;
}

void Action::asyncAloneHandler(Action* act) {
    if (act) {
        act->onProcess();
        if (act->mFinishCallback) {
            if (act->mAsyncCallbackToMainThread) {
                sFinishMutex.lock();
                sFinishList.push_back(act);
                sFinishMutex.unlock();
            } else {
                act->mFinishCallback(act);
                delete act;
                act = nullptr;
            }
        } else {
            delete act;
            act = nullptr;
        }
    }
}

void Action::asyncGroupHandler(void) {
    while (true) {
        Action* act = nullptr;
        sAsyncProcessMutex.lock();
        if (sAsyncProcessList.size() > 0) {
            act = *(sAsyncProcessList.begin());
            sAsyncProcessList.pop_front();
        }
        sAsyncProcessMutex.unlock();
        if (act) {
            act->onProcess();
            if (act->mFinishCallback) {
                if (act->mAsyncCallbackToMainThread) {
                    sFinishMutex.lock();
                    sFinishList.push_back(act);
                    sFinishMutex.unlock();
                } else {
                    act->mFinishCallback(act);
                    delete act;
                    act = nullptr;
                }
            } else {
                delete act;
                act = nullptr;
            }
        }
        usleep(10000);  /* 10毫秒 */
    }
}

void Action::delayGroupHandler(void) {
    const int STEP = 10000; /* 10毫秒 */
    while (true) {
        std::list<Action*> finishList;
        std::list<Action*>::iterator finishIter;
        std::map<unsigned long long, Action*>::iterator iter;
        sDelayProcessMutex.lock();
        iter = sDelayProcessMap.begin();
        for (; sDelayProcessMap.end() != iter; ++iter) {
            iter->second->mDelayTime -= STEP;
            if (iter->second->mDelayTime <= 0) {
                iter->second->mDelayTime = 0;
                finishList.push_back(iter->second);
            }
        }
        finishIter = finishList.begin();
        for (; finishList.end() != finishIter; ++finishIter) {
            iter = sDelayProcessMap.find((*finishIter)->getId());
            if (sDelayProcessMap.end() != iter) {
                sDelayProcessMap.erase(iter);
            }
        }
        sDelayProcessMutex.unlock();
        sFinishMutex.lock();
        finishIter = finishList.begin();
        for (; finishList.end() != finishIter; ++finishIter) {
            sFinishList.push_back(*finishIter);
        }
        sFinishMutex.unlock();
        usleep(STEP);
    }
}

Action::Action(ACTION_FINISH_CALLBACK callback, RunType type) {
    mId = generateActionId();
    mFinishCallback = callback;
    mAllowRunType = type;
}

Action::~Action(void) {}

unsigned long long Action::getId(void) {
    return mId;
}

int Action::syncRun(Action* act, bool autoDestroy) {
    if (!act) {
        return 1;
    }
    if (RT_NONE == act->mAllowRunType) {
        return 2;
    }
    if (RT_SYNC != act->mAllowRunType && RT_BOTH != act->mAllowRunType) {
        return 3;
    }
    act->mNowRunType = RT_SYNC;
    act->onProcess();
    if (act->mFinishCallback) {
        act->mFinishCallback(act);
    }
    if (autoDestroy) {
        delete act;
        act = nullptr;
    }
    return 0;
}

int Action::asyncRun(Action* act, bool alone, bool callbackToMainThread) {
    if (!act) {
        return 1;
    }
    if (RT_NONE == act->mAllowRunType) {
        return 2;
    }
    if (RT_ASYNC != act->mAllowRunType && RT_BOTH != act->mAllowRunType) {
        return 3;
    }
    act->mNowRunType = RT_ASYNC;
    act->mAsyncCallbackToMainThread = callbackToMainThread;
    if (alone) {
        std::thread asyncAloneThread(asyncAloneHandler, act);
        asyncAloneThread.detach();
    } else {
        static bool sInitAsyncGroupThread = true;
        if (sInitAsyncGroupThread) {
            sInitAsyncGroupThread = false;
            std::thread asyncGroupThread(asyncGroupHandler);
            asyncGroupThread.detach();
        }
        sAsyncProcessMutex.lock();
        sAsyncProcessList.push_back(act);
        sAsyncProcessMutex.unlock();
    }
    return 0;
}

int Action::delayRun(Action* act, unsigned long msec) {
    if (!act) {
        return 1;
    }
    if (RT_NONE == act->mAllowRunType) {
        return 2;
    }
    if (RT_DELAY != act->mAllowRunType && RT_BOTH != act->mAllowRunType) {
        return 3;
    }
    act->mNowRunType = RT_DELAY;
    act->mDelayTime = msec * 1000;
    static bool sInitDelayGroupThread = true;
    if (sInitDelayGroupThread) {
        sInitDelayGroupThread = false;
        std::thread delayGroupThread(delayGroupHandler);
        delayGroupThread.detach();
    }
    sDelayProcessMutex.lock();
    sDelayProcessMap.insert(std::pair<unsigned long long, Action*>(act->getId(), act));
    sDelayProcessMutex.unlock();
    return 0;
}

void Action::listen(void) {
    Action* act = nullptr;
    sFinishMutex.lock();
    if (sFinishList.size() > 0) {
        act = *(sFinishList.begin());
        sFinishList.pop_front();
    }
    sFinishMutex.unlock();
    if (act) {
        if (RT_DELAY == act->mNowRunType) {
            act->onProcess();
        }
        if (act->mFinishCallback) {
            act->mFinishCallback(act);
        }
        delete act;
        act = nullptr;
    }
}
