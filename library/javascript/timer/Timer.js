/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-11
 ** Brief:	timer
 ***********************************************************************/
var mTimerList = [];
//----------------------------------------------------------------------
function insertTimer(tm) {
	mTimerList.push(tm);
}
//----------------------------------------------------------------------
function removeTimer(tm) {
	for (var i = 0, len = mTimerList.length; i < len; ++i) {
		if (tm == mTimerList[i]) {
			mTimerList.splice(i, 1);
			return;
		}
	}
}
//----------------------------------------------------------------------
// called every frame
function UpdateTimer() {
	for (var i = 0, len = mTimerList.length; i < len; ++i) {
		var timer = mTimerList[i];
		if (timer && 'object' == typeof(timer) && 'function' == typeof(timer.update)) {
			timer.update();
		}
	}
}
//----------------------------------------------------------------------
// clear timer list
function ClearTimer() {
	mTimerList = [];
}
//----------------------------------------------------------------------
// create a timer
function CreateTimer(interval, count, runCF, overCF, target, param) {
	// private member variables
	var mInterval = interval;			// interval duration in milliseconds
	var mTotalCount = count;			// number of intervals, if count <= 0, timer will repeat forever
	var mCurrentCount = 0;				// current interval count
	var mStartTime = 0;					// start time for the current interval in milliseconds
	var mRunning = false;				// status of the timer
	var mIsPause = false;				// is timer paused
	var mRunCallFunc = runCF;			// called when current count changed
	var mOverCallFunc = overCF;			// called when timer is complete
	var mTarget = target;				// callback target
	var mParam = param;					// parameter
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
				var runCount = 1;
				if (mInterval > 0) {
					runCount = Math.floor(deltaTime / mInterval);
				}
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
		mRunning = false;
		mIsPause = true;
		if ('function' == typeof(mOverCallFunc) && executeFlag) {
			mOverCallFunc.apply(mTarget, [this]);
		}
		removeTimer(this);
	};
	tm.resume = function() {
		mIsPause = false;
	};
	tm.pause = function() {
		mIsPause = true;
	};
	tm.getTotalCount = function() {
		return mTotalCount;
	};
	tm.getCurrentCount = function() {
		return mCurrentCount;
	};
	tm.isRunning = function() {
		return mRunning;
	};
	tm.setParam = function(param) {
		mParam = param;
	};
	tm.getParam = function() {
		return mParam;
	};
	return tm;
}
//----------------------------------------------------------------------
// test code
/*
function timer1_CF1(tm, runCount) {
	tm.setParam("count_" + tm.getCurrentCount());
}
function timer1_CF2(tm) {
}
var timer1 = CreateTimer(1, 0, timer1_CF1, timer1_CF2);
timer1.setParam("hahaha");
timer1.start();
*/
//----------------------------------------------------------------------