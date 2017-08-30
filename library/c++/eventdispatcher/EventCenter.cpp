/**********************************************************************
* Author:	jaron.ho
* Date:		2013-09-16
* Brief:	event center
**********************************************************************/
#include "EventCenter.h"

EventDispatcher* EventCenter::mEventDispatcher = nullptr;

void EventCenter::init(void) {
	if (nullptr == mEventDispatcher) {
		mEventDispatcher = new EventDispatcher();
	}
}

void EventCenter::subscribe(int eventId, Handler* handler) {
	init();
	mEventDispatcher->subscribe(eventId, handler);
}

void EventCenter::subscribe(int eventId, CallFunc callfunc) {
	init();
	mEventDispatcher->subscribe(eventId, callfunc);
}

void EventCenter::unsubscribe(int eventId, Handler* handler) {
	init();
	mEventDispatcher->unsubscribe(eventId, handler);
}

void EventCenter::unsubscribe(int eventId, CallFunc callfunc) {
	init();
	mEventDispatcher->unsubscribe(eventId, callfunc);
}

void EventCenter::unsubscribe(int eventId) {
	init();
	mEventDispatcher->unsubscribe(eventId);
}

void EventCenter::post(int eventId, void* param /* = nullptr*/) {
	init();
	mEventDispatcher->post(eventId, param);
}
