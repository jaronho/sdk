package com.jaronho.sdk.library.eventdispatcher;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Author:  jaron.ho
 * Date:    2017-02-07
 * Brief:   EventDispatcher
 */

public class EventDispatcher {
    public static abstract class Handler {
        public abstract void onCallback(Object param);
    }

    private ConcurrentHashMap<Object, List<Handler>> mEventHandlerMap = new ConcurrentHashMap<>();

    public void subscribe(Object eventId, Handler handler) {
        List<Handler> handlers = mEventHandlerMap.get(eventId);
        if (null == handlers) {
            handlers = new ArrayList<>();
            mEventHandlerMap.put(eventId, handlers);
        }
        if (!handlers.contains(handler)) {
            handlers.add(handler);
        }
    }

    public void unsubscribe(Object eventId, Handler handler) {
        if (null == eventId) {
            mEventHandlerMap.clear();
        } else if (null == handler) {
            mEventHandlerMap.remove(eventId);
        } else {
            List<Handler> handlers = mEventHandlerMap.get(eventId);
            if (null != handlers) {
                handlers.remove(handler);
                if (handlers.isEmpty()) {
                    mEventHandlerMap.remove(eventId);
                }
            }
        }
    }

    public void post(Object eventId, Object param) {
        List<Handler> handlers = mEventHandlerMap.get(eventId);
        if (null != handlers) {
            for (Handler handler : handlers) {
                if (null != handler) {
                    handler.onCallback(param);
                }
            }
        }
    }
}
