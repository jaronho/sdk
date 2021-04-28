----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2013-11-19
-- Brief:	event dispathcer system
----------------------------------------------------------------------
function CreateEventDispatcher()
	local mEventHandlerMap = {}
	local dispathcer = {}
	-- subscribe event handler: eventId(number,string);func(function);priority(number),1>2>3
	function dispathcer:subscribe(eventId, func, target, priority)
		assert("number" == type(eventId) or "string" == type(eventId), "eventId is not number or string, it's type is "..type(eventId))
		assert("function" == type(func), "func is not function, it's type is "..type(func))
		local handlers = mEventHandlerMap[eventId]
		if nil == handlers then
			handlers = {}
			if "number" ~= type(priority) then
				priority = #handlers
			end
			table.insert(handlers, {func = func, target = target, priority = priority})
			mEventHandlerMap[eventId] = handlers
			return
		end
		for key, handler in pairs(handlers) do
			if func == handler.func then
				return
			end
		end
		if "number" ~= type(priority) then
			priority = #handlers
		end
		table.insert(mEventHandlerMap[eventId], {func = func, target = target, priority = priority})
		table.sort(mEventHandlerMap[eventId], function(a, b) return a.priority < b.priority end)
	end
	-- unsubscribe event: eventId(number,string);func(function)
	function dispathcer:unsubscribe(eventId, func)
		if nil == eventId then
			mEventHandlerMap = {}
			return
		end
		assert("number" == type(eventId) or "string" == type(eventId), "eventId is not number or string, it's type is "..type(eventId))
		local handlers = mEventHandlerMap[eventId]
		if nil == handlers then
			return
		end
		if nil == func then
			mEventHandlerMap[eventId] = nil
			return
		end
		for key, handler in pairs(handlers) do
			if func == handler.func then
				table.remove(mEventHandlerMap[eventId], key)
				break
			end
		end
		if 0 == #mEventHandlerMap[eventId] then
			mEventHandlerMap[eventId] = nil
		end
	end
	-- post event: eventId(number,string);...(any type)
	function dispathcer:post(eventId, ...)
		assert("number" == type(eventId) or "string" == type(eventId), "eventId is not number or string, it's type is "..type(eventId))
		local handlers = mEventHandlerMap[eventId]
		if nil == handlers then
			return
		end
		for key, handler in pairs(handlers) do
			if "function" == type(handler.func) then
				if "table" == type(handler.target) or "userdata" == type(handler.target) then
					handler.func(handler.target, ...)
				else
					handler.func(...)
				end
			end
		end
	end
	return dispathcer
end
----------------------------------------------------------------------
local mEventDispatcher = CreateEventDispatcher()
EventCenter = {}
-- bind event
function EventCenter:bind(eventId, func, target, priority)
	mEventDispatcher:subscribe(eventId, func, target, priority)
end
-- unbind event
function EventCenter:unbind(eventId, func)
	mEventDispatcher:unsubscribe(eventId, func)
end
-- clear event
function EventCenter:clear()
	mEventDispatcher:unsubscribe()
end
-- post event
function EventCenter:post(eventId, ...)
	mEventDispatcher:post(eventId, ...)
end
----------------------------------------------------------------------