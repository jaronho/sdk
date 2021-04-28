----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2014-04-14
-- Brief:	system time
----------------------------------------------------------------------
STime = STime or {}
----------------------------------------------------------------------
-- revise time, param: seconds from 1970-1-1, e.g. 1441712924
function STime:revise(reviseSeconds)
	-- calculate the time lag, seconds
	local reviseTime = tonumber(reviseSeconds) or 0
	if reviseTime > 0 then
		if reviseTime == self.mReviseTime then
			return
		end
		self.mReviseTime = reviseTime
		self.mDeltaTime = os.time() - reviseTime
	end
end
----------------------------------------------------------------------
-- get revise time(seconds from 1970-1-1)
function STime:getReviseTime()
	return self.mReviseTime or 0
end
----------------------------------------------------------------------
-- get now time(seconds from 1970-1-1)
function STime:getTime()
	local deltaTime = 0
	if "number" == type(self.mDeltaTime) then
		deltaTime = self.mDeltaTime
	end
	return os.time() - deltaTime
end
----------------------------------------------------------------------
-- get now date
function STime:getDate()
	local deltaTime = 0
	if "number" == type(self.mDeltaTime) then
		deltaTime = self.mDeltaTime
	end
	local dt = os.date("*t", os.time() - deltaTime)
	local wday = dt.wday - 1
	if 0 == dt.wday then
		wday = 7
	end
	return {
		year = dt.year,				-- >= 1970
		month = dt.month,			-- 1-12
		day = dt.day,				-- 1-31
		wday = wday,				-- 1-7
		hour = dt.hour,				-- 0-23
		minute = dt.min,			-- 0-59
		seconds = dt.sec			-- 0-59
	}
end
----------------------------------------------------------------------
-- time to date, param: seconds from 1970-1-1, e.g. 1441712924
function STime:timeToDate(tm)
	local dt = os.date("*t", tm)
	dt.wday = dt.wday - 1
	if 0 == dt.wday then
		dt.wday = 7
	end
	return {
		year = dt.year,				-- >= 1970
		month = dt.month,			-- 1-12
		day = dt.day,				-- 1-31
		wday = wday,				-- 1-7
		hour = dt.hour,				-- 0-23
		minute = dt.min,			-- 0-59
		seconds = dt.sec			-- 0-59
	}
end
----------------------------------------------------------------------
-- date to time
function STime:dateToTime(dt)
	return os.time {
		year = dt.year,				-- >= 1970
		month = dt.month,			-- 1-12
		day = dt.day,				-- 1-31
		hour = dt.hour or 0,		-- 0-23
		min = dt.minute or 0,		-- 0-59
		sec = dt.seconds or 0		-- 0-59
	}
end
----------------------------------------------------------------------
-- get dates in a week
function STime:getWeekDates(year, month, day)
	local specTime = self:dateToTime({year=year, month=month, day=day})
	local specDate = self:timeToDate(specTime)
	local weekDates = {}
	for i=1, 7 do
		local dateDiff = i - specDate.wday
		local timeDiff = dateDiff * 24 * 3600
		table.insert(weekDates, self:timeToDate(specTime + timeDiff))
	end
	return weekDates
end
----------------------------------------------------------------------
-- get days in a month
function STime:getMonthDays(year, month)
	local time1 = self:dateToTime({year=year, month=month, day=1})
	local nextYear, nextMonth = year, month + 1
	if nextMonth > 12 then
		nextYear = year + 1
		nextMonth = 1
	end
	local time2 = self:dateToTime({year=nextYear, month=nextMonth, day=1})
	return math.floor((time2 - time1)/(24*3600))
end
----------------------------------------------------------------------