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

Action::Action(ACTION_FINISH_CALLBACK callback) {
    mFinishCallback = callback;
}

Action::~Action(void) {}

Action* Action::syncRun(Action* act, bool autoDestroy) {
    if (act) {
        act->onProcess();
        if (act->mFinishCallback) {
            act->mFinishCallback(act);
        }
        if (autoDestroy) {
            delete act;
            act = nullptr;
        }
    }
    return act;
}

void Action::asyncRun(Action* act, bool alone, bool callbackToMainThread) {
    if (act) {
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
    }
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
