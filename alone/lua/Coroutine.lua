----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2015-10-08
-- Brief:	coroutine
----------------------------------------------------------------------
local mCoroutineList = {}
----------------------------------------------------------------------
local function insertCoroutine(co)
	table.insert(mCoroutineList, co)
end
----------------------------------------------------------------------
local function removeCoroutine(co)
	for key, val in pairs(mCoroutineList) do
		if co == val then
			table.remove(mCoroutineList, key)
			break
		end
	end
end
----------------------------------------------------------------------
-- called every frame
function UpdateCoroutine()
	for _, val in pairs(mCoroutineList) do
		if ("table" == type(val) or "userdata" == type(val)) and "function" == type(val.update) then
			val:update()
		end
	end
end
----------------------------------------------------------------------
-- clear coroutine list
function ClearCoroutine()
	mCoroutineList = {}
end
----------------------------------------------------------------------
-- create a coroutine
function CreateCoroutine(runCF, overCF, target, param)
	assert("function" == type(runCF), "runCF not support type '"..type(runCF).."'")
	-- private member variables
	local mCoroutine = nil					-- coroutine
	local mWaitFrames = 0					-- wait frame count
	local mWaitSeconds = 0					-- wait seconds
	local mRunning = false					-- status of the coroutine
	local mRunCallFunc = runCF				-- called when execute
	local mOverCallFunc = overCF			-- called when coroutine is clean up
	local mTarget = target					-- callback target
	local mParam = param					-- parameter
	local co = {}
	-- public methods
	function co:update()
		if not mCoroutine then
			return
		end
		if mWaitFrames > 0 then
			mWaitFrames = mWaitFrames - 1
			if mWaitFrames <= 0 then
				mWaitFrames = 0
				self:resume(false)
			end
		elseif mWaitSeconds > 0 then
			if system_gettime() >= mWaitSeconds then
				mWaitSeconds = 0
				self:resume(false)
			end
		end
	end
	function co:run()
		if mCoroutine then
			return false
		end
		mCoroutine = coroutine.create(function()
			if "function" == type(mRunCallFunc) then
				if "table" == type(mTarget) or "userdata" == type(mTarget) then
					mRunCallFunc(mTarget, co)
				else
					mRunCallFunc(co)
				end
			end
		end)
		if not self:resume(false) then
			return false
		end
		insertCoroutine(self)
		return true
	end
	function co:clean()
		if not mCoroutine then
			return
		end
		mCoroutine = nil
		mRunning = false
		removeCoroutine(self)
		if "function" == type(mOverCallFunc) then
			if "table" == type(mTarget) or "userdata" == type(mTarget) then
				mOverCallFunc(mTarget, self)
			else
				mOverCallFunc(self)
			end
		end
	end
	function co:resume(resetWait)
		if not mCoroutine or mRunning then
			return false
		end
		if nil == resetWait or resetWait then
			mWaitFrames = 0
			mWaitSeconds = 0
		end
		mRunning = true
		local success, yieldType, yieldParam = coroutine.resume(mCoroutine, self)
		if not success then
			self:clean()
			return false
		end
		if "dead" == coroutine.status(mCoroutine) then
			self:clean()
			return false
		end
		return true
	end
	function co:pause(waitFrames, waitSeconds)
		if not mCoroutine or not mRunning then
			return
		end
		if "number" == type(waitFrames) and waitFrames > 0 then
			mWaitFrames = waitFrames + 1
			mWaitSeconds = 0
		elseif "number" == type(waitSeconds) and waitSeconds > 0 then
			mWaitFrames = 0
			mWaitSeconds = system_gettime() + waitSeconds
		else
			mWaitFrames = 0
			mWaitSeconds = 0
		end
		mRunning = false
		coroutine.yield()
	end
	function co:setParam(param) mParam = param end
	function co:getParam() return mParam end
	return co
end
----------------------------------------------------------------------