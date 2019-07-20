/**********************************************************************
 * Author:	jaron.ho
 * Date:    2019-04-01
 * Brief:	动作(支持同步/异步),需要实现子类
 **********************************************************************/
#ifndef _ACTION_H_
#define _ACTION_H_

#include <functional>

/* 动作结束回调 */
#define ACTION_FINISH_CALLBACK std::function<void(const Action* act)>

class Action {
public:
    /* 执行方式 */
    enum RunType {
        RT_NONE,                /* 不允许使用syncRun,asyncRun,delayRun执行 */
        RT_SYNC,                /* 只能同步执行 */
        RT_ASYNC,               /* 只能异步执行 */
        RT_DELAY,               /* 只能延迟执行 */
        RT_BOTH                 /* 允许同步,异步,延迟执行 */
    };
public:
    Action(ACTION_FINISH_CALLBACK callback = nullptr, RunType type = RT_BOTH);
    virtual ~Action(void);

    /*
     * Brief:	获取动作id
     * Param:	void
     * Return:	unsigned long long
     */
    unsigned long long getId(void);

protected:
    /*
     * Brief:	动作处理函数,需要子线程实现(同步:在动作调用线程执行,异步:在子线程排队执行,延迟:在主线程执行)
     * Param:	void
     * Return:	void
     */
    virtual void onProcess(void) = 0;

public:
    /*
     * Brief:	同步执行(在函数调用线程执行)
     * Param:	act - 动作
     *          autoDestroy - true.动作自动销毁,false.动作需要手动销毁
     * Return:	0.成功
     *          1.act为空指针
     *          2.act动作不允许使用syncRun,asyncRun,delayRun执行
     *          3.act动作不允许同步执行
     */
    static int syncRun(Action* act, bool autoDestroy = false);

    /*
     * Brief:	异步执行(在子线程中执行,动作自动销毁)
     * Param:	act - 动作
     *          alone - true.在独立的线程中运行,false.在预设的线程中排队运行
     *          callbackToMainThread - true.动作结束回调在主线程执行,false.动作回调在子线程执行
     * Return:	0.成功
     *          1.act为空指针
     *          2.act动作不允许使用syncRun,asyncRun,delayRun执行
     *          3.act动作不允许异步执行
     */
    static int asyncRun(Action* act, bool alone = false, bool callbackToMainThread = true);

    /*
     * Brief:	延迟执行(在主线程中执行,动作自动销毁)
     * Param:	act - 动作
     *          msec - 延迟时间(毫秒)
     * Return:	0.成功
     *          1.act为空指针
     *          2.act动作不允许使用syncRun,asyncRun,delayRun执行
     *          3.act动作不允许延迟执行
     */
    static int delayRun(Action* act, unsigned long msec);

    /*
     * Brief:	监听回调(需要在主线程中循环调用),若动作有回调函数,则回调函数将在主线程中被调用
     * Param:	void
     * Return:	void
     */
    static void listen(void);

private:
    static void asyncAloneHandler(Action* act);

    static void asyncGroupHandler(void);

    static void delayGroupHandler(void);

private:
    unsigned long long mId;                             /* id */
    ACTION_FINISH_CALLBACK mFinishCallback;             /* 动作结束回调函数 */
    RunType mAllowRunType;                              /* 动作允许的执行方式 */
    RunType mNowRunType;                                /* 动作当前执行方式 */
    bool mAsyncCallbackToMainThread;                    /* 异步执行:是否在主线程执行回调 */
    unsigned long mDelayTime;                           /* 延迟执行:时间(毫秒) */
};

#endif	/* _ACTION_H_ */

/*
************************************************** sample_01

class ActionTest: public Action {
public:
    ActionTest(std::string name, int n, ACTION_FINISH_CALLBACK callback)
        : Action(callback) {
        this->name = name;
        this->num = n;
    }
protected:
    void onProcess(void) {
        int i = 0;
        while (i < num) {
            std::cout << "thread id: " << std::this_thread::get_id() << ", " << name << " - " << ++i << std::endl;
            Sleep(1000);
        }
    }
public:
    std::string name;
    int num;
};

int main() {
    std::cout << "main thread id: " << std::this_thread::get_id() << std::endl;
    std::cout << "=========== 1" << std::endl;
    Action::syncRun(new ActionTest("aaa", 3, [&](const Action* act)->void {
        std::cout << "----- thread id: " << std::this_thread::get_id() << ", " << ((const ActionTest*)act)->name << " end ..." << std::endl;
    }), true);
    int asyncCount = 2;
    Action::asyncRun(new ActionTest("bbb", 5, [&](const Action* act)->void {
        std::cout << "----- thread id: " << std::this_thread::get_id() << ", " << ((const ActionTest*)act)->name << " end ..." << std::endl;
        --asyncCount;
    }), false, true);
    Action::asyncRun(new ActionTest("ccc", 7, [&](const Action* act)->void {
        std::cout << "----- thread id: " << std::this_thread::get_id() << ", " << ((const ActionTest*)act)->name << " end ..." << std::endl;
        --asyncCount;
    }), true, false);
    std::cout << "=========== 2" << std::endl;
    while (1) {
        Action::asyncListen();
        if (0 == asyncCount) {
            break;
        }
        Sleep(10);
    }
    std::cout << "=========== 3" << std::endl;
    return 0;
}
*/
