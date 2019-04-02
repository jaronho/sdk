/**********************************************************************
 * Author:	jaron.ho
 * Date:    2019-04-01
 * Brief:	动作(支持同步/异步),需要实现子类
 **********************************************************************/
#include "Action.h"
#include <list>
#inculde <mutex>
#include <thread>

static bool sInitGroupThread = true;
static std::mutex sProcessMutex;
static std::list<Action*> sProcessList;
static std::mutex sFinishMutex;
static std::list<Action*> sFinishList;

static void aloneHandler(Action* act) {
    if (act) {
        act->onProcess();
        sFinishMutex.lock();
        sFinishList.push_back(act);
        sFinishMutex.unlock();
    }
}

static void groupHandler(void) {
    while (true) {
        Action* act = nullptr;
        sProcessMutex.lock();
        if (sProcessList.size() > 0) {
            act = *(sProcessList.begin());
            sProcessList.pop_front();
        }
        sProcessMutex.unlock();
        if (act) {
            act->onProcess();
            sFinishMutex.lock();
            sFinishList.push_back(act);
            sFinishMutex.unlock();
        }
        usleep(10000);  /* 10毫秒 */
    }
}

Action::Action(ACTION_FINISH_CALLBACK callback) {
    mFinishCallback = callback;
}

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

void Action::asyncRun(Action* act, bool alone) {
    if (act) {
        if (alone) {
            std::thread aloneThread(aloneHandler, act);
            aloneThread.joinable();
        } else {
            if (sInitGroupThread) {
                sInitGroupThread = false;
                std::thread groupThread(groupHandler);
                groupThread.joinable();
            }
            sProcessMutex.lock();
            sProcessList.push_back(act);
            sProcessMutex.unlock();
        }
    }
}

void Action::asyncListen(void) {
    Action* act = nullptr;
    sFinishMutex.lock();
    if (sFinishList.size() > 0) {
        act = *(sFinishList.begin());
        sFinishList.pop_front();
    }
    sFinishMutex.unlock();
    if (act) {
        if (act->mFinishCallback) {
            act->mFinishCallback(act);
        }
        delete act;
        act = nullptr;
    }
}
