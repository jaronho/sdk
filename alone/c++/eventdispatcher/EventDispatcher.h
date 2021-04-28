/**********************************************************************
* Author:	jaron.ho
* Date:		2012-09-06
* Brief:	event dispatcher
**********************************************************************/
#ifndef _EVENT_DISPATCHER_H_
#define _EVENT_DISPATCHER_H_

#include <string>
#include <vector>
#include <map>

class Handler;
class Event;
class EventDispatcher;

typedef void (*CallFunc)(void*);
#define EventCallFunc(callfunc) (CallFunc)(&callfunc)

/*
 * Brief:	所有事件监听者的基类
 */
class Handler {
public:
	Handler(void);
	
	~Handler(void);

public:
	/*
	 * Brief:	处理所监听的事件,若子类要处理事件,则需要重写此函数
	 * Param:	eventId - 事件id
	 *			param - 参数
	 * Return:	void
	 */
	virtual void handleEvents(int eventId, void* param);

	/*
	 * Brief:	添加监听事件
	 * Param:	event - 参数
	 * Return:	void
	 */
	void addEvent(Event* event);

private:
	/*
	 * Brief:	清除所有监听事件
	 * Param:	void
	 * Return:	void
	 */
	void clearEvents(void);

	/*
	 * Brief:	获取监听事件索引
	 * Param:	event - 参数
	 * Return:	void
	 */
	int getEventIndex(Event* event);

private:
	std::vector<Event*> mEvents;			// 句柄所监听的事件集
};

/*
 * Brief:	事件
 */
class Event {
public:
	Event(int eventId);

	~Event(void);
	
public:
	/*
	 * Brief:	获取事件标识
	 * Param:	void
	 * Return:	int
	 */
	int getEventId(void);
	
	/*
	 * Brief:	添加事件处理句柄
	 * Param:	handler - 事件处理句柄
	 * Return:	void
	 */
	void addHandler(Handler* handler);

	/*
	 * Brief:	添加事件处理函数
	 * Param:	callfunc - 事件处理函数
	 * Return:	void
	 */
	void addHandler(CallFunc callfunc);
	
	/*
	 * Brief:	移除事件处理句柄
	 * Param:	handler - 事件处理句柄
	 * Return:	void
	 */
	void removeHandler(Handler* handler);

	/*
	 * Brief:	移除事件处理函数
	 * Param:	callfunc - 事件处理函数
	 * Return:	void
	 */
	void removeHandler(CallFunc callfunc);
	
	/*
	 * Brief:	移除所有事件处理
	 * Param:	void
	 * Return:	void
	 */
	void removeAllHandlers(void);
	
	/*
	 * Brief:	执行事件
	 * Param:	param - 参数
	 * Return:	void
	 */
	void excute(void* param = nullptr);

private:
	/*
	 * Brief:	获取事件处理句柄索引
	 * Param:	handler - 事件处理句柄
	 * Return:	int
	 */
	int getHandlerIndex(Handler* handler);

	/*
	 * Brief:	获取事件处理函数索引
	 * Param:	callfunc - 事件处理函数
	 * Return:	int
	 */
	int getCallFuncIndex(CallFunc callfunc);
	
private:
	int mId;									// 事件标识

	std::vector<Handler*> mHandlers;			// 事件处理句柄

	std::vector<CallFunc> mCallFuncs;			// 事件处理函数
};

/*
 * Brief:	事件派发器
 */
class EventDispatcher {
public:
	EventDispatcher(void);

	~EventDispatcher(void);
	
public:
	/*
	 * Brief:	注册事件
	 * Param:	eventId - 事件标识
	 *			handler - 事件处理句柄
	 * Return:	void
	*/
	void subscribe(int eventId, Handler* handler);

	/*
	 * Brief:	注册事件
	 * Param:	eventId - 事件标识
	 *			callfunc - 事件处理函数
	 * Return:	void
	 */
	void subscribe(int eventId, CallFunc callfunc);
	
	/*
	 * Brief:	取消事件注册
	 * Param:	eventId - 事件标识
	 *			handler - 要取消注册的事件处理句柄
	 * Return:	void
	 */
	void unsubscribe(int eventId, Handler* handler);

	/*
	 * Brief:	取消事件注册
	 * Param:	eventId - 事件标识
	 *			callfunc - 要取消注册的事件处理函数
	 * Return:	void
	 */
	void unsubscribe(int eventId, CallFunc callfunc);
	
	/*
	 * Brief:	取消事件注册
	 * Param:	eventId - 事件标识
	 * Return:	void
	 */
	void unsubscribe(int eventId);
	
	/*
	 * Brief:	发布事件
	 * Param:	eventId - 事件标识
	 *			param - 参数
	 * Return:	void
	*/
	void post(int eventId, void* param = nullptr);
	
	/*
	 * Brief:	清除所有事件
	 * Param:	void
	 * Return:	void
	 */
	void clear(void);
	
private:
	std::map<int, Event*> mEventMap;		// 事件集
};

#endif	// _EVENT_DISPATCHER_H_
