/**********************************************************************
 * Author:	jaron.ho
 * Date:    2019-04-01
 * Brief:	动作(支持同步/异步),需要实现子类
 **********************************************************************/
#include "Action.h"
#include <unistd.h>
#include <list>
#include <mutex>
#include <thread>

static std::mutex sAsyncProcessMutex;
static std::list<Action*> sAsyncProcessList;
static std::mutex sAsyncFinishMutex;
static std::list<Action*> sAsyncFinishList;

void Action::aloneHandler(Action* act) {
    if (act) {
        act->onProcess();
        if (act->mFinishCallback) {
            if (act->mAsyncCallbackToMainThread) {
                sAsyncFinishMutex.lock();
                sAsyncFinishList.push_back(act);
                sAsyncFinishMutex.unlock();
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

void Action::groupHandler(void) {
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
                    sAsyncFinishMutex.lock();
                    sAsyncFinishList.push_back(act);
                    sAsyncFinishMutex.unlock();
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

Action::Action(ACTION_FINISH_CALLBACK callback, RunType type) {
    mFinishCallback = callback;
    mRunType = type;
}

Action::~Action(void) {}

int Action::syncRun(Action* act, bool autoDestroy) {
    if (!act) {
        return 1;
    }
    if (RT_SYNC != act->mRunType && RT_BOTH != act->mRunType) {
        return 2;
    }
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
    if (RT_ASYNC != act->mRunType && RT_BOTH != act->mRunType) {
        return 2;
    }
    act->mAsyncCallbackToMainThread = callbackToMainThread;
    if (alone) {
        std::thread aloneThread(aloneHandler, act);
        aloneThread.detach();
    } else {
        static bool sInitGroupThread = true;
        if (sInitGroupThread) {
            sInitGroupThread = false;
            std::thread groupThread(groupHandler);
            groupThread.detach();
        }
        sAsyncProcessMutex.lock();
        sAsyncProcessList.push_back(act);
        sAsyncProcessMutex.unlock();
    }
    return 0;
}

void Action::asyncListen(void) {
    Action* act = nullptr;
    sAsyncFinishMutex.lock();
    if (sAsyncFinishList.size() > 0) {
        act = *(sAsyncFinishList.begin());
        sAsyncFinishList.pop_front();
    }
    sAsyncFinishMutex.unlock();
    if (act) {
        if (act->mFinishCallback) {
            act->mFinishCallback(act);
        }
        delete act;
        act = nullptr;
    }
}
