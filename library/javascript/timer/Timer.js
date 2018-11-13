/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-11
 ** Brief:	timer
 ***********************************************************************/
function createTimer(interval, count, runCF, overCF, target, id, param) {
    if (interval <= 0) {
        throw new Error("interval <= 0");
    }
    var tm = {};
    // private member variables
    tm._interval = interval;			        // interval duration in milliseconds
    tm._totalCount = count;                     // number of intervals, if count <= 0, timer will repeat forever
    tm._currentCount = 0;				        // current interval count
    tm._startTime = 0;					        // start time for the current interval in milliseconds
    tm._running = false;				        // status of the timer
    tm._isPause = false;				        // is timer paused
    tm._runCallFunc = runCF;			        // called when current count changed
    tm._overCallFunc = overCF;			        // called when timer is complete
    tm._target = target;				        // callback target
    tm._id = 'string' == typeof(id) ? id : "";  // id
    tm._param = param;					        // parameter
    // Return: 0.running, 1.tm not running, 2.tm is pause or not trigger, 3.tm is over
    tm.update = function(currentTime) {
        if (!this._running) {
            return 1;
        }
        if (this._isPause || currentTime < this._startTime) {
            this._startTime = currentTime;
            return 2;
        }
        if (this._totalCount <= 0 || this._currentCount < this._totalCount) {
            var deltaTime = Math.abs(currentTime - this._startTime);
            if (deltaTime >= this._interval) {
                var runCount = Math.floor(deltaTime / this._interval);
                this._currentCount = this._currentCount + runCount;
                this._startTime = currentTime;
                if ('function' == typeof(tm._runCallFunc)) {
                    tm._runCallFunc.apply(tm._target, [this, runCount]);
                }
            }
        } else {
            this.stop(true);
            return 3;
        }
        return 0;
    };
    tm.start = function(currentTime, executeFlag) {
        if (this._running) {
            return;
        }
        this._running = true;
        this._isPause = false;
        this._currentCount = 0;
        this._startTime = currentTime;
        if ('function' == typeof(tm._runCallFunc) && executeFlag) {
            tm._runCallFunc.apply(tm._target, [this, 1]);
        }
    };
    tm.stop = function(executeFlag) {
        if (!this._running) {
            return;
        }
        this._running = false;
        this._isPause = true;
        if ('function' == typeof(tm._overCallFunc) && executeFlag) {
            tm._overCallFunc.apply(tm._target, [this]);
        }
    };
    tm.resume = function() {
        this._isPause = false;
    };
    tm.pause = function() {
        this._isPause = true;
    };
    tm.getInterval = function() {
        return this._interval;
    };
    tm.setInterval = function(interval) {
        if (interval <= 0) {
            throw new Error("interval <= 0");
        }
        this._interval = interval;
    };
    tm.getTotalCount = function() {
        return this._totalCount;
    };
    tm.setTotalCount = function(count) {
        this._totalCount = count;
    };
    tm.getCurrentCount = function() {
        return this._currentCount;
    };
    tm.isRunning = function() {
        return this._running;
    };
    tm.setRunHandler = function(runCF) {
        tm._runCallFunc = runCF;
    };
    tm.setOverHandler = function(overCF) {
        tm._overCallFunc = overCF;
    };
    tm.getId = function() {
        return this._id;
    };
    tm.getParam = function() {
        return this._param;
    };
    tm.setParam = function(param) {
        this._param = param;
    };
    return tm;
}
//----------------------------------------------------------------------
function createTimerManager() {
	var timerManager = {};
    timerManager._timerMap = {};
	// update timer manager, need to be called loop
	timerManager.update = function() {
        var nowTime = Date.now();
        for (var key in this._timerMap) {
            if (this._timerMap.hasOwnProperty(key)) {
                var timer = this._timerMap[key];
                if (timer && 'object' == typeof(timer) && 'function' == typeof(timer.update)) {
                    timer.update(nowTime);
                }
            }
        }
	};
	// start a custom timer
	timerManager.run = function(id, interval, count, runCF, overCF, target, param, doStartCB) {
        if ('string' != typeof(id) || 0 == id.length) {
            return;
        }
        if (this._timerMap[id]) {
            delete this._timerMap[id];
        }
        var timer = createTimer(interval, count, runCF, overCF, target, id, param);
        this._timerMap[id] = timer;
        timer.start(Date.now(), doStartCB);
	}
    // start a loop timer
    timerManager.runLoop = function(id, interval, runCF, target, param, doStartCB) {
        this.run(id, interval, 0, runCF, null, target, param, doStartCB);
    }
    // start an once timer
    timerManager.runOnce = function(id, interval, overCF, target, param, doStartCB) {
        this.run(id, interval, 1, null, overCF, target, param, doStartCB);
    }
    // stop a timer
    timerManager.stop = function(id, doStopCB) {
        if ('string' != typeof(id) || 0 == id.length) {
            return;
        }
        if (this._timerMap[id]) {
            var timer = this._timerMap[id];
            delete this._timerMap[id];
            if (doStopCB && timer && 'object' == typeof(timer)) {
                timer.stop(true);
            }
        }
    }
    // clear all timer
	timerManager.clear = function(doStopCB) {
		var tmpMap = this._timerMap;
        this._timerMap = {};
        if (doStopCB) {
            for (var key in tmpMap) {
                if (tmpMap.hasOwnProperty(key)) {
                    var timer = tmpMap[key];
                    if (timer && 'object' == typeof(timer)) {
                        timer.stop(true);
                    }
                }
            }
        }
	}
    // get a timer
    timerManager.get = function(id) {
        if ('string' == typeof(id) && id.length > 0) {
            return this._timerMap[id];
        }
    }
	return timerManager;
}
//----------------------------------------------------------------------
// test code
/*
var timerManager = createTimerManager();
function timer1_CF1(tm, runCount) {
	tm.setParam("count_" + tm.getCurrentCount());
}
function timer1_CF2(tm) {
}
timerManager.run("timer_1", 1, 0, timer1_CF1, timer1_CF2, null, null);
*/
//----------------------------------------------------------------------