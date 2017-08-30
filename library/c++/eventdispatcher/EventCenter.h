/**********************************************************************
* Author:	jaron.ho
* Date:		2013-09-16
* Brief:	event center
**********************************************************************/
#ifndef _EVENT_CENTER_H_
#define _EVENT_CENTER_H_

#include "EventDispatcher.h"

/*
 * Brief:	事件中心
 */
class EventCenter
{
public:
	static void subscribe(int eventId, Handler* handler);
	
	static void subscribe(int eventId, CallFunc callfunc);
	
	static void unsubscribe(int eventId, Handler* handler);
	
	static void unsubscribe(int eventId, CallFunc callfunc);
	
	static void unsubscribe(int eventId);
	
	static void post(int eventId, void* param = nullptr);

private:
	static void init(void);
	
private:
	static EventDispatcher *mEventDispatcher;
};

#endif	// _EVENT_CENTER_H_
