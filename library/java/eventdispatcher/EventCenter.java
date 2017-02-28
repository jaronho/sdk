package com.yaxon.hudmain.jh.library.eventdispatcher;

import android.os.Handler;
import android.os.Message;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Author:  jaron.ho
 * Date:    2017-02-07
 * Brief:   EventCenter
 */

public class EventCenter {
    private static class Event {
        public Object id = null;
        public EventDispatcher.Handler handler = null;
        public Event(Object id, EventDispatcher.Handler handler) {
            this.id = id;
            this.handler = handler;
        }
    }

    private static class HandlerObject {
        public EventDispatcher.Handler handler = null;
        public Object param = null;
        public HandlerObject(EventDispatcher.Handler handler) {
            this.handler = handler;
        }
    }

    private static final EventDispatcher mEventDispatcher = new EventDispatcher();
    private static final ConcurrentHashMap<Object, List<Event>> mEventMap = new ConcurrentHashMap<>();
    private static Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            HandlerObject handlerObj = (HandlerObject)msg.obj;
            if (null != handlerObj && null != handlerObj.handler) {
                handlerObj.handler.onCallback(handlerObj.param);
            }
        }
    };

    public static void subscribe(Object eventId, EventDispatcher.Handler handler, Object tag) {
        final HandlerObject handlerObj = new HandlerObject(handler);
        EventDispatcher.Handler tmpHandler = new EventDispatcher.Handler() {
            @Override
            public void onCallback(Object param) {
                handlerObj.param = param;
                Message msg = mHandler.obtainMessage();
                msg.obj = handlerObj;
                mHandler.sendMessage(msg);
            }
        };
        mEventDispatcher.subscribe(eventId, tmpHandler);
        List<Event> eventList = mEventMap.get(tag);
        if (null == eventList) {
            eventList = new ArrayList<>();
            mEventMap.put(tag, eventList);
        }
        eventList.add(new Event(eventId, tmpHandler));
    }

    public static void unsubscribe(Object tag) {
        if (null == tag) {
            mEventDispatcher.unsubscribe(null, null);
            mEventMap.clear();
        } else {
            List<Event> eventList = mEventMap.get(tag);
            if (null != eventList) {
                for (Event event : eventList) {
                    mEventDispatcher.unsubscribe(event.id, event.handler);
                }
                mEventMap.remove(tag);
            }
        }
    }

    public static void unsubscribe() {
        unsubscribe(null);
    }

    public static void post(Object eventId, Object param) {
        mEventDispatcher.post(eventId, param);
    }

    public static void post(Object eventId) {
        mEventDispatcher.post(eventId, null);
    }
}
