/***********************************************************************
** Author:	jaron.ho
** Date:	2015-08-06
** Brief:	event dispathcer system
***********************************************************************/
function createEventDispatcher() {
	var dispathcer = {};
    dispathcer._eventHandlerMap = {};
	// subscribe event handler: eventId(number,string);func(function);target(object);priority(number),1>2>3
	dispathcer.subscribe = function (eventId, func, target, priority) {
		if ('string' != typeof(eventId) && 'number' != typeof(eventId)) {
			throw new Error("eventId is not number or string, it's type is " + typeof(eventId));
		}
		if ('function' != typeof(func)) {
			throw new Error("func is not function, it's type is " + typeof(func));
		}
		var handlers = this._eventHandlerMap[eventId];
		if (!(handlers instanceof Array)) {
			handlers = [];
			if ('number' != typeof(priority)) {
				priority = handlers.length;
			}
			handlers.push({func: func, target: target, priority: priority});
			this._eventHandlerMap[eventId] = handlers;
			return true;
		}
		for (var i = 0; i < handlers.length; ++i) {
			if (func == handlers[i].func) {
				return false;
			}
		}
		if ('number' != typeof(priority)) {
			priority = handlers.length;
		}
		this._eventHandlerMap[eventId].push({func: func, target: target, priority: priority});
		this._eventHandlerMap[eventId].sort(function (a, b) {
			return a.priority - b.priority;
		});
		return true;
	};
	// unsubscribe event: eventId(number,string);func(function)
	dispathcer.unsubscribe = function (eventId, func) {
		if ('string' != typeof(eventId) && 'number' != typeof(eventId)) {
			throw new Error("eventId is not number or string, it's type is " + typeof(eventId));
		}
		var handlers = this._eventHandlerMap[eventId];
		if (!(handlers instanceof Array)) {
			return;
		}
		if ('function' != typeof(func)) {
			delete this._eventHandlerMap[eventId];
			return;
		}
		for (var i = 0; i < handlers.length; ++i) {
			if (handlers[i] && 'object' == typeof(handlers[i]) && func == handlers[i].func) {
				handlers.splice(i, 1);
				break;
			}
		}
		if (0 == handlers.length) {
			delete this._eventHandlerMap[eventId];
		}
	};
	// post event: eventId(number,string)
	dispathcer.post = function (eventId) {
		if ('string' != typeof(eventId) && 'number' != typeof(eventId)) {
			throw new Error("eventId is not number or string, it's type is " + typeof(eventId));
		}
		var handlers = this._eventHandlerMap[eventId];
		if (!(handlers instanceof Array) || 0 == handlers.length) {
			return false;
		}
		for (var i = 0; i < handlers.length; ++i) {
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
	var center = {};
    center._dispathcer = createEventDispatcher();
	// bind event
	center.bind = function(eventId, func, target, priority) {
		return this._dispathcer.subscribe(eventId, func, target, priority);
	};
	// unbind event
	center.unbind = function(eventId, func) {
		this._dispathcer.unsubscribe(eventId, func);
	};
	// post event
	center.post = function(eventId) {
		return this._dispathcer.post.apply(this._dispathcer, arguments);
	};
	return center;
})();
/**********************************************************************/