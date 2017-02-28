/*--------------------------------------------------------------------
 * Author:	jaron.ho
 * Date:	2016-11-01
 * Brief:	timer
--------------------------------------------------------------------*/
using System.Collections;
//--------------------------------------------------------------------
public class Timer {
	private float mInterval = 0;					// interval duration in seconds
	private int mTotalCount = 0;					// number of intervals, if count <= 0, timer will repeat forever
	private int mCurrentCount = 0;					// current interval count
	private float mStartTime = 0;					// start time for the current interval in seconds
	private bool mRunning = false;					// status of the timer
	private bool mIsPause = false;					// is timer paused
	private TimerRunHandler mRunHandler = null;		// called when current count changed
	private TimerOverHandler mOverHandler = null;	// called when timer is complete
	private System.Object mParam = null;			// parameter

	public delegate void TimerRunHandler(Timer tm, int runCount, System.Object param);
	public delegate void TimerOverHandler(Timer tm, System.Object param);

	public Timer(float interval, int count, TimerRunHandler runHandler = null, TimerOverHandler overHandler = null, System.Object param = null) {
		if (interval <= 0.0f) {
			throw new System.Exception("interval <= 0.0f");
		}
		mInterval = interval;
		mTotalCount = count;
		mRunHandler = runHandler;
		mOverHandler = overHandler;
		mParam = param;
	}

	public bool update(float currentTime) {
		if (!mRunning) {
			return true;
		}
		if (mIsPause || currentTime < mStartTime) {
			mStartTime = currentTime;
			return true;
		}
		if (mTotalCount <= 0 || mCurrentCount < mTotalCount) {
			float deltaTime = System.Math.Abs(currentTime - mStartTime);
			if (deltaTime >= mInterval) {
				int runCount = (int)System.Math.Floor(deltaTime / mInterval);
				mCurrentCount += runCount;
				mStartTime = currentTime;
				if (null != mRunHandler) {
					mRunHandler(this, runCount, mParam);
				}
			}
		} else {
			stop(true);
			return false;
		}
		return true;
	}

	public void start(float currentTime, bool executeFlag = false) {
		if (mRunning) {
			return;
		}
		mRunning = true;
		mIsPause = false;
		mCurrentCount = 0;
		mStartTime = currentTime;
		if (null != mRunHandler && executeFlag) {
			mRunHandler(this, 1, mParam);
		}
	}

	public void stop(bool executeFlag = false) {
		if (!mRunning) {
			return;
		}
		mRunning = false;
		mIsPause = true;
		if (null != mOverHandler && executeFlag) {
			mOverHandler(this, mParam);
		}
	}

	public void resume() {
		mIsPause = false;
	}

	public void pause() {
		mIsPause = true;
	}

	public float getInterval() {
		return mInterval;
	}

	public void setInterval(float interval) {
		if (interval <= 0.0f) {
			throw new System.Exception("interval <= 0.0f");
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

	public System.Object getParam() {
		return mParam;
	}

	public void setParam(System.Object param) {
		mParam = param;
	}
}
//--------------------------------------------------------------------