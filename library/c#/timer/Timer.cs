/*--------------------------------------------------------------------
 * Author:	jaron.ho
 * Date:	2016-11-01
 * Brief:	timer
--------------------------------------------------------------------*/
using System.Collections;
//--------------------------------------------------------------------
public class Timer {
	private long mInterval = 1;						// interval duration in milliseconds
	private int mTotalCount = 0;					// number of intervals, if count <= 0, timer will repeat forever
	private int mCurrentCount = 0;					// current interval count
	private long mStartTime = 0;					// start time for the current interval in milliseconds
	private bool mRunning = false;					// status of the timer
	private bool mIsPause = false;					// is timer paused
	private TimerRunHandler mRunHandler = null;		// called when current count changed
	private TimerOverHandler mOverHandler = null;	// called when timer is complete
    private string mId = "";                        // id
	private System.Object mParam = null;			// parameter

	public delegate void TimerRunHandler(Timer tm, int runCount, System.Object param);
	public delegate void TimerOverHandler(Timer tm, System.Object param);

	public Timer(long interval, int count, TimerRunHandler runHandler = null, TimerOverHandler overHandler = null, string id = "", System.Object param = null) {
		if (interval <= 0) {
			throw new System.Exception("interval <= 0");
		}
		mInterval = interval;
		mTotalCount = count;
		mRunHandler = runHandler;
		mOverHandler = overHandler;
        mId = id;
		mParam = param;
	}

	public void update(long currentTime) {
		if (!mRunning) {
			return;
		}
		if (mIsPause || currentTime < mStartTime) {
			mStartTime = currentTime;
			return;
		}
        if (mTotalCount <= 0 || mCurrentCount < mTotalCount) {
            long deltaTime = System.Math.Abs(currentTime - mStartTime);
            if (deltaTime >= mInterval) {
                int runCount = (int)System.Math.Floor(deltaTime / mInterval);
                mCurrentCount = mCurrentCount + runCount;
                mStartTime = currentTime;
                if (null != mRunHandler) {
                    mRunHandler(this, runCount);
                }
            }
        } else {
            stop(true);
        }
	}

	public void start(long currentTime, bool executeFlag = false) {
		if (mRunning) {
			return;
		}
		mRunning = true;
		mIsPause = false;
		mCurrentCount = 0;
		mStartTime = currentTime;
		if (null != mRunHandler && executeFlag) {
			mRunHandler(this, 1);
		}
	}

	public void stop(bool executeFlag = false) {
		if (!mRunning) {
			return;
		}
		mRunning = false;
		mIsPause = true;
		if (null != mOverHandler && executeFlag) {
			mOverHandler(this);
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
		if (interval <= 0) {
			throw new System.Exception("interval <= 0");
		}
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

	public bool isRunning() {
		return mRunning;
	}

	public void setRunHandler(TimerRunHandler runHandler) {
		mRunHandler = runHandler;
	}

	public void setOverHandler(TimerOverHandler overHandler) {
		mOverHandler = overHandler;
	}
    
    private string getId() {
        return mId;
    }

	public System.Object getParam() {
		return mParam;
	}

	public void setParam(System.Object param) {
		mParam = param;
	}
}
//--------------------------------------------------------------------