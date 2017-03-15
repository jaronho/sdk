/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-011
 ** Brief:	event dispathcer hang
 ***********************************************************************/
function EventDispatcherHang(obj) {
	if (!obj || ('object' != typeof(obj) && 'function' != typeof(obj))) {
		throw new Error("not support for obj type '" + typeof(obj) + "'");
	}
	obj.bind = function (eventId, handler, priority) {
		obj._eventHandlerMap = 'object' == typeof(obj._eventHandlerMap) ? obj._eventHandlerMap : {};
		if (obj._eventHandlerMap[eventId]) {
			throw new Error("can't subscribe two same event " + eventId);
		}
		obj._eventHandlerMap[eventId] = handler;
		EventCenter.bind(eventId, obj._eventHandlerMap[eventId], obj, priority);
	};
	obj.unbind = function (eventId) {
		obj._eventHandlerMap = 'object' == typeof(obj._eventHandlerMap) ? obj._eventHandlerMap : {};
		if (eventId) {		// remove single event
			if (obj._eventHandlerMap[eventId]) {
				EventCenter.unbind(eventId, obj._eventHandlerMap[eventId]);
				delete obj._eventHandlerMap[eventId];
			}
		} else {			// clear all event
			for (var key in obj._eventHandlerMap) {
				if (obj._eventHandlerMap.hasOwnProperty(key)) {
					EventCenter.unbind(key, obj._eventHandlerMap[key]);
				}
			}
			obj._eventHandlerMap = {};
		}
	};
}
//----------------------------------------------------------------------