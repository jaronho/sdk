/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-11
 ** Brief:	timer
 ***********************************************************************/
function CreateTimerManager() {
	// private member variables
	var mTimerList = [];
	var timerManager = {};
	// insert to timer list
	function insertTimer(tm) {
		for (var i = 0; i < mTimerList.length; ++i) {
			if (tm == mTimerList[i]) {
				return;
			}
		}
		mTimerList.push(tm);
	}
	// remove from timer list
	function removeTimer(tm) {
		for (var i = 0; i < mTimerList.length; ++i) {
			if (tm == mTimerList[i]) {
				mTimerList.splice(i, 1);
				return;
			}
		}
	}
	// called every frame
	timerManager.updateTimer = function(currentTime) {
		for (var i = 0; i < mTimerList.length; ++i) {
			var timer = mTimerList[i];
			if (timer && 'object' == typeof(timer) && 'function' == typeof(timer.update)) {
				timer.update(currentTime);
			}
		}
	};
	// clear timer list
	timerManager.clearTimer = function() {
		mTimerList = [];
	}
	// create a timer
	timerManager.createTimer = function(interval, count, runCF, overCF, target, id, param) {
		// private member variables
		if (interval <= 0) {
			throw new Error("interval <= 0");
		}
		var mInterval = interval;			        // interval duration in milliseconds
		var mTotalCount = count;			        // number of intervals, if count <= 0, timer will repeat forever
		var mCurrentCount = 0;				        // current interval count
		var mStartTime = 0;					        // start time for the current interval in milliseconds
		var mRunning = false;				        // status of the timer
		var mIsPause = false;				        // is timer paused
		var mRunCallFunc = runCF;			        // called when current count changed
		var mOverCallFunc = overCF;			        // called when timer is complete
		var mTarget = target;				        // callback target
        var mId = 'string' == typeof(id) ? id : ""; // id
		var mParam = param;					        // parameter
		var tm = {};
		// public methods
		tm.update = function(currentTime) {
			if (!mRunning) {
				return;
			}
			if (mIsPause || currentTime < mStartTime) {
				mStartTime = currentTime;
				return;
			}
            if (mTotalCount <= 0 || mCurrentCount < mTotalCount) {
                var deltaTime = Math.abs(currentTime - mStartTime);
                if (deltaTime >= mInterval) {
                    var runCount = Math.floor(deltaTime / mInterval);
                    mCurrentCount = mCurrentCount + runCount;
                    mStartTime = currentTime;
                    if ('function' == typeof(mRunCallFunc)) {
                        mRunCallFunc.apply(mTarget, [this, runCount]);
                    }
                }
            } else {
                this.stop(true);
            }
		};
		tm.start = function(currentTime, executeFlag) {
			if (mRunning) {
				return;
			}
			mRunning = true;
			mIsPause = false;
			mCurrentCount = 0;
			mStartTime = currentTime;
			if ('function' == typeof(mRunCallFunc) && executeFlag) {
				mRunCallFunc.apply(mTarget, [this, 1]);
			}
			insertTimer(this);
		};
		tm.stop = function(executeFlag) {
			if (!mRunning) {
				return;
			}
			removeTimer(this);
			mRunning = false;
			mIsPause = true;
			if ('function' == typeof(mOverCallFunc) && executeFlag) {
				mOverCallFunc.apply(mTarget, [this]);
			}
		};
		tm.resume = function() {
			mIsPause = false;
		};
		tm.pause = function() {
			mIsPause = true;
		};
		tm.getInterval = function() {
			return mInterval;
		};
		tm.setInterval = function(interval) {
			if (interval <= 0) {
				throw new Error("interval <= 0");
			}
			mInterval = interval;
		};
		tm.getTotalCount = function() {
			return mTotalCount;
		};
		tm.setTotalCount = function(count) {
			mTotalCount = count;
		};
		tm.getCurrentCount = function() {
			return mCurrentCount;
		};
		tm.isRunning = function() {
			return mRunning;
		};
		tm.setRunHandler = function(runCF) {
			mRunCallFunc = runCF;
		};
		tm.setOverHandler = function(overCF) {
			mOverCallFunc = overCF;
		};
        tm.getId = function() {
            return mId;
        };
		tm.getParam = function() {
			return mParam;
		};
		tm.setParam = function(param) {
			mParam = param;
		};
		return tm;
	}
	return timerManager;
}
//----------------------------------------------------------------------
// test code
/*
var TimeManager = CreateTimerManager();
function timer1_CF1(tm, runCount) {
	tm.setParam("count_" + tm.getCurrentCount());
}
function timer1_CF2(tm) {
}
var timer1 = TimeManager.createTimer(1, 0, timer1_CF1, timer1_CF2);
timer1.setParam("hahaha");
timer1.start();
*/
//----------------------------------------------------------------------