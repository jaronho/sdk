----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2014-04-14
-- Brief:	timer
----------------------------------------------------------------------
local mTimerList = {}
----------------------------------------------------------------------
local function insertTimer(tm)
	table.insert(mTimerList, tm)
end
----------------------------------------------------------------------
local function removeTimer(tm)
	for key, val in pairs(mTimerList) do
		if tm == val then
			table.remove(mTimerList, key)
			return
		end
	end
end
----------------------------------------------------------------------
-- called every frame
function UpdateTimer()
	for _, val in pairs(mTimerList) do
		if ("table" == type(val) or "userdata" == type(val)) and "function" == type(val.update) then
			val:update()
		end
	end
end
----------------------------------------------------------------------
-- clear timer list
function ClearTimer()
	mTimerList = {}
end
----------------------------------------------------------------------
-- create a timer
function CreateTimer(interval, count, runCF, overCF, target, param)
	-- private member variables
	assert(interval > 0, "interval <= 0")
	local mInterval = interval			-- interval duration in milliseconds
	local mTotalCount = count			-- number of intervals, if count <= 0, timer will repeat forever
	local mCurrentCount = 0				-- current interval count
	local mStartTime = 0				-- start time for the current interval in milliseconds
	local mRunning = false				-- status of the timer
	local mIsPause = false				-- is timer paused
	local mRunCallFunc = runCF			-- called when current count changed
	local mOverCallFunc = overCF		-- called when timer is complete
	local mTarget = target				-- callback target
	local mParam = param				-- parameter
	local tm = {}
	-- public methods
	function tm:update(currentTime)
		if not mRunning then
			return
		end
		if mIsPause or currentTime < mStartTime then
			mStartTime = currentTime
			return
		end
		if mTotalCount <= 0 or mCurrentCount < mTotalCount then
			local deltaTime = math.abs(currentTime - mStartTime)
			if deltaTime >= mInterval then
				local runCount = math.floor(deltaTime/mInterval)
				mCurrentCount = mCurrentCount + runCount
				mStartTime = currentTime
				if "function" == type(mRunCallFunc) then
					if "table" == type(mTarget) or "userdata" == type(mTarget) then
						mRunCallFunc(mTarget, self, runCount)
					else
						mRunCallFunc(self, runCount)
					end
				end
			end
		else
			self:stop(true)
		end
	end
	function tm:start(currentTime, executeFlag)
		if mRunning then
			return
		end
		mRunning = true
		mIsPause = false
		mCurrentCount = 0
		mStartTime = currentTime
		if "function" == type(mRunCallFunc) and executeFlag then
			if "table" == type(mTarget) or "userdata" == type(mTarget) then
				mRunCallFunc(mTarget, self, 1)
			else
				mRunCallFunc(self, 1)
			end
		end
		insertTimer(self)
	end
	function tm:stop(executeFlag)
		if not mRunning then
			return
		end
		mRunning = false
		mIsPause = true
		if "function" == type(mOverCallFunc) and executeFlag then
			if "table" == type(mTarget) or "userdata" == type(mTarget) then
				mOverCallFunc(mTarget, self)
			else
				mOverCallFunc(self)
			end
		end
		removeTimer(self)
	end
	function tm:resume()
		mIsPause = false
	end
	function tm:pause()
		mIsPause = true
	end
	function tm:getInterval()
		return mInterval
	end
	function tm:setInterval(interval)
		assert(interval > 0, "interval <= 0")
		mInterval = interval
	end
	function tm:getTotalCount()
		return mTotalCount
	end
	function tm:setTotalCount(count)
		mTotalCount = count
	end
	function tm:getCurrentCount()
		return mCurrentCount
	end
	function tm:isRunning()
		return mRunning
	end
	function tm:setRunHandler(runCF)
		mRunCallFunc = runCF
	end
	function tm:setOverHandler(overCF)
		mOverCallFunc = overCF
	end
	function tm:getParam()
		return mParam
	end
	function tm:setParam(param)
		mParam = param
	end
	return tm
end
----------------------------------------------------------------------
-- test code
--[[
local function timer1_CF1(tm, runCount)
	tm:setParam("count_"..tm:getCurrentCount())
end
local function timer1_CF2(tm)
end
local timer1 = CreateTimer(1, 0, timer1_CF1, timer1_CF2)
timer1:setParam("hahaha")
timer1:start()
]]
----------------------------------------------------------------------