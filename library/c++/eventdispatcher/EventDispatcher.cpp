/**********************************************************************
* Author:	jaron.ho
* Date:		2012-09-06
* Brief:	event dispatcher
**********************************************************************/
#include "EventDispatcher.h"

Handler::Handler(void) {
}

Handler::~Handler(void) {
	clearEvents();
}

void Handler::handleEvents(int eventId, void* param) {
}

void Handler::addEvent(Event* event) {
	if (nullptr == event) {
		return;
	}
	if (-1 == getEventIndex(event)) {
		mEvents.push_back(event);
	}
}

void Handler::clearEvents(void) {
	for (size_t i = 0; i < mEvents.size(); ++i) {
		Event *event = mEvents[i];
		if (event) {
			event->removeHandler(this);
		}
	}
	mEvents.clear();
}

int Handler::getEventIndex(Event* event) {
	for (size_t i = 0; i < mEvents.size(); ++i) {
		if (event == mEvents[i]) {
			return i;
		}
	}
	return -1;
}

Event::Event(int eventId) : mId(eventId) {
}

Event::~Event(void) {
	removeAllHandlers();
}

int Event::getEventId(void) {
	return mId;
}

void Event::addHandler(Handler* handler) {
	if (nullptr == handler) {
		return;
	}
	if (-1 == getHandlerIndex(handler)) {
		mHandlers.push_back(handler);
		handler->addEvent(this);
	}
}

void Event::addHandler(CallFunc callfunc) {
	if (nullptr == callfunc) {
		return;
	}
	if (-1 == getCallFuncIndex(callfunc)) {
		mCallFuncs.push_back(callfunc);
	}
}

void Event::removeHandler(Handler* handler) {
	if (nullptr == handler) {
		return;
	}
	int index = getHandlerIndex(handler);
	if (-1 == index) {
		return;
	}
	mHandlers.erase(mHandlers.begin() + index);
}

void Event::removeHandler(CallFunc callfunc) {
	if (nullptr == callfunc) {
		return;
	}
	int index = getCallFuncIndex(callfunc);
	if (-1 == index) {
		return;
	}
	mCallFuncs.erase(mCallFuncs.begin() + index);
}

void Event::removeAllHandlers(void) {
	mHandlers.clear();
	mCallFuncs.clear();
}

void Event::excute(void* param /* = nullptr */) {
	for (size_t i = 0; i < mHandlers.size(); ++i) {
		Handler *handler = mHandlers[i];
		if (handler) {
			handler->handleEvents(mId, param);
		}
	}
	for (size_t j = 0; j < mCallFuncs.size(); ++j) {
		CallFunc callfunc = mCallFuncs[j];
		if (callfunc) {
			callfunc(param);
		}
	}
}

int Event::getHandlerIndex(Handler* handler) {
	for (size_t i = 0; i < mHandlers.size(); ++i) {
		if (handler == mHandlers[i]) {
			return i;
		}
	}
	return -1;
}

int Event::getCallFuncIndex(CallFunc callfunc) {
	for (size_t i = 0; i < mCallFuncs.size(); ++i) {
		if (callfunc == mCallFuncs[i]) {
			return i;
		}
	}
	return -1;
}

typedef std::map<int, Event*>::iterator DISPATCHER_ITER;

EventDispatcher::EventDispatcher(void) {
}

EventDispatcher::~EventDispatcher(void) {
	clear();
}

void EventDispatcher::subscribe(int eventId, Handler* handler) {
	if (nullptr == handler) {
		return;
	}
	if (mEventMap.end() == mEventMap.find(eventId)) {
		mEventMap.insert(std::make_pair(eventId, new Event(eventId)));
	}
	mEventMap[eventId]->addHandler(handler);
}

void EventDispatcher::subscribe(int eventId, CallFunc callfunc) {
	if (nullptr == callfunc) {
		return;
	}
	if (mEventMap.end() == mEventMap.find(eventId)) {
		mEventMap.insert(std::make_pair(eventId, new Event(eventId)));
	}
	mEventMap[eventId]->addHandler(callfunc);
}

void EventDispatcher::unsubscribe(int eventId, Handler* handler) {
	if (mEventMap.end() != mEventMap.find(eventId)) {
		mEventMap[eventId]->removeHandler(handler);
	}
}

void EventDispatcher::unsubscribe(int eventId, CallFunc callfunc) {
	if (mEventMap.end() != mEventMap.find(eventId)) {
		mEventMap[eventId]->removeHandler(callfunc);
	}
}

void EventDispatcher::unsubscribe(int eventId) {
	DISPATCHER_ITER iter = mEventMap.find(eventId);
	if (mEventMap.end() != iter) {
		delete (iter->second);
		iter->second = nullptr;
		mEventMap.erase(iter);
	}
}

void EventDispatcher::post(int eventId, void* param /* = nullptr */) {
	if (mEventMap.end() != mEventMap.find(eventId)) {
		mEventMap[eventId]->excute(param);
	}
}

void EventDispatcher::clear(void) {
	DISPATCHER_ITER iter = mEventMap.begin();
	for (; mEventMap.end() != iter; ++iter) {
		delete (iter->second);
		iter->second = nullptr;
	}
	mEventMap.clear();
}
