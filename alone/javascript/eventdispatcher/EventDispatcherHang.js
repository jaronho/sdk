/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-011
 ** Brief:	event dispathcer hang
 ***********************************************************************/
function eventDispatcherHang(obj) {
	if (!obj || ('object' != typeof(obj) && 'function' != typeof(obj))) {
		throw new Error("not support for obj type '" + typeof(obj) + "'");
	}
	obj.bind = function (eventId, handler, priority) {
		this._eventHandlerMap = 'object' == typeof(this._eventHandlerMap) ? this._eventHandlerMap : {};
		if (this._eventHandlerMap[eventId]) {
			throw new Error("can't subscribe two same event " + eventId);
		}
		this._eventHandlerMap[eventId] = handler;
		EventCenter.bind(eventId, this._eventHandlerMap[eventId], this, priority);
	};
	obj.unbind = function (eventId) {
		this._eventHandlerMap = 'object' == typeof(this._eventHandlerMap) ? this._eventHandlerMap : {};
		if (eventId) {		// remove single event
			if (this._eventHandlerMap[eventId]) {
				EventCenter.unbind(eventId, this._eventHandlerMap[eventId]);
				delete this._eventHandlerMap[eventId];
			}
		} else {			// clear all event
			for (var key in this._eventHandlerMap) {
				if (this._eventHandlerMap.hasOwnProperty(key)) {
					EventCenter.unbind(key, this._eventHandlerMap[key]);
				}
			}
			this._eventHandlerMap = {};
		}
	};
}
//----------------------------------------------------------------------