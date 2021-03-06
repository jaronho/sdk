package com.jaronho.sdk.library.timer;

import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;

import java.util.Map.Entry;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Author:  jaron.ho
 * Date:    2017-02-08
 * Brief:   TimerManager
 */
public class TimerManager {
    private class RunObject {
        public Timer timer = null;
        public Timer.RunHandler handler = null;
        public RunObject(Timer.RunHandler handler) {
            this.handler = handler;
        }
    }

    private class OverObject {
        public Timer timer = null;
        public Timer.OverHandler handler = null;
        public OverObject(Timer.OverHandler handler) {this.handler = handler;}
    }

    private static TimerManager mInstance = null;
    private final ConcurrentHashMap<String, Timer> mTimerMap = new ConcurrentHashMap<>();
    private final Handler mRunHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            RunObject runObj = (RunObject)msg.obj;
            if (null != runObj && null != runObj.handler) {
                int runCount = msg.arg1;
                runObj.handler.onCallback(runObj.timer, runCount);
            }
        }
    };
    private final Handler mOverHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            OverObject overObj = (OverObject)msg.obj;
            if (null != overObj && null != overObj.handler) {
                overObj.handler.onCallback(overObj.timer);
            }
        }
    };
    private Thread mThread = new Thread(new Runnable() {
        private void loopUpdate(long currentTime) {
            Set<Entry<String, Timer>> entrySet = mTimerMap.entrySet();
            for (Entry<String, Timer> entry : entrySet) {
                Timer tm = entry.getValue();
                if (null != tm) {
                    tm.update(currentTime);
                }
            }
        }

        @Override
        public void run() {
            while (!mThread.isInterrupted()) {
                loopUpdate(SystemClock.elapsedRealtime());
            }
        }
    });

    private TimerManager() {
        if (null != mThread) {
            mThread.start();
        }
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        mTimerMap.clear();
        if (null != mThread) {
            mThread.interrupt();
            mThread = null;
        }
    }

    public static TimerManager getInstance() {
        if (null == mInstance) {
            synchronized(TimerManager.class) {
                if (null == mInstance) {
                    mInstance = new TimerManager();
                }
            }
        }
        return mInstance;
    }

    public void run(final String id, long interval, int count, Timer.RunHandler runHandler, Timer.OverHandler overHandler, Object param, boolean doStartCB) {
        if (id.length() > 0 && interval > 0) {
            final RunObject runObj = new RunObject(runHandler);
            final OverObject overObj = new OverObject(overHandler);
            Timer tm = new Timer(interval, count, new Timer.RunHandler() {
                @Override
                public void onCallback(Timer tm, int runCount, Object param) {
                    Message msg = mRunHandler.obtainMessage();
                    msg.obj = runObj;
                    msg.arg1 = runCount;
                    mRunHandler.sendMessage(msg);
                }
            }, new Timer.OverHandler() {
                @Override
                public void onCallback(Timer tm, Object param) {
                    if (mTimerMap.containsKey(id)) {
                        mTimerMap.remove(id);
                    }
                    Message msg = mOverHandler.obtainMessage();
                    msg.obj = overObj;
                    mOverHandler.sendMessage(msg);
                }
            }, id, param);
            if (mTimerMap.containsKey(id)) {
                mTimerMap.remove(id);
            }
            mTimerMap.put(id, tm);
            runObj.timer = tm;
            overObj.timer = tm;
            tm.start(SystemClock.elapsedRealtime(), doStartCB);
        }
    }

    public void run(final String id, long interval, int count, Timer.RunHandler runHandler, Timer.OverHandler overHandler, boolean doStartCB) {
        run(id, interval, count, runHandler, overHandler, null, doStartCB);
    }

    public void run(final String id, long interval, int count, Timer.RunHandler runHandler, boolean doStartCB) {
        run(id, interval, count, runHandler, null, null, doStartCB);
    }

    public void run(final String id, long interval, int count, Timer.OverHandler overHandler) {
        run(id, interval, count, null, overHandler, null, false);
    }

    public String run(long interval, int count, Timer.RunHandler runHandler, Timer.OverHandler overHandler, Object param, boolean doStartCB) {
        String id = UUID.randomUUID().toString();
        run(id, interval, count, runHandler, overHandler, param, doStartCB);
        return id;
    }

    public String run(long interval, int count, Timer.RunHandler runHandler, Timer.OverHandler overHandler, boolean doStartCB) {
        String id = UUID.randomUUID().toString();
        run(id, interval, count, runHandler, overHandler, null, doStartCB);
        return id;
    }

    public String run(long interval, int count, Timer.RunHandler runHandler, boolean doStartCB) {
        String id = UUID.randomUUID().toString();
        run(id, interval, count, runHandler, null, null, doStartCB);
        return id;
    }

    public String run(long interval, int count, Timer.OverHandler overHandler) {
        String id = UUID.randomUUID().toString();
        run(id, interval, count, null, overHandler, null, false);
        return id;
    }

    public void stop(String id, boolean doStopCB) {
        if (mTimerMap.containsKey(id)) {
            Timer tm = mTimerMap.remove(id);
            if (null != tm && doStopCB) {
                tm.stop(true);
            }
        }
    }

    public void clear(boolean doStopCB) {
        Set<Entry<String, Timer>> entrySet = mTimerMap.entrySet();
        mTimerMap.clear();
        if (doStopCB) {
            for (Entry<String, Timer> entry : entrySet) {
                Timer tm = entry.getValue();
                if (null != tm) {
                    tm.stop(true);
                }
            }
        }
    }
}
