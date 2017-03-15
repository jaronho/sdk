/***********************************************************************
** Author:	jaron.ho
** Date:	2015-08-06
** Brief:	event dispathcer system
***********************************************************************/
function CreateEventDispatcher() {
	var mEventHandlerMap = {};
	var dispathcer = {};
	// subscribe event handler: eventId(number,string);func(function);target(object);priority(number),1>2>3
	dispathcer.subscribe = function (eventId, func, target, priority) {
		if ('string' != typeof(eventId) && 'number' != typeof(eventId)) {
			throw new Error("eventId is not number or string, it's type is " + typeof(eventId));
		}
		if ('function' != typeof(func)) {
			throw new Error("func is not function, it's type is " + typeof(func));
		}
		var handlers = mEventHandlerMap[eventId];
		if (!(handlers instanceof Array)) {
			handlers = [];
			if ('number' != typeof(priority)) {
				priority = handlers.length;
			}
			handlers.push({func: func, target: target, priority: priority});
			mEventHandlerMap[eventId] = handlers;
			return true;
		}
		for (var i = 0, len = handlers.length; i < len; ++i) {
			if (func == handlers[i].func) {
				return false;
			}
		}
		if ('number' != typeof(priority)) {
			priority = handlers.length;
		}
		mEventHandlerMap[eventId].push({func: func, target: target, priority: priority});
		mEventHandlerMap[eventId].sort(function (a, b) {
			return a.priority - b.priority;
		});
		return true;
	};
	// unsubscribe event: eventId(number,string);func(function)
	dispathcer.unsubscribe = function (eventId, func) {
		if ('string' != typeof(eventId) && 'number' != typeof(eventId)) {
			throw new Error("eventId is not number or string, it's type is " + typeof(eventId));
		}
		var handlers = mEventHandlerMap[eventId];
		if (!(handlers instanceof Array)) {
			return;
		}
		if ('function' != typeof(func)) {
			delete mEventHandlerMap[eventId];
			return;
		}
		for (var i = 0, len = handlers.length; i < len; ++i) {
			if (handlers[i] && 'object' == typeof(handlers[i]) && func == handlers[i].func) {
				handlers.splice(i, 1);
				break;
			}
		}
		if (0 == handlers.length) {
			delete mEventHandlerMap[eventId];
		}
	};
	// post event: eventId(number,string)
	dispathcer.post = function (eventId) {
		if ('string' != typeof(eventId) && 'number' != typeof(eventId)) {
			throw new Error("eventId is not number or string, it's type is " + typeof(eventId));
		}
		var handlers = mEventHandlerMap[eventId];
		if (!(handlers instanceof Array) || 0 == handlers.length) {
			return false;
		}
		for (var i = 0, len = handlers.length; i < len; ++i) {
			if (handlers[i] && 'object' == typeof(handlers[i]) && 'function' == typeof(handlers[i].func)) {
				handlers[i].func.apply(handlers[i].target, Array.prototype.slice.call(arguments, 1));
			}
		}
		return true;
	};
	return dispathcer;
}
/**********************************************************************/
var EventCenter = (function () {
	var mEventDispatcher = CreateEventDispatcher();
	var mEventCenter = {};
	// bind event
	mEventCenter.bind = function(eventId, func, target, priority) {
		return mEventDispatcher.subscribe(eventId, func, target, priority);
	};
	// unbind event
	mEventCenter.unbind = function(eventId, func) {
		mEventDispatcher.unsubscribe(eventId, func);
	};
	// post event
	mEventCenter.post = function(eventId) {
		return mEventDispatcher.post.apply(mEventDispatcher, arguments);
	};
	return mEventCenter;
})();
/**********************************************************************/