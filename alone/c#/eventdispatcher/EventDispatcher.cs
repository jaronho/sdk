/*--------------------------------------------------------------------
 * Author:	jaron.ho
 * Date:	2016-11-01
 * Brief:	event dispathcer system
--------------------------------------------------------------------*/
using System.Collections;
using System.Collections.Generic;
//--------------------------------------------------------------------
public class EventDispatcher {
	private Hashtable mEventHandlerMap = new Hashtable();

	public delegate void EventHandler(System.Object obj);

	public void subscribe(System.Object eventId, EventHandler handler) {
		List<EventHandler> handlers = (List<EventHandler>)mEventHandlerMap[eventId];
		if (null == handlers) {
			handlers = new List<EventHandler>();
			mEventHandlerMap.Add(eventId, handlers);
		}
		handlers.Add(handler);
	}

	public void unsubscribe(System.Object eventId, EventHandler handler = null) {
		List<EventHandler> handlers = (List<EventHandler>)mEventHandlerMap[eventId];
		if (null == handlers) {
			return;
		}
		if (null == handler) {
			mEventHandlerMap.Remove(eventId);
		} else {
			handlers.Remove(handler);
			if (0 == handlers.Count) {
				mEventHandlerMap.Remove(eventId);
			}
		}
	}

	public void post(System.Object eventId, System.Object param = null) {
		List<EventHandler> handlers = (List<EventHandler>)mEventHandlerMap[eventId];
		if (null == handlers) {
			return;
		}
		handlers.ForEach(delegate(EventHandler handler) {
			handler(param);
		});
	}

	public void post(int eventId) {
		post(eventId, null);
	}
}
//--------------------------------------------------------------------
public class EventCenter {
	private static EventDispatcher mEventDispatcher = new EventDispatcher();
	public static void subscribe(System.Object eventId, EventDispatcher.EventHandler handler) {
		mEventDispatcher.subscribe(eventId, handler);
	}
	public static void unsubscribe(System.Object eventId, EventDispatcher.EventHandler handler = null) {
		mEventDispatcher.unsubscribe(eventId, handler);
	}
	public static void unsubscribe(System.Object eventId) {
		mEventDispatcher.unsubscribe(eventId, null);
	}
	public static void post(System.Object eventId, System.Object param = null) {
		mEventDispatcher.post(eventId, param);
	}
	public static void post(System.Object eventId) {
		mEventDispatcher.post(eventId);
	}
}
//--------------------------------------------------------------------
