package com.jh.library.timer;

/**
 * Author:  jaron.ho
 * Date:    2017-02-07
 * Brief:   Timer
 */

public class Timer {
    public static abstract class RunHandler {
        public abstract void onCallback(Timer tm, int runCount, Object param);
    }

    public static abstract class OverHandler {
        public abstract void onCallback(Timer tm, Object param);
    }

    private long mInterval = 0;				    // interval duration in milliseconds
    private int mTotalCount = 0;				    // number of intervals, if count <= 0, timer will repeat forever
    private int mCurrentCount = 0;			    // current interval count
    private long mStartTime = 0;				    // start time for the current interval in milliseconds
    private boolean mRunning = false;			// status of the timer
    private boolean mIsPause = false;			// is timer paused
    private RunHandler mRunHandler = null;		// called when current count changed
    private OverHandler mOverHandler = null;	    // called when timer is complete
    private Object mParam = null;				    // parameter

    public Timer(long interval, int count, RunHandler runHandler, OverHandler overHandler, Object param) {
        mInterval = interval;
        mTotalCount = count;
        mRunHandler = runHandler;
        mOverHandler = overHandler;
        mParam = param;
    }

    public boolean update(long currentTime) {
        if (!mRunning) {
            return true;
        }
        if (mIsPause || currentTime < mStartTime) {
            mStartTime = currentTime;
            return true;
        }
        if (mTotalCount <= 0 || mCurrentCount < mTotalCount) {
            long deltaTime = Math.abs(currentTime - mStartTime);
            if (deltaTime >= mInterval) {
                int runCount = (int)Math.floor(deltaTime / mInterval);
                mCurrentCount += runCount;
                mStartTime = currentTime;
                if (null != mRunHandler) {
                    mRunHandler.onCallback(this, runCount, mParam);
                }
            }
        } else {
            stop(true);
            return false;
        }
        return true;
    }

    public void start(long currentTime, boolean executeFlag) {
        if (mRunning) {
            return;
        }
        mRunning = true;
        mIsPause = false;
        mCurrentCount = 0;
        mStartTime = currentTime;
        if (null != mRunHandler && executeFlag) {
            mRunHandler.onCallback(this, 1, mParam);
        }
    }

    public void stop(boolean executeFlag) {
        if (!mRunning) {
            return;
        }
        mRunning = false;
        mIsPause = true;
        if (null != mOverHandler && executeFlag) {
            mOverHandler.onCallback(this, mParam);
        }
    }

    public void resume() {
        mIsPause = false;
    }

    public void pause() {
        mIsPause = true;
    }

    public long getInterval() {
        return mInterval;
    }

    public void setInterval(long interval) {
        mInterval = interval;
    }

    public int getTotalCount() {
        return mTotalCount;
    }

    public void setTotalCount(int count) {
        mTotalCount = count;
    }

    public int getCurrentCount() {
        return mCurrentCount;
    }

    public boolean isRunning() {
        return mRunning;
    }

    public void setRunHandler(RunHandler runHandler) {
        mRunHandler = runHandler;
    }

    public void setOverHandler(OverHandler overHandler) {
        mOverHandler = overHandler;
    }

    public Object getParam() {
        return mParam;
    }

    public void setParam(Object param) {
        mParam = param;
    }
}
