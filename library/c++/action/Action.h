/**********************************************************************
 * Author:	jaron.ho
 * Date:    2019-04-01
 * Brief:	动作(支持同步/异步),需要实现子类
 **********************************************************************/
#ifndef _ACTION_H_
#define _ACTION_H_

#include <functional>

/* 动作结束回调 */
#define ACTION_FINISH_CALLBACK std::function<void(Action* act)>

class Action {
public:
    Action(ACTION_FINISH_CALLBACK callback = nullptr);
    virtual ~Action(void);

protected:
    virtual void onProcess(void) = 0;

public:
    /*
     * Brief:	同步执行(在函数调用线程执行)
     * Param:	autoDestroy - true.动作自动销毁,false.需要手动销毁动作
     * Return:	Action*,若autoDestroy为true,则返回空指针
     */
    static Action* syncRun(Action* act, bool autoDestroy = false);

    /*
     * Brief:	异步执行
     * Param:	alone - true.在独立的线程中运行,false.在预设的线程中排队运行
     * Return:	void
     */
    static void asyncRun(Action* act, bool alone = false);

    /*
     * Brief:	异步监听(在主线程中循环执行),若动作有回调函数,则回调函数将在主线程中被调用
     * Param:	void
     * Return:	void
     */
    static void asyncListen(void);

private:
    static void aloneHandler(Action* act);

    static void groupHandler(void);

private:
    ACTION_FINISH_CALLBACK mFinishCallback;
};

#endif	/* _ACTION_H_ */
