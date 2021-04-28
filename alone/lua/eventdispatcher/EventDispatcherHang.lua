----------------------------------------------------------------------
-- Author:	jaron.ho
-- Date:	2013-11-19
-- Brief:	event dispatcher hang
----------------------------------------------------------------------
-- 事件派发器挂载
function EventDispatcherHang(obj)
	assert("table" == type(obj), "type of obj '"..type(obj).."' is error")
	-- 事件注册函数
	function obj:bind(eventId, handler, priority)
		obj._eventHandlerMap = obj._eventHandlerMap or {}
		assert(nil == obj._eventHandlerMap[eventId], "can't bind two same event")
		obj._eventHandlerMap[eventId] = handler
		EventCenter:bind(eventId, obj._eventHandlerMap[eventId], obj, priority)
	end
	-- 取消事件注册函数
	function obj:unbind(eventId)
		obj._eventHandlerMap = obj._eventHandlerMap or {}
		if nil == eventId then	-- 清空所有事件注册函数
			for eventId, handler in pairs(obj._eventHandlerMap) do
				EventCenter:unbind(eventId, handler)
			end
			obj._eventHandlerMap = {}
		else					-- 取消单个事件注册函数
			if obj._eventHandlerMap[eventId] then
				EventCenter:unbind(eventId, obj._eventHandlerMap[eventId])
			end
			obj._eventHandlerMap[eventId] = nil
		end
	end
end
----------------------------------------------------------------------